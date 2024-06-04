#include "roblox/classes/classes.hpp"

#include <iostream>
#include <cstdlib>

int main(int argc, char* argv[])
{
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <pid_number>" << std::endl;
        return 1;
    }

    std::uint64_t pid = std::strtoull(argv[1], nullptr, 10);
    cat::roblox::init(pid);

    return 0;
}