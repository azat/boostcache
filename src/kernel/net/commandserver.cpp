
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

#include <functional>
#include <vector>
#include <sstream>

namespace PlaceHolders = std::placeholders;
namespace Ip = boost::asio::ip;
namespace Local = boost::asio::local;

CommandServer::CommandServer(const Options &options)
    : m_options(options)
    , m_ioServicePool(m_options.numOfWorkers)
    , m_tcpAcceptor(m_ioServicePool.ioService())
    , m_unixDomainAcceptor(m_tcpAcceptor.get_io_service())
{
    createTcpEndpoint();
    createUnixDomainEndpoint();
}

CommandServer::~CommandServer()
{
}

void CommandServer::start()
{
    LOG(info) << "Starting server with " << m_options.numOfWorkers << " workers";

    /**
     * TODO: more test/benchmarks
     * Maybe this will be slower then single-threaded application in some cases
     *
     * TODO: Also this is just simple/raw implementation, it is not final.
     * A lot of work need to do.
     */
    m_ioServicePool.start();
}

void CommandServer::createTcpEndpoint()
{
    std::stringstream streamForPort;
    streamForPort << m_options.port;

    Ip::tcp::resolver resolver(m_tcpAcceptor.get_io_service());
    // TODO: support ipv6
    Ip::tcp::resolver::query query(Ip::tcp::v4(), m_options.host, streamForPort.str());
    Ip::tcp::endpoint endpoint = *resolver.resolve(query);

    m_tcpAcceptor.open(endpoint.protocol());
    m_tcpAcceptor.set_option(Ip::tcp::acceptor::reuse_address(true));
    m_tcpAcceptor.bind(endpoint);
    m_tcpAcceptor.listen();

    LOG(info) << "Listening on " << endpoint;

    startAcceptOnTcp();
}

void CommandServer::createUnixDomainEndpoint()
{
    // TODO: unlink only "*.sock"
    ::unlink(m_options.socket.c_str());

    Local::stream_protocol::endpoint unixDomainEndpoint(m_options.socket);

    m_unixDomainAcceptor.open(unixDomainEndpoint.protocol());
    m_unixDomainAcceptor.bind(unixDomainEndpoint);
    m_unixDomainAcceptor.listen();

    LOG(info) << "Listening on " << unixDomainEndpoint;

    startAcceptOnUnixDomain();
}

void CommandServer::startAcceptOnTcp()
{
    TcpSession* newSession = new TcpSession(m_ioServicePool.ioService());
    m_tcpAcceptor.async_accept(newSession->socket(),
                               std::bind(&CommandServer::handleAcceptOnTcp,
                                         this,
                                         newSession,
                                         PlaceHolders::_1));
}

void CommandServer::startAcceptOnUnixDomain()
{
    UnixDomainSession* newSession = new UnixDomainSession(m_ioServicePool.ioService());
    m_unixDomainAcceptor.async_accept(newSession->socket(),
                                      std::bind(&CommandServer::handleAcceptOnUnixDomain,
                                                this,
                                                newSession,
                                                PlaceHolders::_1));
}

void CommandServer::handleAcceptOnTcp(TcpSession* newSession,
                                      const boost::system::error_code& error)
{
    if (!error) {
        LOG(debug) << "Client connected " << newSession
                   << " on " << m_tcpAcceptor.local_endpoint();
        newSession->start();
    } else {
        LOG(error) << "Client session error on "
                   << m_tcpAcceptor.local_endpoint();
        delete newSession;
    }

    startAcceptOnTcp();
}

void CommandServer::handleAcceptOnUnixDomain(UnixDomainSession* newSession,
                                             const boost::system::error_code& error)
{
    if (!error) {
        LOG(debug) << "Client connected " << newSession
                   << " on " << m_unixDomainAcceptor.local_endpoint();
        newSession->start();
    } else {
        LOG(error) << "Client session error on "
                   << m_unixDomainAcceptor.local_endpoint();
        delete newSession;
    }

    startAcceptOnUnixDomain();
}