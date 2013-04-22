
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

#include <unordered_map>
#include <string>

/**
 * @brief Hash table using std::unordered_map
 */

namespace Db
{
    class HashTable : public Interface
    {
    public:
        typedef std::unordered_map<std::string, Value> Table;

        HashTable();

        std::string get(const Command::Arguments& arguments);
        std::string set(const Command::Arguments& arguments);
        std::string del(const Command::Arguments& arguments);

    private:
        Table m_table;
    };
}