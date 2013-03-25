
/**
 * This file is part of the boostcache package.
 *
 * (c) Azat Khuzhin <a3at.mail@gmail.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

#pragma once

#include <sstream>
#include <string>
#include <vector>
#include <functional>
#include <boost/noncopyable.hpp>

/**
 * TODO: speedup parsing
 * TODO: more error-friendly parsing
 *
 * Protocol format:
 *
 * *<number of arguments> CR LF
 * $<number of bytes of argument 1> CR LF
 * <argument data> CR LF
 * ...
 * $<number of bytes of argument N> CR LF
 * <argument data> CR LF
 *
 * Example:
 *
 * *3
 * $3
 * SET
 * $5
 * mykey
 * $7
 * myvalue
 *
 * Also inline commands supports, example:
 * GET mykey
 *
 * More info at http://redis.io/topics/protocol
 */
class Command : boost::noncopyable
{
public:
    typedef std::function<void(const std::string&)> FinishCallback;
    typedef std::vector<std::string> Arguments;

    Command()
    {
        reset();
    }

    void setFinishCallback(FinishCallback callback)
    {
        m_finishCallback = callback;
    }

    /**
     * Return true if need it is not the end of command,
     * and need to feed more data.
     */
    bool feedAndParseCommand(const char *buffer, size_t size);

private:
    enum Type {
        NOT_SET,
        INLINE,
        MULTI_BULK
    } m_type;
    std::string m_lineBuffer;
    std::string m_commandString;
    size_t m_commandOffset;
    int m_numberOfArguments;
    /**
     * Number of command arguments left for parsing
     */
    int m_numberOfArgumentsLeft;
    int m_lastArgumentLength;
    Arguments m_commandArguments;

    /**
     * This callback will be called with result of executed command
     */
    FinishCallback m_finishCallback;


    /**
     * Return true if command successfully parsed
     */
    bool parseInline(std::istringstream& stream);
    /**
     * Return true if we can go next, i.e. number of arguments
     * successfully parsed
     */
    bool parseNumberOfArguments(std::istringstream& stream);
    /**
     * Return true if we can go next, i.e. all arguments
     * successfully parsed
     */
    bool parseArguments(std::istringstream& stream);
    void executeCommand();
    std::string toString() const;
    /**
     * Reset internal structures
     * i.e. "Connection failover"
     */
    void reset();
    /**
     * Check is stream good, and do some stuff otherwise
     * @return true if stream is good
     */
    bool handleStreamIsValid(const std::istringstream& stream);
};