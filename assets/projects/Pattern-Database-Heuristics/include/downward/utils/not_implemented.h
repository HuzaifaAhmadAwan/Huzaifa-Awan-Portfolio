
#ifndef DOWNWARD_UTILS_NOT_IMPLEMENTED_H
#define DOWNWARD_UTILS_NOT_IMPLEMENTED_H

#include <source_location>
#include <string>

namespace utils {

[[noreturn]]
extern void not_implemented(
    const std::source_location& source_location =
        std::source_location::current());

}

#endif
