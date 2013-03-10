
/**
 * This file is part of the boostcache package.
 *
 * (c) Azat Khuzhin <a3at.mail@gmail.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

#pragma once

#include <exception>
#include <iostream>
#include <string>


class Exception : public std::exception {
public:
    Exception(std::string message = "", long code = 0) : message(message), code(code) {
    }

    virtual const char* what() const throw() {
        std::cerr << "[" << code << "] " << message << std::endl;

        return "Exception";
    }

    virtual ~Exception() throw() {}

    std::string getMessage() const {
        return message;
    }

    int getCode() const {
        return code;
    }

private:
    // exception message
    std::string message;

    // exception code
    long code;
};
