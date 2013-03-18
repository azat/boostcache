
/**
 * This file is part of the boostcache package.
 *
 * (c) Azat Khuzhin <a3at.mail@gmail.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

#pragma once

#include <boost/asio.hpp>


/**
 * Session handler for CommandServer
 */
class Session
{
public:
    Session(boost::asio::io_service& socket)
        : m_socket(socket)
    {
    }

    void start();

    boost::asio::ip::tcp::socket& socket() {
        return m_socket;
    }

private:
    void handleRead(const boost::system::error_code& error, size_t bytesTransferred);
    void handleWrite(const boost::system::error_code& error);

    boost::asio::ip::tcp::socket m_socket;
    enum Constants {
        MAX_LENGTH = 1024
    };
    char m_data[MAX_LENGTH];
};