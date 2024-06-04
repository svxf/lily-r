#include "classes.hpp"

#include <Windows.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <cstdlib> 
#include <string>
#include <algorithm>

#include "../../utils/logs/logs.hpp"
#include "../globals/globals.hpp"

template<typename T>
T readQword(HANDLE hProcess, std::uintptr_t address)
{
    T value;
    ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(address), &value, sizeof(T), nullptr);
    return value;
}

std::uint64_t get_render_view()
{
    auto latest_log = cat::utils::log::get_latest_log();

    std::ifstream rbx_log(latest_log);
    std::string rbx_log_line;

    while (true)
    {
        std::getline(rbx_log, rbx_log_line);
        if (rbx_log_line.find("initialize view(") != std::string::npos)
        {
            rbx_log_line = rbx_log_line.substr(rbx_log_line.find("initialize view(") + 21);
            rbx_log_line = rbx_log_line.substr(0, rbx_log_line.find(')'));

            std::uint64_t renderview = std::strtoull(rbx_log_line.c_str(), nullptr, 16);
            return renderview;
        }
    }
}

void cat::roblox::init(std::uint64_t pid)
{
    std::cout << "Initializing with PID: " << pid << std::endl;

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, static_cast<DWORD>(pid));
    if (hProcess == NULL)
    {
        std::cerr << "Failed to open the process. Error code: " << GetLastError() << std::endl;
        return;
    }

    std::cout << "Successfully opened the process!" << std::endl;

    auto gamePtr = readQword<std::uint64_t>(hProcess, get_render_view() + 0x118);
    auto game = static_cast<cat::roblox::instance_t>(readQword<std::uint64_t>(hProcess, gamePtr + 0x198));
    std::cout << "Game instance: " << game.self << std::endl;


    if (!game.self)
    {
        std::cerr << "Failed to get game." << std::endl;
        CloseHandle(hProcess);
        return;
    }

    globals::datamodel = game;

    auto players = game.find_first_child(hProcess, "Players");
    std::cout << "Players instance: " << players.self << std::endl;

    if (!players.self)
    {
        std::cerr << "Failed to get players." << std::endl;
        CloseHandle(hProcess);
        return;
    }

    globals::players = players;

    globals::players = players;
    auto children = players.get_children(hProcess).front();

    if (!children.self)
    {
        std::cerr << "Failed to get locaplayer." << std::endl;
        CloseHandle(hProcess);
        return;
    }

    globals::localplayer = children;
    std::cout << "LocalPlayer Name: " << children.get_name(hProcess) << std::endl;

    auto workspace = game.get_children(hProcess).front();
    auto character = workspace.find_first_child(hProcess, children.get_name(hProcess));
    auto targetScript = character.find_first_child(hProcess, "Animate");
    std::cout << "TargetScript instance: " << targetScript.self << std::endl;

    
    CloseHandle(hProcess);
}

std::vector<cat::roblox::instance_t> cat::roblox::instance_t::get_children(HANDLE hProcess)
{
    std::vector<cat::roblox::instance_t> children;

    if (this->self != 0)
    {
        auto child_list = readQword<std::uint64_t>(hProcess, this->self + cat::offsets::children);

        if (child_list != 0)
        {
            auto child_begin = readQword<std::uint64_t>(hProcess, child_list);
            auto end_child = readQword<std::uint64_t>(hProcess, child_list + cat::offsets::size);

            while (child_begin != end_child)
            {
                auto current_instance_address = readQword<std::uint64_t>(hProcess, child_begin);
                if (current_instance_address != 0)
                {
                    children.emplace_back(current_instance_address);
                }
                child_begin = child_begin + 16;
            }
        }
    }

    return children;
}

cat::roblox::instance_t cat::roblox::instance_t::find_first_child(HANDLE hProcess, std::string child)
{
    cat::roblox::instance_t ret;

    auto children = this->get_children(hProcess);


    for (auto& object : children)
    {
        auto childName = object.get_name(hProcess);

        if (childName == child)
        {
            ret = static_cast<cat::roblox::instance_t>(object);
            break;
        }
    }

    return ret;
}

std::string readstring(HANDLE hProcess, std::uint64_t address)
{
    std::string string;
    char character = 0;
    int char_size = sizeof(character);
    int offset = 0;

    string.reserve(204);

    while (offset < 200)
    {
        character = readQword<char>(hProcess, address + offset);

        if (character == 0)
            break;

        offset += char_size;
        string.push_back(character);
    }

    return string;
}

std::string read_string2(HANDLE hProcess, std::uint64_t string)
{
    const auto length = readQword<int>(hProcess, string + 0x18);

    if (length >= 16u)
    {
        const auto New = readQword<std::uint64_t>(hProcess, string);
        return readstring(hProcess, New);
    }
    else
    {
        const auto Name = readstring(hProcess, string);
        return Name;
    }
}

std::string cat::roblox::instance_t::get_name(HANDLE hProcess)
{
    const auto ptr = readQword<std::uint64_t>(hProcess, this->self + cat::offsets::name);

    if (ptr)
        return read_string2(hProcess, ptr);

}

std::string cat::roblox::instance_t::get_class_name(HANDLE hProcess)
{
    const auto ptr = readQword<std::uint64_t>(hProcess, this->self + cat::offsets::classname);

    if (ptr)
        return read_string2(hProcess, ptr + 0x8);
}