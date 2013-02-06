
#include "server/options.h"

using namespace Server;

int main(int argc, char **argv)
{
    Options options;
    options.parse(argc, argv);

    return EXIT_SUCCESS;
}
