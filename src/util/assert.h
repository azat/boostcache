
/**
 * This file is part of the boostcache package.
 *
 * (c) Azat Khuzhin <a3at.mail@gmail.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

/**
 * @brief Assert macros (just aliases for boost assert utils)
 */

#include <boost/assert.hpp>

#define ASSERT(...) BOOST_ASSERT(__VA_ARGS__)
#define ASSERT_MSG(...) BOOST_ASSERT_MSG(__VA_ARGS__)
#define VERIFY(...) BOOST_VERIFY(__VA_ARGS__)

#ifdef NDEBUG
#undef NDEBUG
#include <boost/assert.hpp>

#define BUG(...) BOOST_ASSERT(__VA_ARGS__)

#define NDEBUG
#else

#define BUG(...) BOOST_ASSERT(__VA_ARGS__)

#endif

