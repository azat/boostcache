
/**
 * This file is part of the boostcache package.
 *
 * (c) Azat Khuzhin <a3at.mail@gmail.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

#include "server/options.h"

using namespace Server;

int main(int argc, char **argv)
{
    Options options;
    options.parse(argc, argv);

    return EXIT_SUCCESS;
}
