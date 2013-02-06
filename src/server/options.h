
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
