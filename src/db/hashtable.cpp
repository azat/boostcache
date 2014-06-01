
/**
 * This file is part of the boostcache package.
 *
 * (c) Azat Khuzhin <a3at.mail@gmail.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */


#include "hashtable.h"
#include "server/jsvm.h"
#include "kernel/exception.h"
#include "util/log.h"


namespace Db
{
    HashTable::HashTable()
        : Interface()
    {
    }

    std::string HashTable::get(const CommandHandler::Arguments &arguments)
    {
        // get shared lock
        boost::shared_lock<boost::shared_mutex> lock(m_access);

        Table::const_iterator value = m_table.find(arguments[1]);
        if (value == m_table.end()) {
            return CommandHandler::REPLY_NIL;
        }
        return CommandHandler::toReplyString(value->second);
    }

    std::string HashTable::set(const CommandHandler::Arguments &arguments)
    {
        // get exclusive lock
        boost::upgrade_lock<boost::shared_mutex> lock(m_access);
        boost::upgrade_to_unique_lock<boost::shared_mutex> uniqueLock(lock);

        m_table[arguments[1] /* key */] = arguments[2] /* value */;

        return CommandHandler::REPLY_OK;
    }

    std::string HashTable::del(const CommandHandler::Arguments &arguments)
    {
        // get exclusive lock
        boost::upgrade_lock<boost::shared_mutex> lock(m_access);
        boost::upgrade_to_unique_lock<boost::shared_mutex> uniqueLock(lock);

        Table::const_iterator value = m_table.find(arguments[1]);
        if (value == m_table.end()) {
            return CommandHandler::REPLY_FALSE;
        }

        m_table.erase(value);
        return CommandHandler::REPLY_TRUE;
    }

    std::string HashTable::foreach(const CommandHandler::Arguments &arguments)
    {
        JsVm vm(arguments[1]);
        if (!vm.init()) {
            return CommandHandler::REPLY_ERROR;
        }

        // get exclusive lock
        boost::upgrade_lock<boost::shared_mutex> lock(m_access);
        // XXX: support non-atomic mode
        boost::upgrade_to_unique_lock<boost::shared_mutex> uniqueLock(lock);

        for (std::pair<const Key, Value> &i : m_table) {
            std::string key(i.first);
            std::string &value = i.second;

            try {
                value = vm.call(key, value);
            } catch (const Exception &e) {
                LOG(error) << e.getMessage();
                LOG(error) << "Will not continue";
                return CommandHandler::REPLY_ERROR;
            }
        }

        return CommandHandler::REPLY_TRUE;
    }
}
