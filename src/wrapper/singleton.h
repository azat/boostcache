
/**
 * This file is part of the boostcache package.
 *
 * (c) Azat Khuzhin <a3at.mail@gmail.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

/**
 * TODO: we need more right/transparent way to do this.
 * TODO: thread-safity.
 */

namespace Wrapper
{
    template<class Type>
    class Singleton
    {
    public:
        static Type& instance()
        {
            static Type instance;
            return instance;
        }

    private:
        Singleton() {}
        Type& operator=(const Type &) = delete;
        Singleton(const Type &) = delete;
    };
}