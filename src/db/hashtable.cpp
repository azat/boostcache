
/**
 * This file is part of the boostcache package.
 *
 * (c) Azat Khuzhin <a3at.mail@gmail.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */


#include "hashtable.h"

#include <boost/format.hpp>


namespace Db
{
    HashTable::HashTable()
    {
    }

    std::string HashTable::get(const Command::Arguments& arguments)
    {
        Table::const_iterator value = m_table.find(arguments[0]);
        if (value == m_table.end()) {
            return Command::REPLY_NIL;
        }
        return valueToReplyString(value->second);
    }

    std::string HashTable::set(const Command::Arguments& arguments)
    {
        m_table[arguments[0] /* key */] = arguments[1] /* value */;
        return Command::REPLY_OK;
    }

    std::string HashTable::del(const Command::Arguments& arguments)
    {
        Table::const_iterator value = m_table.find(arguments[0]);
        if (value == m_table.end()) {
            return Command::REPLY_FALSE;
        }

        m_table.erase(value);
        return Command::REPLY_TRUE;
    }

    std::string HashTable::valueToReplyString(const Value& value)
    {
        return value;
    }
}