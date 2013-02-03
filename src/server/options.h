
#include "util/options.h"

namespace Server
{
	class Options : public Util::Options
	{
	public:
		Options(int argc, char **argv) : Util::Options(argc, argv) {}
	};
}
