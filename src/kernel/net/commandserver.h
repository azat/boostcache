
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
#include "ioservicepool.h"

#include <string>

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/local/stream_protocol.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/noncopyable.hpp>

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
    Options m_options;

    IoServicePool m_ioServicePool;
    boost::asio::ip::tcp::acceptor m_tcpAcceptor;
    boost::asio::local::stream_protocol::acceptor m_unixDomainAcceptor;

    boost::asio::signal_set m_stopSignals;

    void setupStopSignals();

    void createTcpEndpoint();
    void createUnixDomainEndpoint();

    void startAcceptOnTcp();
    void startAcceptOnUnixDomain();

    void handleAcceptOnTcp(TcpSession *newSession,
                           const boost::system::error_code &error);
    void handleAcceptOnUnixDomain(UnixDomainSession *newSession,
                                  const boost::system::error_code &error);

    void stop();
};