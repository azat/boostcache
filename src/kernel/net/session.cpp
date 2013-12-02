
/**
 * This file is part of the boostcache package.
 *
 * (c) Azat Khuzhin <a3at.mail@gmail.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

#include "session.h"

#include "util/log.h"

#include <boost/asio/write.hpp>
#include <functional>

namespace PlaceHolders = std::placeholders;
namespace Asio = boost::asio;

namespace {

void asyncRead(struct bufferevent * /*bev*/, void *arg)
{
    Session *session = (Session *)arg;
    session->handleRead();
}

void eventTriggered(struct bufferevent *bev, short events, void *arg)
{
    if (events & (BEV_EVENT_ERROR | BEV_EVENT_EOF)) {
        Session *session = (Session *)arg;
        LOG(debug) << "Client disconnected " << session;

        bufferevent_free(bev);
        delete session;
    }
}

};

Session::Session(evconnlistener *lev, int fd)
    : m_lev(lev)
    , m_bev(bufferevent_socket_new(evconnlistener_get_base(lev), fd, BEV_OPT_CLOSE_ON_FREE))
    , m_input(bufferevent_get_input(m_bev))
    , m_output(bufferevent_get_output(m_bev))
{
    m_commandHandler.setFinishCallback(std::bind(&Session::asyncWrite, this, PlaceHolders::_1));

    bufferevent_setcb(m_bev, asyncRead, NULL, eventTriggered, this);
    bufferevent_enable(m_bev, EV_READ | EV_WRITE);
}

void Session::asyncWrite(const std::string &message)
{
    evbuffer_add(m_output, message.c_str(), message.size());
}

void Session::handleRead()
{
    const char *body = (const char *)evbuffer_pullup(m_input, -1);
    size_t bytesTransferred = evbuffer_get_length(m_input);

    /* Will call asyncWrite() as finish callback,
     * when parsing will be finished. */
    m_commandHandler.feedAndParseCommand(body, bytesTransferred);

    evbuffer_drain(m_input, bytesTransferred);
}

