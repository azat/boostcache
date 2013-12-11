
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

#include <boost/format.hpp>
#include <cstring>


const char *CommandHandler::REPLY_FALSE              = ":0\r\n";
const char *CommandHandler::REPLY_TRUE               = ":1\r\n";
const char *CommandHandler::REPLY_NIL                = "$-1\r\n";
const char *CommandHandler::REPLY_OK                 = "+OK\r\n";
const char *CommandHandler::REPLY_ERROR              = "-ERR\r\n";
const char *CommandHandler::REPLY_ERROR_NOTSUPPORTED = "-ERR Not supported\r\n";


std::string CommandHandler::toReplyString(const std::string &string)
{
    return str(boost::format("$%i\r\n%s\r\n") % string.size() % string);
}
std::string CommandHandler::toErrorReplyString(const std::string &string)
{
    return str(boost::format("-ERR %s\r\n") % string);
}
std::string CommandHandler::toInlineReplyString(const std::string &string)
{
    return str(boost::format("+%s\r\n") % string);
}


bool CommandHandler::feedAndParseCommand(const char *buffer, size_t size)
{
    m_commandString.append(buffer, size);

    LOG(trace) << "Try to read/parser command(" << m_commandString.length() << ") "
               "with " << m_numberOfArguments << " arguments, "
               "for " << this;

    const char *begin = &m_commandString.c_str()[ m_commandOffset ];
    const char *end   = &m_commandString.c_str()[ m_commandString.size() - m_commandOffset ];
    if (!begin) {
        LOG(trace) << "Parse: need more data, for " << this;
        return true;
    }

    // Try to read new command
    if (m_numberOfArguments < 0) {
        // We have inline request, because it is not start with '*'
        if (*begin != '*') {
            if (!parseInline(begin, end)) {
                reset();
                return true;
            }
        } else if (!(begin = parseNumberOfArguments(begin, end))) {
            LOG(trace) << "Number of arguments: need more data, for " << this;
            return true;
        }
    }

    if ((m_type == MULTI_BULK) && !parseArguments(begin, end)) {
        LOG(trace) << "Arguments: need more data, for " << this;
        return true;
    }

    executeCommand();
    return false;
}

bool CommandHandler::parseInline(const char *begin, const char *end)
{
    m_type = INLINE;

    const char *lfPtr = (const char *)memchr((const void *)begin, '\n', end - begin);
    if (!lfPtr) {
        LOG(debug) << "LF not found, for " << this;
        return false;
    }
    m_commandOffset += (lfPtr - begin);

    // trim
    if (*(lfPtr-1) == '\r') {
        --lfPtr;
    }
    split(begin, lfPtr, m_commandArguments);

    if (!m_commandArguments.size() || !m_commandArguments[0].size() /* no command */) {
        return false;
    }

    LOG(trace) << "Have " << m_commandArguments.size() << " arguments, "
               << "for " << this << " (inline)";

    return true;
}

const char* CommandHandler::parseNumberOfArguments(const char *begin, const char *end)
{
    m_type = MULTI_BULK;

    const char *lfPtr = (const char *)memchr((const void *)begin, '\n', end - begin);
    if (!lfPtr) {
        LOG(debug) << "LF not found, for " << this;
        return nullptr;
    }

    if ((*begin != '*') ||
        ((m_numberOfArguments = toInt(begin + 1, lfPtr)) <= 0)) {
        LOG(debug) << "Don't have number of arguments, for " << this;
        reset();
        return nullptr;
    }
    ++lfPtr; // Seek LF

    m_commandArguments.reserve(m_numberOfArguments);
    m_numberOfArgumentsLeft = m_numberOfArguments;

    m_commandOffset += (lfPtr - begin);

    LOG(trace) << "Have " << m_numberOfArguments << " arguments, "
               << "for " << this << " (bulk)";

    return lfPtr;
}

bool CommandHandler::parseArguments(const char *begin, const char *end)
{
    const char *c = begin;
    const char *lfPtr;
    size_t prevCommandOffset = m_commandOffset;

    while (m_numberOfArgumentsLeft &&
           (lfPtr = (const char *)memchr((const void *)c, '\n', end - c))) {
        if ((*c != '$') || ((m_lastArgumentLength = toInt(c + 1, lfPtr)) <= 0)) {
            LOG(debug) << "Can't find valid argument length, for " << this;
            reset();
            break;
        }
        ++lfPtr; // Seek LF
        LOG(trace) << "Reading " << m_lastArgumentLength << " bytes, "
                   << "for " << this << " (bulk)";

        if ((m_lastArgumentLength + 2 /* CRLF */) > (end - lfPtr)) {
            break;
        }
        if (memcmp(lfPtr + m_lastArgumentLength, "\r\n", 2) != 0) {
            LOG(debug) << "Malfomed end of argument, for " << this << " (bulk)";
            reset();
            break;
        }

        // Save command argument
        m_commandArguments.push_back(std::string(lfPtr, m_lastArgumentLength));
        LOG(trace) << "Saving " << m_commandArguments.back() << " argument, "
                   << "for " << this << " (bulk)";

        // Update some counters/offsets
        c = (lfPtr + m_lastArgumentLength + 2);
        --m_numberOfArgumentsLeft;
        m_commandOffset = prevCommandOffset + (c - begin);
        m_lastArgumentLength = -1;
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

    Commands &commands = TheCommands::instance();
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
    /**
     * This is test support of bulk write and then bulk read.
     * It means that first client send all requests, after this
     * read all responses.
     *
     * TODO: This is very un-optimal, since erase()
     * will do memmove() I think.
     * (However it can just move pointer to the begining of string)
     *
     * TODO: after this partial erase we need something like fullReset()
     * on errors.
     *
     * TODO: we need to re-parse buffer, since it's not empty
     */
    m_commandString.erase(0, m_commandOffset);
    LOG(trace) << "Erased " << m_commandOffset;
    m_commandOffset = 0;
    m_numberOfArguments = -1;
    m_numberOfArgumentsLeft = -1;
    m_lastArgumentLength = -1;

    m_commandArguments.clear();
}

void CommandHandler::split(const char *begin, const char *end,
                           std::vector<std::string>& destination, char delimiter)
{
    const char *found;

    while (true) {
        if (found == end) {
            break;
        }

        found = (const char *)memchr((const void *)begin, delimiter, end - begin) ?: end;
        destination.push_back(std::string(begin, found - begin));

        // trim
        while (*found == delimiter) {
            ++found;
        }

        begin = found;
    }
}

int CommandHandler::toInt(const char *begin, const char *end)
{
    int num = 0;

    if (*(end - 1) != '\r') {
        return -1;
    }

    while (*begin != '\r') {
        num = (num * 10) + (*begin - '0');
        ++begin;
    }

    return num;
}
