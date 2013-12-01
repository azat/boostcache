
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

#include <event2/event.h>
#include <event2/listener.h>

/**
 * Session handler for CommandServer
 */
class Session : boost::noncopyable
{
public:
    Session(evconnlistener *lev);

    void start();

private:
    evconnlistener *m_lev;
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
    void asyncWrite(const std::string &message);
    void handleRead(const boost::system::error_code &error, size_t bytesTransferred);
    void handleWrite(const boost::system::error_code &error);
};

