
/**
 * This file is part of the boostcache package.
 *
 * (c) Azat Khuzhin <a3at.mail@gmail.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

#pragma once

#include <string>
#include <vector>
#include <functional>
#include <boost/noncopyable.hpp>
#include <boost/asio/buffer.hpp>

/**
 * @brief Protocol format (based on redis protocol):
 * (@see http://redis.io/topics/protocol)
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
 * GET mykey LF
 * GET mykey CR LF
 *
 * TODO: speedup parsing
 * TODO: more error-friendly parsing
 * TODO: work on compatitiblity with redis protocol
 */
class CommandHandler : boost::noncopyable
{
public:
    /**
     * TODO: use boost::array instead
     */
    typedef std::vector<boost::asio::const_buffer> Reply;
    typedef std::function<void(const Reply&)> FinishCallback;
    typedef std::vector<std::string> Arguments;

    /**
     * Some of default replices for commands
     * TODO: wrap this to enum
     */
    static const Reply REPLY_FALSE;
    static const Reply REPLY_TRUE;
    /**
     * Not found
     */
    static const Reply REPLY_NIL;
    static const Reply REPLY_OK;
    /**
     * Use this when debug, and don't want to write full error message
     */
    static const Reply REPLY_ERROR;
    /**
     * Use this for non implemented _yet_ stuff
     */
    static const Reply REPLY_ERROR_NOTSUPPORTED;


    /**
     * TODO: optimize
     */
    static Reply toReplyString(const std::string &string);
    static Reply toInlineReplyString(const std::string &string);
    static Reply toErrorReplyString(const std::string &string);

    CommandHandler()
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
     *
     * Possible line separator: LF OR CRLF
     */
    bool parseInline(const char *begin, const char *end);
    /**
     * Return true if we can go next, i.e. number of arguments
     * successfully parsed
     *
     * Possible line separator: CRLF
     */
    const char* parseNumberOfArguments(const char *begin, const char *end);
    /**
     * Return true if we can go next, i.e. all arguments
     * successfully parsed
     *
     * Possible line separator: CRLF
     */
    bool parseArguments(const char *begin, const char *end);
    void executeCommand();
    std::string toString() const;
    /**
     * Reset internal structures
     * i.e. "Connection failover"
     */
    void reset();

    static void split(const char *begin, const char *end,
                      std::vector<std::string>& destination, char delimiter = ' ');
    static int toInt(const char *begin, const char *end);
};

typedef CommandHandler::Reply CommandReply;
