
/**
 * This file is part of the boostcache package.
 *
 * (c) Azat Khuzhin <a3at.mail@gmail.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

#pragma once

// Reconfigure boost/log/detail/config.hpp
#define BOOST_LOG_DYN_LINK
#include <boost/log/trivial.hpp>

#define LOG(...) BOOST_LOG_TRIVIAL(__VA_ARGS__)

namespace Util
{
    void installLoggerLevel(int level);
}