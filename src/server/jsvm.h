
/**
 * This file is part of the boostcache package.
 *
 * (c) Azat Khuzhin <a3at.mail@gmail.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

#pragma once

#include "db/interface.h"
#include <string>
#include <v8.h>

/**
 * @brief Wrapper for JS VM (v8).
 *
 * Create and run this module in the same thread.
 *
 * XXX: adopt to the newer version of v8 (need Isolate* in constructor)
 * XXX: more debugging/profiling/testing
 */
class JsVm
{
public:
    JsVm(const std::string &code);
    ~JsVm();
    bool init();

    void call(const Db::Interface::Key &key, const Db::Interface::Value &value);

private:
    v8::Isolate *m_isolate;
    v8::Isolate::Scope m_isolateScope;
    v8::Locker m_locker;

    v8::TryCatch m_trycatch;

    v8::HandleScope m_scope;
    v8::Handle<v8::ObjectTemplate> m_global;
    v8::Handle<v8::Context> m_context;

    v8::Handle<v8::String> m_source;
    v8::Handle<v8::Function> m_function;


    void fillTryCatch();
};
