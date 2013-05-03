
/**
 * This file is part of the boostcache package.
 *
 * (c) Azat Khuzhin <a3at.mail@gmail.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */


#include "hashtable.h"


namespace Db
{
    HashTable::HashTable()
        : Interface()
    {
    }

    std::string HashTable::get(const Command::Arguments& arguments)
    {
        // get shared lock
        boost::shared_lock<boost::shared_mutex> lock(m_access);

        Table::const_iterator value = m_table.find(arguments[1]);
        if (value == m_table.end()) {
            return Command::REPLY_NIL;
        }
        return valueToReplyString(value->second);
    }

    std::string HashTable::set(const Command::Arguments& arguments)
    {
        // get exclusive lock
        boost::upgrade_lock<boost::shared_mutex> lock(m_access);
        boost::upgrade_to_unique_lock<boost::shared_mutex> uniqueLock(lock);

        m_table[arguments[1] /* key */] = arguments[2] /* value */;

        return Command::REPLY_OK;
    }

    std::string HashTable::del(const Command::Arguments& arguments)
    {
        // get exclusive lock
        boost::upgrade_lock<boost::shared_mutex> lock(m_access);
        boost::upgrade_to_unique_lock<boost::shared_mutex> uniqueLock(lock);

        Table::const_iterator value = m_table.find(arguments[1]);
        if (value == m_table.end()) {
            return Command::REPLY_FALSE;
        }

        m_table.erase(value);
        return Command::REPLY_TRUE;
    }
}