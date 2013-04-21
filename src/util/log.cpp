
/**
 * This file is part of the boostcache package.
 *
 * (c) Azat Khuzhin <a3at.mail@gmail.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

#include "log.h"

#include <boost/log/core/core.hpp>
#include <boost/log/utility/setup/file.hpp>

namespace Util
{
    void installLoggerLevel(int level)
    {
        const int maxLevel = 5;
        if (level > maxLevel) {
            level = maxLevel;
        }
        if (level < 0) {
            level = 0;
        }

        namespace trivial = boost::log::trivial;

        /**
         * Reversing because severity_level::trace = 0, while
         * severity_level::fatal = 5, so if we have level=5, we must revers it,
         * and get level=0 (means all messages)
         */
        int severityLevel = (trivial::severity_level)(maxLevel - level);

#define CASE_SEVERITY_LEVEL(level) {                              \
    case trivial::severity_level::level:                          \
        boost::log::core::get()->set_filter(                      \
            trivial::severity >= trivial::severity_level::level   \
        );                                                        \
        break;                                                    \
}

        switch (severityLevel) {
            // 0
            CASE_SEVERITY_LEVEL(trace)
            CASE_SEVERITY_LEVEL(debug)
            CASE_SEVERITY_LEVEL(info)
            CASE_SEVERITY_LEVEL(warning)
            CASE_SEVERITY_LEVEL(error)
            // 5
            CASE_SEVERITY_LEVEL(fatal)
        }
#undef CASE_SEVERITY_LEVEL
    }
}