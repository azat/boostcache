
/**
 * This file is part of the boostcache package.
 *
 * (c) Azat Khuzhin <a3at.mail@gmail.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

#pragma once

#include "kernel/commandhandler.h"

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/local/stream_protocol.hpp>
#include <boost/noncopyable.hpp>
#include <string>

/**
 * Session handler for CommandServer
 */
template <typename SocketType>
class Session : boost::noncopyable
{
public:
    Session(boost::asio::io_service& ioService);

    void start();

    SocketType& socket()
    {
        return m_socket;
    }

private:
    SocketType m_socket;
    /**
     * TODO: We can avoid this, by using buffers with std::string
     */
    enum Constants
    {
        MAX_BUFFER_LENGTH = 1 << 10 /* 1024 */
    };
    char m_buffer[MAX_BUFFER_LENGTH];
    CommandHandler m_commandHandler;

    void asyncRead();
    void asyncWrite(const std::string& message);
    void handleRead(const boost::system::error_code& error, size_t bytesTransferred);
    void handleWrite(const boost::system::error_code& error);
};

typedef Session<boost::asio::local::stream_protocol::socket> UnixDomainSession;
typedef Session<boost::asio::ip::tcp::socket> TcpSession;