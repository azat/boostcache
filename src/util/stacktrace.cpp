
#include "stacktrace.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <map>
#include <vector>

#ifndef _WIN32

#include <execinfo.h>

namespace Util {
    static const int MAX_BACK_TRACE_FRAMES = 20;

    void printStackTrace(std::ostream& os) {
        void *b[MAX_BACK_TRACE_FRAMES];

        int size = ::backtrace(b, MAX_BACK_TRACE_FRAMES);
        for (int i = 0; i < size; i++) {
            os << std::hex << b[i] << std::dec << ' ';
        }
        os << std::endl;

        char **strings = ::backtrace_symbols(b, size);
        for (int i = 0; i < size; i++) {
            os << ' ' << strings[i] << std::endl;
        }
        os.flush();
        ::free(strings);
    }
}
#else

namespace Util {
    void printStackTrace(std::ostream& os) {
    }
}

#endif
