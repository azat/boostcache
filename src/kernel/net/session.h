
/**
 * This file is part of the boostcache package.
 *
 * (c) Azat Khuzhin <a3at.mail@gmail.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

#pragma once

#include "kernel/command.h"

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/noncopyable.hpp>
#include <string>

/**
 * Session handler for CommandServer
 */
class Session : boost::noncopyable
{
public:
    Session(boost::asio::io_service& socket);

    void start();

    boost::asio::ip::tcp::socket& socket()
    {
        return m_socket;
    }

private:
    boost::asio::ip::tcp::socket m_socket;
    /**
     * TODO: We can avoid this, by using buffers with std::string
     */
    enum Constants
    {
        MAX_BUFFER_LENGTH = 1 << 10 /* 1024 */
    };
    char m_buffer[MAX_BUFFER_LENGTH];
    Command m_command;

    void asyncRead();
    void asyncWrite(const std::string& message);
    void handleRead(const boost::system::error_code& error, size_t bytesTransferred);
    void handleWrite(const boost::system::error_code& error);
};