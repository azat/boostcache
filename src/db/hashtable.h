
/**
 * This file is part of the boostcache package.
 *
 * (c) Azat Khuzhin <a3at.mail@gmail.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */


#pragma once

#include "db/interface.h"

#include <boost/thread/pthread/shared_mutex.hpp>
#include <unordered_map>
#include <string>


namespace Db
{
    /**
     * @brief Hash table using std::unordered_map
     *
     * Thread-safe (TODO: improve thread-safe support)
     */
    class HashTable : public Interface
    {
    public:
        HashTable();

        std::string get(const CommandHandler::Arguments& arguments);
        std::string set(const CommandHandler::Arguments& arguments);
        std::string del(const CommandHandler::Arguments& arguments);

    private:
        typedef std::unordered_map<Key, Value> Table;
        Table m_table;

        /**
         * TODO: maybe move to ThreadSafe wrapper
         */
        boost::shared_mutex m_access;
    };
}