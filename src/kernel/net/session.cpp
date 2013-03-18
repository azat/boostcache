
/**
 * This file is part of the boostcache package.
 *
 * (c) Azat Khuzhin <a3at.mail@gmail.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

#include "session.h"

#include <boost/bind.hpp>


void Session::start()
{
    m_socket.async_read_some(boost::asio::buffer(m_data, MAX_LENGTH),
                             boost::bind(&Session::handleRead, this,
                                         boost::asio::placeholders::error,
                                         boost::asio::placeholders::bytes_transferred));
}

void Session::handleRead(const boost::system::error_code& error, size_t bytesTransferred)
{
    if (error) {
        delete this;
        return;
    }
    boost::asio::async_write(m_socket,
                             boost::asio::buffer(m_data, bytesTransferred),
                             boost::bind(&Session::handleWrite, this,
                                         boost::asio::placeholders::error));
}

void Session::handleWrite(const boost::system::error_code& error)
{
    if (error) {
        delete this;
        return;
    }

    m_socket.async_read_some(boost::asio::buffer(m_data, MAX_LENGTH),
                             boost::bind(&Session::handleRead, this,
                                         boost::asio::placeholders::error,
                                         boost::asio::placeholders::bytes_transferred));
}