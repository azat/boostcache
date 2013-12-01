
/**
 * This file is part of the boostcache package.
 *
 * (c) Azat Khuzhin <a3at.mail@gmail.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

#pragma once

#include "kernel/commandhandler.h"

#include <boost/noncopyable.hpp>

#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

/**
 * Session handler for CommandServer
 */
class Session : boost::noncopyable
{
public:
    Session(evconnlistener *lev, int fd);

    void handleRead();

private:
    evconnlistener *m_lev;
    bufferevent *m_bev;

    evbuffer *m_input;
    evbuffer *m_output;

    CommandHandler m_commandHandler;

    void asyncWrite(const std::string &message);
};

