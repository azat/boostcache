
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

#include <arpa/inet.h>
#include <sys/un.h>
#include <cstring>
#include <csignal>


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

    BUG(signal(SIGPIPE, SIG_IGN) != SIG_ERR);
}

CommandServer::~CommandServer()
{
}

void CommandServer::start()
{
    LOG(info) << "Starting server with " << m_options.numOfWorkers << " workers";

    setupStopSignals();

    evconnlistener_enable(m_tcpAcceptor);
    evconnlistener_enable(m_unixDomainAcceptor);

    event_base_loop(m_base, 0);
    stop();
}

void CommandServer::prepareToStop(int /* fd */, short /* events */, void *arg)
{
    CommandServer *self = (CommandServer *)arg;
    event_base_loopexit(self->m_base, 0);
    LOG(info) << "Stop scheduled";
}
void CommandServer::stop()
{
    evconnlistener_disable(m_tcpAcceptor);
    evconnlistener_disable(m_unixDomainAcceptor);

    evconnlistener_free(m_tcpAcceptor);
    evconnlistener_free(m_unixDomainAcceptor);

    event_del(m_eInt);
    event_del(m_eTerm);

    event_free(m_eInt);
    event_free(m_eTerm);

    event_base_free(m_base);

    LOG(info) << "Exited cleanly";
}
event* CommandServer::createSignalHandler(int signal)
{
    event *e = evsignal_new(m_base, signal, prepareToStop, this);
    BUG(e);
    evsignal_add(e, NULL);

    return e;
}
void CommandServer::setupStopSignals()
{
    m_eTerm = createSignalHandler(SIGTERM);
    m_eInt = createSignalHandler(SIGINT);
}

void CommandServer::createTcpEndpoint()
{
    struct sockaddr_in addr;
    /* TODO: support IPv6 */
    addr.sin_family = AF_INET;
    addr.sin_port = htons(m_options.port);
    ASSERT(inet_pton(AF_INET, m_options.host.c_str(), &addr.sin_addr) == 1);

    m_tcpAcceptor = evconnlistener_new_bind(m_base,
                                            startAccept, this,
                                            FLAGS, BACKLOG,
                                            (struct sockaddr *)&addr, sizeof(addr));
    BUG(m_tcpAcceptor);

    LOG(info) << "Listening on " << m_options.host << ":" << m_options.port;
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
                                                   (struct sockaddr *)&addr, sizeof(addr));
    BUG(m_unixDomainAcceptor);

    LOG(info) << "Listening on " << m_options.socket;
}

