
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
        typedef std::unordered_map<Key, Value> Table;

        HashTable();

        std::string get(const Command::Arguments& arguments);
        std::string set(const Command::Arguments& arguments);
        std::string del(const Command::Arguments& arguments);

    private:
        Table m_table;

        /**
         * TODO: maybe move to ThreadSafe wrapper
         */
        boost::shared_mutex m_access;
    };
}