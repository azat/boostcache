
#include "server/options.h"

namespace Server
{
    void Options::additionalOptions()
    {
        visibleOptions.add_options()
            ("port,p", boost::program_options::value<int>()->default_value(9876), "Port listen to")
            ("host,H", boost::program_options::value<std::string>(), "Host listen to")
            ("socket,s", boost::program_options::value<std::string>()->default_value("boostcached.sock"), "Socket listen to")
        ;
    }
}