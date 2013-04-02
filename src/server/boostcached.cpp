
/**
 * This file is part of the boostcache package.
 *
 * (c) Azat Khuzhin <a3at.mail@gmail.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

#include "server/options.h"
#include "kernel/net/commandserver.h"

#include <unistd.h>

using namespace Server;

int main(int argc, char **argv)
{
    Options options;
    options.parse(argc, argv);

    if (options.getValue("fork")) {
        // TODO: VERIFY macros
        if (daemon(0, 0) == -1) {
            return EXIT_FAILURE;
        }
    }

    CommandServer server(CommandServer::Options(options.getValue<int>("port")));
    server.start();

    return EXIT_SUCCESS;
}
