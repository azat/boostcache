
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

#include <boost/noncopyable.hpp>
#include <string>
#include <functional>
#include <unordered_map>

/**
 * All supported commands, includes interface to get it from hashtable
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
    typedef std::unordered_map<std::string, Callback> HashTable;

    Callback find(const std::string& commandName);

private:
    /**
     * Hashtable of all supported commands
     */
    HashTable m_commands;

    /**
     * Just print warning, that such command not supported yet.
     */
    std::string notImplementedYetCallback(const Command::Arguments& arguments);

    /******* DB ******/
    Db::HashTable m_dbHashTable;


    Commands();
};

typedef Wrapper::Singleton<Commands> TheCommands;