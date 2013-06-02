
/**
 * This file is part of the boostcache package.
 *
 * (c) Azat Khuzhin <a3at.mail@gmail.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

#include "version.h"
#include "util/stringify.h"

namespace Util
{
    float version()
    {
        return BOOSTCACHE_VERSION_FLOAT;
    }

    std::string versionString(const bool verbose)
    {
        std::string version = STRVAL(BOOSTCACHE_VERSION_FLOAT);
        if (verbose) {
            version += " (";
            version += BOOSTCACHE_VERSION_GIT;
            version += ")";
        }

        return version;
    }
}
