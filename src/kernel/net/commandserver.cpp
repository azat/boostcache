
/**
 * This file is part of the boostcache package.
 *
 * (c) Azat Khuzhin <a3at.mail@gmail.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */


#include "commandserver.h"
#include "util/log.h"

#include <boost/bind.hpp>
#include <boost/asio/placeholders.hpp>

using namespace boost;

CommandServer::CommandServer(const Options &options)
    : m_options(options)
    , m_acceptor(m_socket, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), options.port))
{
    startAccept();
}

CommandServer::~CommandServer()
{
}

void CommandServer::start()
{
    LOG(info) << "Starting server";
    m_socket.run();
}

void CommandServer::startAccept()
{
    Session* newSession = new Session(m_socket);
    m_acceptor.async_accept(newSession->socket(),
                            boost::bind(&CommandServer::handleAccept,
                                        this,
                                        newSession,
                                        boost::asio::placeholders::error));
}

void CommandServer::handleAccept(Session* newSession, const boost::system::error_code& error)
{
    if (!error) {
        LOG(info) << "Client connected " << newSession;
        newSession->start();
    } else {
        LOG(info) << "Client session error";
        delete newSession;
    }

    startAccept();
}