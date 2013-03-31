
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
#include <unordered_map>
#include <vector>
#include <string>

/**
 * @brief Hash table using std::unordered_map
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
    class HashTable : boost::noncopyable
    {
    public:
        typedef std::string Value;
        typedef std::unordered_map<std::string, Value> Table;

        HashTable();

        std::string get(const Command::Arguments& arguments);
        std::string set(const Command::Arguments& arguments);
        std::string del(const Command::Arguments& arguments);

    private:
        Table m_table;

        static std::string valueToReplyString(const Value& value);
    };
}