
/**
 * This file is part of the boostcache package.
 *
 * (c) Azat Khuzhin <a3at.mail@gmail.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

#include "commandhandler.h"

#include "commands.h"
#include "util/log.h"

#include <boost/spirit/include/qi.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/format.hpp>


const char *CommandHandler::REPLY_FALSE              = ":0\r\n";
const char *CommandHandler::REPLY_TRUE               = ":1\r\n";
const char *CommandHandler::REPLY_NIL                = "$-1\r\n";
const char *CommandHandler::REPLY_OK                 = "+OK\r\n";
const char *CommandHandler::REPLY_ERROR              = "-ERR\r\n";
const char *CommandHandler::REPLY_ERROR_NOTSUPPORTED = "-ERR Not supported\r\n";


namespace qi = boost::spirit::qi;

std::string CommandHandler::toReplyString(const std::string& string)
{
    return str(boost::format("$%i\r\n%s\r\n") % string.size() % string);
}
std::string CommandHandler::toErrorReplyString(const std::string& string)
{
    return str(boost::format("-ERR %s\r\n") % string);
}


bool CommandHandler::feedAndParseCommand(const char *buffer, size_t size)
{
    m_commandString.append(buffer, size);
    LOG(trace) << "Try to read/parser command(" << m_commandString.length() << ") "
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

bool CommandHandler::parseInline(std::istringstream& stream)
{
    m_type = INLINE;

    std::getline(stream, m_lineBuffer);
    if (!handleStreamIsValid(stream)) {
        return false;
    }
    m_commandOffset = stream.tellg();

    boost::trim_right(m_lineBuffer);
    splitString(m_lineBuffer, m_commandArguments);

    if (!m_commandArguments.size() || !m_commandArguments[0].size() /* no command */) {
        return false;
    }

    LOG(trace) << "Have " << m_commandArguments.size() << " arguments, "
               << "for " << this << " (inline)";

    return true;
}

bool CommandHandler::parseNumberOfArguments(std::istringstream& stream)
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

    m_commandArguments.reserve(m_numberOfArguments);
    m_numberOfArgumentsLeft = m_numberOfArguments;
    m_commandOffset = stream.tellg();

    LOG(trace) << "Have " << m_numberOfArguments << " arguments, "
               << "for " << this << " (bulk)";

    return true;
}

bool CommandHandler::parseArguments(std::istringstream& stream)
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
        LOG(trace) << "Reading " << m_lastArgumentLength << " bytes, for " << this;

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
        LOG(trace) << "Saving " << argument << " argument, for " << this;
        m_commandArguments.push_back(argument);
        m_lastArgumentLength = -1;

        // Update some counters/offsets
        --m_numberOfArgumentsLeft;
        m_commandOffset = stream.tellg();
    }

    return !m_numberOfArgumentsLeft;
}

void CommandHandler::executeCommand()
{
    LOG(trace) << "Execute new command, for " << this;

    /**
     * TODO: We need here something like vector::pop() method,
     * but it is slow for vectors.
     * So need to think on it.
     * As a temporary decision for one hashtable db I will just not use 0 index.
     */

    Commands& commands = TheCommands::instance();
    m_finishCallback((commands.find(m_commandArguments[0],
                                    m_commandArguments.size() - 1))
    (
         m_commandArguments
    ));

    reset();
}

std::string CommandHandler::toString() const
{
    std::string arguments;
    int i;
    for_each(m_commandArguments.begin(), m_commandArguments.end(),
             [&arguments, &i] (std::string argument)
             {
                  arguments += "'";
                  arguments += argument;
                  arguments += "'" "\n";
             }
    );

    return arguments;
}

void CommandHandler::reset()
{
    m_type = NOT_SET;
    m_commandString.clear();
    m_commandOffset = 0;
    m_numberOfArguments = -1;
    m_numberOfArgumentsLeft = -1;
    m_lastArgumentLength = -1;

    m_commandArguments.clear();
}

bool CommandHandler::handleStreamIsValid(const std::istringstream& stream)
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

void CommandHandler::splitString(const std::string &string, std::vector<std::string>& destination)
{
    const char *cString = string.c_str();
    const char *lastCString = cString;
    while (true) {
        if (*cString == ' ' || *cString == '\0') {
            destination.push_back(std::string(lastCString, cString - lastCString));

            if (*cString == '\0') {
                break;
            }

            // Avoid copying whitespace
            lastCString = (cString + 1);
        }

        ++cString;
    }
}