
/**
 * This file is part of the boostcache package.
 *
 * (c) Azat Khuzhin <a3at.mail@gmail.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */


#pragma once

#include "kernel/command.h" // Command::Arguments

#include <boost/noncopyable.hpp>
#include <string>

/**
 * @brief Db interface
 *
 * TODO: make it abstract (I'm not sure about vtable for now)
 *
 * TODO: make a abstract factory, and use virtual methods? (vtable lookup)
 *
 * TODO: make private constructor and a static method,
 * that will do all stuff, including different db names
 *
 * TODO: make multi-value (value something like std::vector)
 *
 * TODO: see Command::executeCommand() notes
 */

namespace Db
{
    class Interface : boost::noncopyable
    {
    public:
        typedef std::string Value;
        typedef std::string Key;

        Interface();

        /**
         * TODO: make this virtual
         * (measure performance regression)
         */
        std::string get(const Command::Arguments& arguments);
        std::string set(const Command::Arguments& arguments);
        std::string del(const Command::Arguments& arguments);

    protected:
        static std::string valueToReplyString(const Value& value);
    };
}