
/**
 * This file is part of the boostcache package.
 *
 * (c) Azat Khuzhin <a3at.mail@gmail.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

#pragma once

#include <boost/asio.hpp>
#include <string>
#include <vector>

/**
 * Session handler for CommandServer
 */
class Session
{
public:
    Session(boost::asio::io_service& socket)
        : m_socket(socket)
    {
        reset();
    }

    void start();

    boost::asio::ip::tcp::socket& socket()
    {
        return m_socket;
    }

private:
    void asyncRead();
    void handleRead(const boost::system::error_code& error, size_t bytesTransferred);
    void handleWrite(const boost::system::error_code& error);
    void handleReadParseCommand();
    void handleCommand();
    /**
     * Write command to client
     */
    void writeCommand();
    void reset();

    boost::asio::ip::tcp::socket m_socket;
    /**
     * TODO: We can avid this, by using buffers with std::string
     */
    enum Constants
    {
        MAX_BUFFER_LENGTH = 1 << 10 /* 1024 */
    };
    char m_buffer[MAX_BUFFER_LENGTH];

    /**
     * (TODO: move to separate module ?)
     * Format:
     *
     * *<number of arguments> CR LF
     * $<number of bytes of argument 1> CR LF
     * <argument data> CR LF
     * ...
     * $<number of bytes of argument N> CR LF
     * <argument data> CR LF
     *
     * Example:
     *
     * *3
     * $3
     * SET
     * $5
     * mykey
     * $7
     * myvalue
     *
     * More info at http://redis.io/topics/protocol
     */
    std::string m_commandString;
    int m_commandOffset;
    int m_numberOfArguments;
    /**
     * Number of command arguments left for parsing
     */
    int m_numberOfArgumentsLeft;
    int m_lastArgumentLength;
    std::vector<std::string> commandArguments;
};