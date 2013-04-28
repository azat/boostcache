
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

namespace PlaceHolders = std::placeholders;
namespace Ip = boost::asio::ip;

CommandServer::CommandServer(const Options &options)
    : m_options(options)
    , m_ioServicePool(m_options.numOfWorkers)
    , m_acceptor(m_ioServicePool.ioService(),
                 Ip::tcp::endpoint(Ip::tcp::v4(), options.port))
{
    startAccept();
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

void CommandServer::startAccept()
{
    Session* newSession = new Session(m_ioServicePool.ioService());
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