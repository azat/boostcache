
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
#include "util/assert.h"

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <functional>
#include <vector>
#include <sstream>


namespace PlaceHolders = std::placeholders;
namespace Ip = boost::asio::ip;

namespace {

void startAccept(evconnlistener *lev, evutil_socket_t fd,
                 sockaddr * /*addr*/, int /*socklen*/,
                 void * /*arg = CommandServer **/)
{
    Session *newSession = new Session(lev, fd);
    LOG(debug) << "Client connected " << newSession;
}

};

CommandServer::CommandServer(const Options &options)
    : m_options(options)
    , m_base(event_base_new())
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

    evconnlistener_enable(m_tcpAcceptor);
    evconnlistener_enable(m_unixDomainAcceptor);

    event_base_loop(m_base, 0);
}

void CommandServer::createTcpEndpoint()
{
    std::stringstream streamForPort;
    streamForPort << m_options.port;

    boost::asio::io_service io;
    Ip::tcp::resolver resolver(io);
    // TODO: support ipv6
    Ip::tcp::resolver::query query(Ip::tcp::v4(), m_options.host, streamForPort.str());
    Ip::tcp::endpoint endpoint = *resolver.resolve(query);

    struct sockaddr *addr = endpoint.data();
    m_tcpAcceptor = evconnlistener_new_bind(m_base,
                                            startAccept, this,
                                            FLAGS, BACKLOG,
                                            addr, sizeof(*addr));
    ASSERT(m_tcpAcceptor);

    LOG(info) << "Listening on " << endpoint;
}

void CommandServer::createUnixDomainEndpoint()
{
    // TODO: unlink only "*.sock"
    ::unlink(m_options.socket.c_str());

    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, m_options.socket.c_str(), sizeof(addr.sun_path));
    m_unixDomainAcceptor = evconnlistener_new_bind(m_base,
                                                   startAccept, this,
                                                   FLAGS, BACKLOG,
                                                   (sockaddr *)&addr, sizeof(addr));
    ASSERT(m_unixDomainAcceptor);

    LOG(info) << "Listening on " << m_options.socket;
}

