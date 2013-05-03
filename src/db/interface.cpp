
/**
 * This file is part of the boostcache package.
 *
 * (c) Azat Khuzhin <a3at.mail@gmail.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */


#include "interface.h"

#include <boost/format.hpp>


namespace Db
{
    Interface::Interface()
    {
    }

    /**
     * TODO: add @arguments into error?
     */
    std::string Interface::get(const Command::Arguments& UNUSED(arguments))
    {
        return Command::REPLY_ERROR_NOTSUPPORTED;
    }

    std::string Interface::set(const Command::Arguments& UNUSED(arguments))
    {
        return Command::REPLY_ERROR_NOTSUPPORTED;
    }

    std::string Interface::del(const Command::Arguments& UNUSED(arguments))
    {
        return Command::REPLY_ERROR_NOTSUPPORTED;
    }

    std::string Interface::valueToReplyString(const Value& value)
    {
        return str(boost::format("$%i\r\n%s\r\n") % value.size() % value);
    }
}