
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

#include <boost/noncopyable.hpp>
#include <string>
#include <functional>
#include <unordered_map>

/**
 * All supported commands, includes interface to get it from hashtable
 *
 * TODO: add conception of database/key-space
 */
class Commands : boost::noncopyable
{
    friend class Wrapper::Singleton<Commands>;

public:
    typedef std::function<std::string(const Command::Arguments&)> Callback;

    Callback find(const std::string& commandName);

private:
    /**
     * Hashtable of all supported commands
     */
    std::unordered_map<std::string, Callback> m_commands;

    /**
     * Just print warning, that such command not supported yet.
     */
    std::string notImplementedYetCallback(const Command::Arguments& arguments);


    Commands();
};

typedef Wrapper::Singleton<Commands> TheCommands;