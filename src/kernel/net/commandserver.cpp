
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
#include <boost/thread/thread.hpp>

namespace PlaceHolders = std::placeholders;
namespace Ip = boost::asio::ip;

CommandServer::CommandServer(const Options &options)
    : m_options(options)
    , m_acceptor(m_ioService, Ip::tcp::endpoint(Ip::tcp::v4(), options.port))
{
    startAccept();
}

CommandServer::~CommandServer()
{
}

void CommandServer::start()
{
    LOG(info) << "Starting server";

    /**
     * TODO: more test/benchmarks
     * Maybe this will be slower then single-threaded application in some cases
     *
     * TODO: Also this is just simple/raw implementation, it is not final.
     * A lot of work need to do.
     */
    std::vector< std::shared_ptr<boost::thread> > threads;
    for (int i = 0; i < m_options.numOfWorkers; ++i) {
        LOG(info) << "Creating " << i << " thread";

        /**
         * static_cast<> is the work around for std::bind() vs boost::bind()
         * for overloaded functions.
         */
        std::shared_ptr<boost::thread> thread(new boost::thread(std::bind(
                                              static_cast<size_t (boost::asio::io_service::*)()>
                                              (&boost::asio::io_service::run),
                                              &m_ioService)));
        threads.push_back(thread);
    }

    for (int i = 0; i < m_options.numOfWorkers; ++i) {
        threads[i]->join();
    }
}

void CommandServer::startAccept()
{
    Session* newSession = new Session(m_ioService);
    m_acceptor.async_accept(newSession->socket(),
                            std::bind(&CommandServer::handleAccept,
                                      this,
                                      newSession,
                                      PlaceHolders::_1));
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