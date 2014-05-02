
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


namespace
{

/** XXX: must be less then sizeof(Request) */
static constexpr char STOP_ROUTINE_CMD[4] = { 0 };

}; // namespace


CommandServer::CommandServer(const Options &options)
    : m_options(options)
    , m_routines(options.numOfWorkers)
    , m_base(event_base_new())
    , m_cb(m_routines.size() ? startRoutineAccept : startAccept)
    , m_handledRequests(0)
{
    BUG(m_options.numOfWorkers >= 0);

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

    BUG(!evconnlistener_enable(m_tcpAcceptor));
    BUG(!evconnlistener_enable(m_unixDomainAcceptor));

    if (m_routines.size()) {
        createThreads();
    }

    BUG(event_base_loop(m_base, 0) >= 0);
    stop();
    stopThreads();
}

void CommandServer::startAccept(evconnlistener *lev, evutil_socket_t fd,
                                sockaddr * /*addr*/, int /*socklen*/,
                                void * /*arg = CommandServer **/)
{
    Session *newSession = new Session(lev, fd);
    LOG(debug) << "Client connected " << newSession;
}
void CommandServer::startRoutineAccept(evconnlistener *lev, evutil_socket_t fd,
                                       sockaddr * /*addr*/, int /*socklen*/,
                                       void *arg)
{
    CommandServer *server = (CommandServer *)arg;

    /** Simplest round-robin */
    size_t routineNum = (server->m_handledRequests++ % server->m_routines.size());
    Routine &routine = server->m_routines[routineNum];
    Request request = { lev, fd };

    if (write(routine.write, &request, sizeof(request)) != sizeof(request)) {
        LOG(error) << "Error proxying request to " << &routine;
    } else {
        LOG(debug) << "Request proxied to routine " << &routine;
    }
}


void CommandServer::prepareToStop(int /* fd */, short /* events */, void *arg)
{
    CommandServer *self = (CommandServer *)arg;

    /** XXX: be more clever here */
    self->prepareToStopThreads();
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
                                            m_cb, this,
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
                                                   m_cb, this,
                                                   FLAGS, BACKLOG,
                                                   (struct sockaddr *)&addr, sizeof(addr));
    BUG(m_unixDomainAcceptor);

    LOG(info) << "Listening on " << m_options.socket;
}

void CommandServer::createThreads()
{
    LOG(info) << "Create " << m_routines.size() << " threads";
    for (Routine &r : m_routines) {
        initRoutine(r);
    }
}
void CommandServer::initRoutine(Routine &routine)
{
    int fds[2];
    /** XXX: O_CLOEXEC */
    BUG(!pipe(fds));

    routine.read = fds[0];
    routine.write = fds[1];

    routine.base = event_base_new();
    /** XXX: defer */
    routine.event = event_new(routine.base, routine.read,
                              EV_READ | EV_PERSIST,
                              routineReadCmd, &routine);
    event_add(routine.event, NULL);

    routine.thread = new std::thread([&routine] {
        event_base_loop(routine.base, 0);
        LOG(info) << "Routine " << &routine << " finished";
    });
}
void CommandServer::routineReadCmd(int /* fd */, short /* events */, void *arg)
{
    Routine *routine = (Routine *)arg;

    Request request;
    ssize_t readed = read(routine->read, &request, sizeof(request));

    if (readed == sizeof(request)) {
        Session *newSession = new Session(request.lev, request.fd);
        LOG(debug) << "Client connected " << newSession << " on routine " << routine;
    } else if (readed == sizeof(STOP_ROUTINE_CMD) &&
               !memcmp(&request, STOP_ROUTINE_CMD, sizeof(STOP_ROUTINE_CMD))) {
        event_base_loopexit(routine->base, 0);
        LOG(info) << "Stop scheduled on routine " << routine;
    } else {
        LOG(error) << "Malformed command on routine " << routine;
    }
}
void CommandServer::prepareToStopThreads()
{
    for (Routine &r : m_routines) {
        prepareToStopRoutine(r);
    }
}
void CommandServer::prepareToStopRoutine(Routine &routine)
{
    /** Stop the loop on the next event: or this one */
    write(routine.write, STOP_ROUTINE_CMD, sizeof(STOP_ROUTINE_CMD));
}
void CommandServer::stopThreads()
{
    for (Routine &r : m_routines) {
        stopRoutine(r);
    }
}
void CommandServer::stopRoutine(Routine &routine)
{
    routine.thread->join();
    delete routine.thread;

    close(routine.read);
    close(routine.write);

    event_del(routine.event);
    event_free(routine.event);

    event_base_free(routine.base);
}

