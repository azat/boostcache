
/**
 * This file is part of the boostcache package.
 *
 * StackTrace wrapper
 * win32 not supported now
 *
 * Use addr2line to convert addr to file:line
 *
 * (c) Azat Khuzhin <a3at.mail@gmail.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

#pragma once

#include <iostream>

namespace Util
{
    // Print stack trace to "os", default to std::cerr
    void printStackTrace(std::ostream &os = std::cerr);
}
