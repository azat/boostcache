
/**
 * This file is part of the boostcache package.
 *
 * (c) Azat Khuzhin <a3at.mail@gmail.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

#pragma once

#include "session.h"

#include <string>

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/local/stream_protocol.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/noncopyable.hpp>

#include <event2/event.h>
#include <event2/listener.h>

/**
 * @brief Async command server
 *
 * TODO: singleton
 *
 * TODO: common implementation for TCP/IP and unix domain sockets
 */
class CommandServer : boost::noncopyable
{
public:
    struct Options
    {
        short port;
        std::string host;
        std::string socket;
        int numOfWorkers;

        Options(short port = 0, std::string host = "",
                std::string socket = "", int numOfWorkers = 0)
            : port(port)
            , host(host)
            , socket(socket)
            , numOfWorkers(numOfWorkers)
        {}
    };

    CommandServer(const Options &options);
    virtual ~CommandServer();

    void start();

private:
    /* TODO: ~LEV_OPT_THREADSAFE */
    static const unsigned FLAGS = LEV_OPT_LEAVE_SOCKETS_BLOCKING |
                                  LEV_OPT_CLOSE_ON_FREE |
                                  LEV_OPT_CLOSE_ON_EXEC |
                                  LEV_OPT_REUSEABLE |
                                  LEV_OPT_THREADSAFE |
                                  LEV_OPT_DISABLED;
    static const int BACKLOG = -1;

    Options m_options;

    /**
     * TODO: Because of migration to libevent:
     *
     * - we didn't have normal stop
     * - memory leaks
     * - more verbose error messages
     * - [?] multi-threadding support (workers)
     */
    event_base *m_base;
    evconnlistener *m_tcpAcceptor;
    evconnlistener *m_unixDomainAcceptor;

    void createTcpEndpoint();
    void createUnixDomainEndpoint();
};

