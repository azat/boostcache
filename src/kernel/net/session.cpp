
/**
 * This file is part of the boostcache package.
 *
 * (c) Azat Khuzhin <a3at.mail@gmail.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

#include "session.h"
#include "util/log.h"

#include <boost/bind.hpp>
#include <boost/spirit/include/qi.hpp>
#include <sstream>


using namespace boost::spirit::qi;


void Session::start()
{
    asyncRead();
}

void Session::asyncRead()
{
    m_socket.async_read_some(boost::asio::buffer(m_buffer, MAX_BUFFER_LENGTH),
                             boost::bind(&Session::handleRead, this,
                                         boost::asio::placeholders::error,
                                         boost::asio::placeholders::bytes_transferred));
}

void Session::asyncWrite(const std::string& message)
{
    boost::asio::async_write(m_socket,
                             boost::asio::buffer(message),
                             boost::bind(&Session::handleWrite, this,
                                         boost::asio::placeholders::error));
}

void Session::handleRead(const boost::system::error_code& error, size_t /* bytesTransferred */)
{
    if (error) {
        delete this;
        return;
    }

    handleReadParseCommand();
}

void Session::handleWrite(const boost::system::error_code& error)
{
    if (error) {
        delete this;
        return;
    }

    asyncRead();
}

/**
 * TODO: speedup parsing
 * TODO: more error-friendly parsing
 * TODO: add logging before every reset() call
 */
void Session::handleReadParseCommand()
{
    LOG(debug) << "Try to read/parser command " << m_buffer
               << "with " << m_numberOfArguments << " arguments, "
               "for " << this;
    m_commandString += m_buffer;

    std::string line;
    std::istringstream stream(m_commandString);
    stream.seekg(m_commandOffset);

    // Number of arguments
    if (m_numberOfArguments < 0) {
        std::getline(stream, line);
        parse(line.begin(), line.end(),
              '*' >> int_ >> "\r",
              m_numberOfArguments);

        if (m_numberOfArguments < 0) {
            LOG(debug) << "Don't have number of arguments, for " << this;

            reset();
            asyncRead();
            return;
        }

        commandArguments.reserve(m_numberOfArguments);
        m_numberOfArgumentsLeft = m_numberOfArguments;
        m_commandOffset = stream.tellg();

        LOG(info) << "Have " << m_numberOfArguments << " number of arguments, "
                  << "for " << this;
    }

    char crLf[2];
    char *argument = NULL;
    int argumentLength = 0;
    while (m_numberOfArgumentsLeft && std::getline(stream, line)) {
        if (!parse(line.begin(), line.end(),
                   '$' >> int_ >> "\r",
                   m_lastArgumentLength)
        ) {
            LOG(debug) << "Can't find valid argument length, for " << this;
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
        if (!stream.good()) {
            /**
             * TODO: add some internal counters for failover,
             * or smth like this
             */
            if (stream.bad()) {
                LOG(debug) << "Bad stream, for " << this;
                reset();
            }
            break;
        }
        // Read CRLF separator
        stream.read(crLf, 2);
        if (!stream.good()) {
            /**
             * TODO: add some internal counters for failover,
             * or smth like this
             */
            if (stream.bad()) {
                LOG(debug) << "Bad stream, for " << this;
                reset();
            }
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

    if (!m_numberOfArgumentsLeft) {
        handleCommand();
        // Will be called asyncRead() in write callback
    } else {
        asyncRead();
    }
}

void Session::handleCommand()
{
    LOG(info) << "Handling new command, for " << this;

    writeCommand();

    reset();
}

void Session::writeCommand()
{
    std::string arguments;
    int i;
    for_each(commandArguments.begin(), commandArguments.end(),
             [&arguments, &i] (std::string argument)
             {
                  arguments += ++i;
                  arguments += " ";
                  arguments += argument;
                  arguments += "\n";
             }
    );

    asyncWrite(arguments);
}

void Session::reset()
{
    m_commandString = "";
    m_commandOffset = 0;
    m_numberOfArguments = -1;
    m_numberOfArgumentsLeft = -1;
    m_lastArgumentLength = -1;

    commandArguments.clear();
}