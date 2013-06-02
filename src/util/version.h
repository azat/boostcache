
/**
 * This file is part of the boostcache package.
 *
 * (c) Azat Khuzhin <a3at.mail@gmail.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

#pragma once

#include "config.h"

#include <string>

namespace Util
{
    /**
     * TODO: convert version to numeric type (version macros too)
     */
    float version();
    std::string versionString(const bool verbose = false);
}
