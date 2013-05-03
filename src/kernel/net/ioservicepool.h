
/**
 * This file is part of the boostcache package.
 *
 * (c) Azat Khuzhin <a3at.mail@gmail.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

#pragma once


#include <boost/noncopyable.hpp>
#include <boost/asio/io_service.hpp>
#include <memory>
#include <vector>


/**
 * @brief Pool of io_service boost objects
 *
 * Based on boost_asio/example/http/server2/io_service_pool.hpp
 */
class IoServicePool : boost::noncopyable
{
public:
    IoServicePool(size_t size);

    void start();
    void stop();

    /**
     * Use a round-robin scheme to choose the next io_service to use.
     */
    boost::asio::io_service& ioService();

private:
    typedef std::shared_ptr<boost::asio::io_service> IoServicePtr;
    typedef std::shared_ptr<boost::asio::io_service::work> WorkPtr;

    std::vector<IoServicePtr> m_ioServices;
    std::vector<WorkPtr> m_work;
    size_t m_next;
};