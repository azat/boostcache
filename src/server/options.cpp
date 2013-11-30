
/**
 * This file is part of the boostcache package.
 *
 * (c) Azat Khuzhin <a3at.mail@gmail.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

#include "server/options.h"

namespace Server
{
    void Options::additionalOptions()
    {
        m_visibleOptions.add_options()
            ("port,p", boost::program_options::value<int>()->default_value(9876),
             "Port listen to")
            ("host,H", boost::program_options::value<std::string>(),
             "Host listen to")
            ("socket,s", boost::program_options::value<std::string>()->default_value("boostcached.sock"),
             "Socket listen to")
            ("fork,f", "Fork server process")
            ("workers,w", boost::program_options::value<int>()->default_value(2),
             "Number of workers-threads")
        ;
    }
}
