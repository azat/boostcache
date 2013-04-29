
/**
 * This file is part of the boostcache package.
 *
 * (c) Azat Khuzhin <a3at.mail@gmail.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */


#include "ioservicepool.h"

#include <thread>


IoServicePool::IoServicePool(size_t size)
    : m_next(0)
{
    for (size_t i = 0; i < size; ++i) {
        IoServicePtr ioService(new boost::asio::io_service);
        WorkPtr work(new boost::asio::io_service::work(*ioService));

        m_ioServices.push_back(ioService);
        m_work.push_back(work);
    }
}

void IoServicePool::start()
{
    std::vector< std::shared_ptr<std::thread> > threads;

    for (size_t i = 0; i < m_ioServices.size(); ++i) {
        /**
         * static_cast<> is the work around for std::bind() vs boost::bind()
         * for overloaded functions.
         */
        std::shared_ptr<std::thread> thread(new std::thread(
                                            static_cast<size_t (boost::asio::io_service::*)()>
                                            (&boost::asio::io_service::run),
                                            m_ioServices[i].get()));
        threads.push_back(thread);
    }

    // Wait for all threads in the pool to exit.
    for (size_t i = 0; i < threads.size(); ++i) {
        threads[i]->join();
    }
}

boost::asio::io_service& IoServicePool::ioService()
{
    boost::asio::io_service& io_service = *m_ioServices[m_next];

    ++m_next;
    if (m_next == m_ioServices.size()) {
        m_next = 0;
    }

    return io_service;
}