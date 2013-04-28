
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
#include <boost/noncopyable.hpp>

/**
 * Async command server
 *
 * TODO: singleton
 */
class CommandServer : boost::noncopyable
{
public:
    struct Options
    {
        short port;
        std::string ip;
        int numOfWorkers;

        Options(short port = 0, std::string ip = "", int numOfWorkers = 0)
            : port(port)
            , ip(ip)
            , numOfWorkers(numOfWorkers)
        {}
    };

    CommandServer(const Options &options);
    virtual ~CommandServer();

    void start();

private:
    Options m_options;

    IoServicePool m_ioServicePool;
    boost::asio::ip::tcp::acceptor m_acceptor;

    void startAccept();
    void handleAccept(Session* newSession, const boost::system::error_code& error);
};