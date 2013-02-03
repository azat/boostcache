/**
 * Exception
 * With printStackTrace() in constructor
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

#pragma once

#include <string>

#include "kernel/exception.h"
#include "util/stacktrace.h"

class WithTraceException : public Exception {
public:
    WithTraceException(std::string message = "", long code = 0) : Exception(message, code) {
        Util::printStackTrace();
    }
};
