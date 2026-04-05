#include "downward/utils/not_implemented.h"

#include "downward/utils/exceptions.h"

#include <filesystem>
#include <format>
#include <sstream>

namespace utils {

void not_implemented(const std::source_location& source_location)
{
    class NotImplementedError : public std::runtime_error {
    public:
        explicit NotImplementedError(const std::string& message)
            : std::runtime_error(message)
        {
        }
    };

    std::filesystem::path full_path(source_location.file_name());

    std::stringstream ss;

    ss << "Called unimplemented function " <<
        source_location.function_name() << "(at " <<
        full_path.filename().generic_string() << ":" <<
        source_location.line() << ")";

    throw NotImplementedError(ss.str());
}

}