
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
// TODO: we need to use this in production
// #define BOOST_LOG_NO_THREADS
#include <boost/log/trivial.hpp>

#define LOG(...) BOOST_LOG_TRIVIAL(__VA_ARGS__)

/**
 * TODO: This is not so pretty as could be
 */
namespace Util
{
    enum LoggerConstants {
        LOGGER_ROTATION_SIZE = (10 * 1024 * 1024)
    };

    void installLoggerLevel(int level);
    void installLoggerFile(const std::string &fileFormat);
}