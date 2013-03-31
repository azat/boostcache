
/**
 * This file is part of the boostcache package.
 *
 * (c) Azat Khuzhin <a3at.mail@gmail.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

#include "commands.h"
#include "util/log.h"

#include <boost/format.hpp>

namespace PlaceHolders = std::placeholders;

Commands::Callback Commands::find(const std::string& commandName)
{
    HashTable::const_iterator command = m_commands.find(commandName);
    if (command == m_commands.end()) {
        return std::bind(&Commands::notImplementedYetCallback,
                         this, PlaceHolders::_1);
    }
    return command->second;
}

Commands::Commands()
{
    Callback defaultCallback = std::bind(&Commands::notImplementedYetCallback,
                                         this, PlaceHolders::_1);

    // Service commands
    m_commands["STATUS"] =         defaultCallback;
    m_commands["SHUTDOWN"] =       defaultCallback;

    // "db" operations
    m_commands["GET"] =            defaultCallback;
    m_commands["SET"] =            defaultCallback;
    m_commands["DEL"] =            defaultCallback;
    m_commands["INC"] =            defaultCallback;
    m_commands["DEC"] =            defaultCallback;
    m_commands["CAS"] =            defaultCallback;
}

std::string Commands::notImplementedYetCallback(const Command::Arguments& arguments)
{
    return str(boost::format("-ERR %s is not implemented\n") % arguments[0]);
}