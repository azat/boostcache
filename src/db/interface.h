
/**
 * This file is part of the boostcache package.
 *
 * (c) Azat Khuzhin <a3at.mail@gmail.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */


#pragma once

#include "kernel/commandhandler.h" // CommandHandler::Arguments

#include <boost/noncopyable.hpp>
#include <string>


namespace Db
{
    /**
     * @brief Db interface
     *
     * TODO: make it abstract (I'm not sure about vtable for now)
     *
     * TODO: make a abstract factory, and use virtual methods? (vtable lookup)
     *
     * TODO: make private constructor and a static method,
     * that will do all stuff, including different db names
     *
     * TODO: make multi-value (value something like std::vector)
     *
     * TODO: see CommandHandler::executeCommand() notes
     */
    class Interface : boost::noncopyable
    {
    public:
        Interface();

        /**
         * TODO: make this virtual
         * (measure performance regression)
         *
         * TODO: add bulk get/set/del commands
         */
        std::string get(const CommandHandler::Arguments &arguments);
        std::string set(const CommandHandler::Arguments &arguments);
        std::string del(const CommandHandler::Arguments &arguments);

    protected:
        typedef std::string Key;
        typedef std::string Value;
    };
}