
/**
 * This file is part of the boostcache package.
 *
 * (c) Azat Khuzhin <a3at.mail@gmail.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */


#include "interface.h"
#include "util/compiler.h"


namespace Db
{
    Interface::Interface()
    {
    }

    /**
     * TODO: add @arguments into error?
     */
    std::string Interface::get(const CommandHandler::Arguments &UNUSED(arguments))
    {
        return CommandHandler::REPLY_ERROR_NOTSUPPORTED;
    }

    std::string Interface::set(const CommandHandler::Arguments &UNUSED(arguments))
    {
        return CommandHandler::REPLY_ERROR_NOTSUPPORTED;
    }

    std::string Interface::del(const CommandHandler::Arguments &UNUSED(arguments))
    {
        return CommandHandler::REPLY_ERROR_NOTSUPPORTED;
    }

    std::string Interface::foreach(const CommandHandler::Arguments &UNUSED(arguments))
    {
        return CommandHandler::REPLY_ERROR_NOTSUPPORTED;
    }
}
