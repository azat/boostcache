
/**
 * This file is part of the boostcache package.
 *
 * (c) Azat Khuzhin <a3at.mail@gmail.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

#include "server/options.h"
#include "util/log.h"
#include "kernel/net/commandserver.h"

#include <exception>
#include <unistd.h>
#include <v8.h>

using namespace Server;

namespace Js
{

std::string logArgs2String(const ::v8::Arguments &args)
{
    std::string message;
    for (int i = 0; i < args.Length(); i++) {
        ::v8::HandleScope scope;
        ::v8::String::Utf8Value str(args[i]);
        message += *str;
    }
    return message;
}
::v8::Handle<::v8::Value> log(const ::v8::Arguments &args)
{
    LOG(info) << logArgs2String(args);
    return v8::Handle<v8::Value>();
}
::v8::Handle<::v8::Value> error(const ::v8::Arguments &args)
{
    LOG(error) << logArgs2String(args);
    return v8::Handle<v8::Value>();
}

}

int main(int argc, char **argv)
{
    Options options;
    options.parse(argc, argv);

    // TODO: VERIFY macros
    if (options.getValue("fork") && (daemon(1 /* nochdir */, 0 /* noclose */) == -1)) {
        return EXIT_FAILURE;
    }

    v8::V8::Initialize();
    LOG(trace) << "v8 vm initialized (" << v8::V8::GetVersion() << ")";

    v8::HandleScope scope;
    v8::Handle<v8::ObjectTemplate> global = v8::ObjectTemplate::New();
    v8::Handle<v8::ObjectTemplate> console = v8::ObjectTemplate::New();
    global->Set(v8::String::New("console"), console);
    v8::Handle<v8::FunctionTemplate> logFunction =
        v8::FunctionTemplate::New(&Js::log);
    console->Set(v8::String::New("log"), logFunction);
    v8::Handle<v8::FunctionTemplate> errorFunction =
        v8::FunctionTemplate::New(&Js::error);
    console->Set(v8::String::New("error"), errorFunction);

    try {
        CommandServer server(CommandServer::Options(
            options.getValue<int>("port"),
            options.getValue<std::string>("host"),
            options.getValue<std::string>("socket"),
            options.getValue<int>("workers")
        ));
        server.start();
    } catch (const std::exception &exception) {
        LOG(fatal) << exception.what();

        return EXIT_FAILURE;
    }

    LOG(trace) << "Freeing v8 vm resources";
    v8::V8::Dispose();

    return EXIT_SUCCESS;
}
