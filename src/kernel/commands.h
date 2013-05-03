
/**
 * This file is part of the boostcache package.
 *
 * (c) Azat Khuzhin <a3at.mail@gmail.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */


#pragma once

#include "command.h" // Command::Arguments
#include "wrapper/singleton.h"

#include "db/hashtable.h"
#include "db/avltree.h"

#include <boost/noncopyable.hpp>
#include <string>
#include <functional>
#include <unordered_map>


/**
 * All supported commands, includes interface to get it from hashtable
 * (that store all commands)
 *
 * TODO: add conception of database/key-space
 * TODO: maybe it is not good to delegate all response for command callback?
 *
 * TODO: check number of arguments inside the commands callback
 * make method that write errors
 */
class Commands : boost::noncopyable
{
    /**
     * TODO: maybe move all properties to static,
     * and make new method to initalize it, and just use static methods.
     */
    friend class Wrapper::Singleton<Commands>;

public:
    typedef std::function<std::string(const Command::Arguments&)> Callback;

    Callback find(const std::string& commandName,
                  int numberOfArguments) const;

private:
    struct CallbackInfo
    {
        Callback callback;
        /**
         * Number of arguments that command is required
         * -1 means any number of arguments
         *
         * @TODO: make it minimum number of arguments required?
         */
        int numberOfArguments;

        CallbackInfo(Callback callback = NULL, int numberOfArguments = 0)
            : callback(callback)
            , numberOfArguments(numberOfArguments)
        {}
    };
    typedef std::unordered_map<std::string, CallbackInfo> HashTable;
    typedef std::pair<std::string, CallbackInfo> HashTablePair;
    /**
     * Hashtable of all supported commands
     */
    HashTable m_commands;

    /**
     * Just print warning, that such command not supported yet.
     */
    static std::string notImplementedYetCallback(const Command::Arguments& arguments);

    /**
     * Just print warning, about malformed arguments
     * Not enough arguments, extra arguments, e.t.c.
     */
    static std::string malformedArgumentsCallback(const Command::Arguments& arguments,
                                                  int inputArguments, int expectedArguments);

    /**
     * Print list of commands
     * @TODO: add number of arguments like "COMMAND1 arg1 arg2" and so on.
     */
    std::string commandsList(const Command::Arguments& arguments);

    /******* DB ******/
    Db::HashTable m_dbHashTable;
    Db::AvlTree m_dbAvlTree;


    Commands();
    void addGenericCommands();
    void addDbCommands();
};

typedef Wrapper::Singleton<Commands> TheCommands;