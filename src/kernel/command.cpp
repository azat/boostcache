
/**
 * This file is part of the boostcache package.
 *
 * (c) Azat Khuzhin <a3at.mail@gmail.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

#include "command.h"
#include "util/log.h"

#include <boost/spirit/include/qi.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/split.hpp>

namespace qi = boost::spirit::qi;

bool Command::feedAndParseCommand(const char *buffer)
{
    m_commandString += buffer;
    LOG(debug) << "Try to read/parser command(" << m_commandString.length() << ") "
               "with " << m_numberOfArguments << " arguments, "
               "for " << this;

    std::istringstream stream(m_commandString);
    stream.seekg(m_commandOffset);
    if (!handleStreamIsValid(stream)) {
        // Need to feed more data
        return true;
    }

    // Try to read new command
    if (m_numberOfArguments < 0) {
        // We have inline request, because it is not start with '*'
        if (m_commandString[0] != '*') {
            if (!parseInline(stream)) {
                reset();
                return true;
            }
        } else if (!parseNumberOfArguments(stream)) {
            // Need to feed more data
            return true;
        }
    }

    if ((m_type == MULTI_BULK) && !parseArguments(stream)) {
        // Need to feed more data
        return true;
    }

    executeCommand();
    return false;
}

bool Command::parseInline(std::istringstream& stream)
{
    m_type = INLINE;

    std::getline(stream, m_lineBuffer);
    if (!handleStreamIsValid(stream)) {
        return false;
    }
    m_commandOffset = stream.tellg();

    boost::trim_right(m_lineBuffer);
    boost::split(commandArguments, m_lineBuffer, boost::algorithm::is_space());

    if (!commandArguments.size() || !commandArguments[0].size() /* no command */) {
        return false;
    }

    LOG(info) << "Have " << commandArguments.size() << " arguments, "
              << "for " << this << " (inline)";

    return true;
}

bool Command::parseNumberOfArguments(std::istringstream& stream)
{
    m_type = MULTI_BULK;

    std::getline(stream, m_lineBuffer);
    qi::parse(m_lineBuffer.begin(), m_lineBuffer.end(),
              '*' >> qi::int_ >> "\r",
              m_numberOfArguments);

    if (m_numberOfArguments < 0) {
        LOG(debug) << "Don't have number of arguments, for " << this;

        reset();
        return false;
    }

    commandArguments.reserve(m_numberOfArguments);
    m_numberOfArgumentsLeft = m_numberOfArguments;
    m_commandOffset = stream.tellg();

    LOG(info) << "Have " << m_numberOfArguments << " arguments, "
              << "for " << this << " (bulk)";

    return true;
}

bool Command::parseArguments(std::istringstream& stream)
{
    char crLf[2];
    char *argument = NULL;
    int argumentLength = 0;

    while (m_numberOfArgumentsLeft && std::getline(stream, m_lineBuffer)) {
        if (!qi::parse(m_lineBuffer.begin(), m_lineBuffer.end(),
                       '$' >> qi::int_ >> "\r",
                       m_lastArgumentLength)
        ) {
            LOG(debug) << "Can't find valid argument length, for " << this;
            reset();
            break;
        }
        LOG(debug) << "Reading " << m_lastArgumentLength << " bytes, for " << this;

        if (argumentLength < m_lastArgumentLength) {
            argument = (char *)realloc(argument, argumentLength + 1 /* NULL byte */
                                       + m_lastArgumentLength);
            argumentLength += m_lastArgumentLength;
        }
        stream.read(argument, m_lastArgumentLength);
        argument[m_lastArgumentLength] = 0;
        if (!handleStreamIsValid(stream)) {
            break;
        }
        // Read CRLF separator
        stream.read(crLf, 2);
        if (!handleStreamIsValid(stream)) {
            break;
        }
        if (memcmp(crLf, "\r\n", 2) != 0) {
            LOG(debug) << "Malfomed end of argument, for " << this;
            reset();
            break;
        }
        // Save command argument
        LOG(debug) << "Saving " << argument << " argument, for " << this;
        commandArguments.push_back(argument);
        m_lastArgumentLength = -1;

        // Update some counters/offsets
        --m_numberOfArgumentsLeft;
        m_commandOffset = stream.tellg();
    }

    return !m_numberOfArgumentsLeft;
}

void Command::executeCommand()
{
    LOG(info) << "Execute new command, for " << this;

    m_finishCallback(toString());

    reset();
}

std::string Command::toString() const
{
    std::string arguments;
    int i;
    for_each(commandArguments.begin(), commandArguments.end(),
             [&arguments, &i] (std::string argument)
             {
                  arguments += ++i;
                  arguments += " " "'";
                  arguments += argument;
                  arguments += "'" "\n";
             }
    );

    return arguments;
}

void Command::reset()
{
    m_type = NOT_SET;
    m_commandString.clear();
    m_commandOffset = 0;
    m_numberOfArguments = -1;
    m_numberOfArgumentsLeft = -1;
    m_lastArgumentLength = -1;

    commandArguments.clear();
}

bool Command::handleStreamIsValid(const std::istringstream& stream)
{
    if (stream.good()) {
        return true;
    }

    /**
     * TODO: add some internal counters for failover,
     * or smth like this
     */
    if (stream.bad()) {
        LOG(debug) << "Bad stream, for " << this;
        reset();
    }

    return false;
}