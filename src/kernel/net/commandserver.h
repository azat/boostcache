
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
#include <vector>
#include <thread>

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
     * - more verbose error messages
     * - [?] domains resolving
     * - more statistics
     * - split threads support into another module
     */
    struct Routine
    {
        std::thread *thread;

        event_base *base;
        struct event *event;

        int read;
        int write;
        bool preparedForStop;
    };
    /** See also routineReadCmd() and startRoutineAccept() */
    struct Request
    {
        evconnlistener *lev;
        evutil_socket_t fd;
    } __attribute__((packed));
    std::vector<Routine> m_routines;
    event_base *m_base;
    evconnlistener *m_tcpAcceptor;
    evconnlistener *m_unixDomainAcceptor;
    event *m_eTerm;
    event *m_eInt;
    evconnlistener_cb m_cb;
    size_t m_handledRequests;

    static void
    startAccept(evconnlistener *lev, evutil_socket_t fd,
                sockaddr * /*addr*/, int /*socklen*/,
                void * /*arg = CommandServer **/);
    static void
    startRoutineAccept(evconnlistener *lev, evutil_socket_t fd,
                       sockaddr * /*addr*/, int /*socklen*/,
                       void *arg);

    static void prepareToStop(int /* fd */, short /* events */, void *arg);
    void stop();
    event* createSignalHandler(int signal);
    void setupStopSignals();
    void createTcpEndpoint();
    void createUnixDomainEndpoint();

    void createThreads();
    void initRoutine(Routine &routine);
    static void routineReadCmd(int /* fd */, short /* events */, void *arg);
    void prepareToStopThreads();
    void prepareToStopRoutine(Routine &routine);
    void stopThreads();
    void stopRoutine(Routine &routine);
};

