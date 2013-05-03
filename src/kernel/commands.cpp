
/**
 * This file is part of the boostcache package.
 *
 * (c) Azat Khuzhin <a3at.mail@gmail.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

#include "commands.h"

#include <boost/format.hpp>

namespace PlaceHolders = std::placeholders;

Commands::Callback Commands::find(const std::string& commandName,
                                  int numberOfArguments) const
{
    HashTable::const_iterator command = m_commands.find(commandName);

    if (command == m_commands.end()) {
        return std::bind(&Commands::notImplementedYetCallback,
                         PlaceHolders::_1);
    }

    const int expectedArguments = command->second.numberOfArguments;
    if ((expectedArguments > 0) && (expectedArguments != numberOfArguments)) {
        return std::bind(malformedArgumentsCallback,
                         PlaceHolders::_1, numberOfArguments, expectedArguments);
    }

    return command->second.callback;
}

Commands::Commands()
{
    addDbCommands();
}

void Commands::addDbCommands()
{
    /* hashtable */
    m_commands["HGET"] =           CallbackInfo(std::bind(&Db::HashTable::get,
                                                          &m_dbHashTable,
                                                          PlaceHolders::_1),
                                                1);
    m_commands["HSET"] =           CallbackInfo(std::bind(&Db::HashTable::set,
                                                          &m_dbHashTable,
                                                          PlaceHolders::_1),
                                                2);
    m_commands["HDEL"] =           CallbackInfo(std::bind(&Db::HashTable::del,
                                                          &m_dbHashTable,
                                                          PlaceHolders::_1),
                                                1);
    /* avltree */
    m_commands["ATGET"] =          CallbackInfo(std::bind(&Db::AvlTree::get,
                                                          &m_dbAvlTree,
                                                          PlaceHolders::_1),
                                                1);
    m_commands["ATSET"] =          CallbackInfo(std::bind(&Db::AvlTree::set,
                                                          &m_dbAvlTree,
                                                          PlaceHolders::_1),
                                                2);
    m_commands["ATDEL"] =          CallbackInfo(std::bind(&Db::AvlTree::del,
                                                          &m_dbAvlTree,
                                                          PlaceHolders::_1),
                                                1);
}

std::string Commands::notImplementedYetCallback(const Command::Arguments& arguments)
{
    return str(boost::format("-ERR %s is not implemented\r\n") % arguments[0]);
}

std::string Commands::malformedArgumentsCallback(const Command::Arguments& arguments,
                                                 int inputArguments, int expectedArguments)
{
    return str(boost::format("-ERR %s malformed number of arguments (%i vs %i)\r\n")
               % arguments[0]
               % inputArguments
               % expectedArguments);
}