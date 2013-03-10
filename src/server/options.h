
/**
 * This file is part of the boostcache package.
 *
 * (c) Azat Khuzhin <a3at.mail@gmail.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

#include "util/options.h"

namespace Server
{
    class Options : public Util::Options
    {
    public:
        Options() : Util::Options() {}

    protected:
        virtual void additionalOptions();
    };
}
