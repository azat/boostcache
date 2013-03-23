
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
    m_socket.async_read_some(boost::asio::buffer(m_buffer, MAX_BUFFER_LENGTH),
                             boost::bind(&Session::handleRead, this,
                                         boost::asio::placeholders::error,
                                         boost::asio::placeholders::bytes_transferred));
}

void Session::handleRead(const boost::system::error_code& error, size_t bytesTransferred)
{
    if (error) {
        delete this;
        return;
    }

    boost::asio::buffer(m_buffer, bytesTransferred);
    handleReadParseCommand();
}

void Session::handleWrite(const boost::system::error_code& error)
{
    if (error) {
        delete this;
        return;
    }

    m_socket.async_read_some(boost::asio::buffer(m_buffer, MAX_BUFFER_LENGTH),
                             boost::bind(&Session::handleRead, this,
                                         boost::asio::placeholders::error,
                                         boost::asio::placeholders::bytes_transferred));
}

/**
 * TODO: speedup parsing
 * TODO: more error-friendly parsing
 * TODO: add logging before every reset() call
 */
void Session::handleReadParseCommand()
{
    m_commandString += m_buffer;

    std::string line;
    std::istringstream stream(m_commandString);
    stream.seekg(m_commandOffset);

    // Number of arguments
    if (m_numberOfArguments < 0) {
        std::getline(stream, line);
        if (!parse(line.begin(),
                   line.end(),
                   '*' >> int_ >> "\r\n",
                   m_numberOfArguments)) {
            reset();
            return;
        }
        if (m_numberOfArguments < 0) {
            reset();
            return;
        }

        commandArguments.reserve(m_numberOfArguments);
        m_numberOfArgumentsLeft = m_numberOfArguments;

        LOG(info) << "Have " << m_numberOfArgumentsLeft
                  << " number of arguments for " << this;
    }

    char *argument = NULL;
    int argumentLength = 0;
    while (m_numberOfArgumentsLeft && std::getline(stream, line)) {
        if (m_lastArgumentLength < 0
            && !parse(line.begin(),
                      line.end(),
                      '$' >> int_ >> "\r\n",
                      m_lastArgumentLength)) {
            return;
        }
        LOG(info) << "Reading argument for " << this;

        if (argumentLength < m_lastArgumentLength) {
            argument = (char *)realloc(argument, argumentLength + m_lastArgumentLength);
            argumentLength += m_lastArgumentLength;
        }
        stream.get(argument, m_lastArgumentLength);
        if (!stream.good()) {
            reset();
            return;
        }
        commandArguments.push_back(argument);
        m_lastArgumentLength = -1;

        --m_numberOfArgumentsLeft;
    }

    if (!m_numberOfArgumentsLeft) {
        handleCommand();
    }

    m_commandOffset = stream.tellg();
}

void Session::handleCommand()
{
    LOG(info) << "Handling new command on " << this;

    writeCommand();

    reset();
}

void Session::writeCommand()
{
    std::string arguments;
    int i;
    for_each(commandArguments.begin(), commandArguments.end(), [&arguments, &i] (std::string argument)
    {
        arguments += i++;
        arguments += " ";
        arguments += argument;
        arguments += "\r\n";
    });

    boost::asio::async_write(m_socket,
                             boost::asio::buffer(arguments),
                             boost::bind(&Session::handleWrite, this,
                                         boost::asio::placeholders::error));
}

void Session::reset()
{
    m_commandString = "";
    m_commandOffset = -1;
    m_numberOfArguments = -1;
    m_numberOfArgumentsLeft = -1;
    m_lastArgumentLength = -1;

    commandArguments.clear();
}