
/**
 * This file is part of the boostcache package.
 *
 * (c) Azat Khuzhin <a3at.mail@gmail.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

#include "jsvm.h"
#include "util/log.h"
#include "config.h" /** HAVE_* */


/**
 * XXX: get rid of support for old v8
 */

namespace Js
{
#ifdef HAVE_V8_FUNCTIONCALLBACKINFO
    typedef ::v8::FunctionCallbackInfo<::v8::Value> Args;

    std::string logArgs2String(const Args &args)
    {
        std::string message;
        for (int i = 0; i < args.Length(); i++) {
            ::v8::HandleScope scope(args.GetIsolate());
            ::v8::String::Utf8Value str(args[i]);
            message += *str;
        }
        return message;
    }
    void log(const Args &args)
    {
        LOG(info) << logArgs2String(args);
    }
    void error(const Args &args)
    {
        LOG(error) << logArgs2String(args);
    }
#else
    typedef ::v8::Arguments Args;

    std::string logArgs2String(const Args &args)
    {
        std::string message;
        for (int i = 0; i < args.Length(); i++) {
            ::v8::HandleScope scope;
            ::v8::String::Utf8Value str(args[i]);
            message += *str;
        }
        return message;
    }
    ::v8::Handle<::v8::Value> log(const Args &args)
    {
        LOG(info) << logArgs2String(args);
        return v8::Handle<v8::Value>();
    }
    ::v8::Handle<::v8::Value> error(const Args &args)
    {
        LOG(error) << logArgs2String(args);
        return v8::Handle<v8::Value>();
    }
#endif
}

namespace {
#ifdef HAVE_V8_WITH_MOST_CONSTRUCTORS_ISOLATE
    v8::Local<v8::String> newUtf8String(const char *data)
    {
        return v8::String::NewFromUtf8(v8::Isolate::GetCurrent(), data);
    }
#else
    v8::Local<v8::String> newUtf8String(const char *data)
    {
        return v8::String::New(data);
    }
#endif
}

JsVm::JsVm(const std::string &code)
    : m_isolate(v8::Isolate::GetCurrent())
    , m_locker(m_isolate)
    , m_global(v8::ObjectTemplate::New())
{
    v8::Handle<v8::ObjectTemplate> console = v8::ObjectTemplate::New();
    m_global->Set(newUtf8String("console"), console);

    console->Set(newUtf8String("log"),
                 v8::FunctionTemplate::New(&Js::log));
    console->Set(newUtf8String("error"),
                 v8::FunctionTemplate::New(&Js::error));

    /**
     * We can't do this inside initialization list, since we are modifying
     * "m_global", and during ths gc can update references to it, since we are
     * adding _properties_ to it.
     *
     * And also to avoid Context::Scope for every call(), just enter context
     * here, since this module already provides "box" for executing
     * user-specific code in current thread.
     */
    m_context = v8::Context::New(NULL, m_global);
    m_context->Enter();

    m_source = newUtf8String(code.c_str());
}
JsVm::~JsVm()
{
    m_context->Exit();
}

bool JsVm::init()
{
    v8::Handle<v8::Script> script = v8::Script::Compile(m_source);
    if (!*script) {
        fillTryCatch();
        return false;
    }

    v8::Handle<v8::Value> sourceResult = script->Run();
    if (sourceResult.IsEmpty()) {
        fillTryCatch();
        return false;
    }

    m_function = v8::Handle<v8::Function>::Cast(sourceResult);
    return true;
}

void JsVm::call(const Db::Interface::Key &key, const Db::Interface::Value &value)
{
    v8::Local<v8::Value> args[] = {
        newUtf8String(key.c_str()),
        newUtf8String(value.c_str())
    };

    m_function->Call(m_context->Global(), 2, args);
}

void JsVm::fillTryCatch()
{
    v8::Handle<v8::Value> exception = m_trycatch.Exception();
    v8::String::AsciiValue exceptionMessage(exception);
    LOG(error) << *exceptionMessage;
}
