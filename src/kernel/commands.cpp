
/**
 * This file is part of the boostcache package.
 *
 * (c) Azat Khuzhin <a3at.mail@gmail.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

#include "commands.h"
#include "util/compiler.h"
#include "util/version.h"

#include <boost/format.hpp>

namespace PlaceHolders = std::placeholders;

#define ADD_COMMAND(callback, objectPtr, argsNum) \
    CallbackInfo(std::bind(callback, objectPtr, PlaceHolders::_1), argsNum);

Commands::Callback Commands::find(const std::string &commandName,
                                  int numberOfArguments) const
{
    HashTable::const_iterator command = m_commands.find(commandName);

    if (command == m_commands.end()) {
        return std::bind(&Commands::notImplementedYetCallback,
                         PlaceHolders::_1);
    }

    const int expectedArguments = command->second.numberOfArguments;
    if ((expectedArguments >= 0) && (expectedArguments != numberOfArguments)) {
        return std::bind(malformedArgumentsCallback,
                         PlaceHolders::_1, numberOfArguments, expectedArguments);
    }

    return command->second.callback;
}

Commands::Commands()
{
    addGenericCommands();
    addDbCommands();
}

void Commands::addGenericCommands()
{
    m_commands["COMMANDS"] = ADD_COMMAND(&Commands::commandsList, this, 0);
    m_commands["PING"]     = ADD_COMMAND(&Commands::pingPong, this, 0);
    /**
     * -1 because this command has an optional argument
     * @TODO: handle optional arguments more pretty
     * @TODO: somehow told which optional arguments command have
     */
    m_commands["VERSION"]  = ADD_COMMAND(&Commands::version, this, -1);
}

void Commands::addDbCommands()
{
    /* hashtable */
    m_commands["HGET"] =   ADD_COMMAND(&Db::HashTable::get, &m_dbHashTable, 1);
    m_commands["HSET"] =   ADD_COMMAND(&Db::HashTable::set, &m_dbHashTable, 2);
    m_commands["HDEL"] =   ADD_COMMAND(&Db::HashTable::del, &m_dbHashTable, 1);
    m_commands["HFOR"] =   ADD_COMMAND(&Db::HashTable::foreach, &m_dbHashTable, 1);
    /* avltree */
    m_commands["ATGET"] =  ADD_COMMAND(&Db::AvlTree::get, &m_dbAvlTree, 1);
    m_commands["ATSET"] =  ADD_COMMAND(&Db::AvlTree::set, &m_dbAvlTree, 2);
    m_commands["ATDEL"] =  ADD_COMMAND(&Db::AvlTree::del, &m_dbAvlTree, 1);
    m_commands["ATFOR"] =  ADD_COMMAND(&Db::AvlTree::foreach, &m_dbAvlTree, 1);
}

std::string Commands::notImplementedYetCallback(const CommandHandler::Arguments &arguments)
{
    boost::format format = boost::format("%s is not implemented") % arguments[0];
    return CommandHandler::toErrorReplyString(boost::str(format));
}

std::string Commands::malformedArgumentsCallback(const CommandHandler::Arguments &arguments,
                                                 int inputArguments, int expectedArguments)
{
    boost::format format = boost::format("%s malformed number of arguments (%i vs %i)")
                                         % arguments[0]
                                         % inputArguments
                                         % expectedArguments;
    return CommandHandler::toErrorReplyString(boost::str(format));
}

std::string Commands::commandsList(const CommandHandler::Arguments &UNUSED(arguments))
{
    std::string asString;
    for (const HashTablePair &pair : m_commands) {
        asString += pair.first;
        asString += "\n";
    }
    return CommandHandler::toReplyString(asString);
}

std::string Commands::pingPong(const CommandHandler::Arguments &UNUSED(arguments))
{
    return CommandHandler::toInlineReplyString("PONG");
}

std::string Commands::version(const CommandHandler::Arguments &arguments)
{
    /**
     * TODO: add helper for checking arguments
     */
    bool verbose = (arguments.size() == 2 && arguments[1] == "VERBOSE");

    return CommandHandler::toInlineReplyString(Util::versionString(verbose));
}
