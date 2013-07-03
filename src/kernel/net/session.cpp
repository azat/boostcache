
/**
 * This file is part of the boostcache package.
 *
 * (c) Azat Khuzhin <a3at.mail@gmail.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

#include "session.h"

#include <boost/asio/write.hpp>
#include <functional>

namespace PlaceHolders = std::placeholders;
namespace Asio = boost::asio;

template <typename SocketType>
Session<SocketType>::Session(boost::asio::io_service &ioService)
    : m_socket(ioService)
{
    m_commandHandler.setFinishCallback(std::bind(&Session::asyncWrite, this, PlaceHolders::_1));
}

template <typename SocketType>
void Session<SocketType>::start()
{
    asyncRead();
}

template <typename SocketType>
void Session<SocketType>::asyncRead()
{
    m_socket.async_read_some(Asio::buffer(m_buffer, MAX_BUFFER_LENGTH),
                             std::bind(&Session::handleRead, this,
                                       PlaceHolders::_1,
                                       PlaceHolders::_2));
}

template <typename SocketType>
void Session<SocketType>::asyncWrite(const CommandReply &response)
{
    Asio::async_write(m_socket, response,
                      std::bind(&Session::handleWrite, this, PlaceHolders::_1));
}

template <typename SocketType>
void Session<SocketType>::handleRead(const boost::system::error_code &error, size_t bytesTransferred)
{
    if (error) {
        delete this;
        return;
    }

    if (m_commandHandler.feedAndParseCommand(m_buffer, bytesTransferred)) {
        asyncRead();
    }
}

template <typename SocketType>
void Session<SocketType>::handleWrite(const boost::system::error_code &error)
{
    if (error) {
        delete this;
        return;
    }

    asyncRead();
}


// Explicit instantiations
template class Session<boost::asio::local::stream_protocol::socket>;
template class Session<boost::asio::ip::tcp::socket>;