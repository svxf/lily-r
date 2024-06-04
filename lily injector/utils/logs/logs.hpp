#pragma once

#include <vector>
#include <filesystem>
#include <cstdint>
#include <string>

namespace cat
{
    namespace utils
    {
        namespace log
        {
            void debug_log(const std::uint64_t address, const char* name);

            std::vector<std::filesystem::path> get_roblox_file_logs();
            std::filesystem::path get_latest_log();
        }
    }
}