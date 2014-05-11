
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
