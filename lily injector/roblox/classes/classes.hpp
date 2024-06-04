#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <Windows.h>

namespace cat
{
	namespace offsets
	{
		constexpr std::uint32_t size = 0x8;
		constexpr std::uint32_t name = 0x48;
		constexpr std::uint32_t children = 0x50;
		constexpr std::uint32_t classname = 0x18;
		constexpr std::uint32_t gameid = 0x160;
	}

	namespace roblox
	{
		void init(std::uint64_t pid);

		struct vector2_t final { float x, y; };
		struct vector3_t final { float x, y, z; };
		struct quaternion final { float x, y, z, w; };
		struct matrix4_t final { float data[16]; };

		class instance_t final
		{
		public:

			std::uint64_t self;

			std::string get_name(HANDLE hProcess);
			std::string get_class_name(HANDLE hProcess);
			std::vector<instance_t> get_children(HANDLE hProcess);
			cat::roblox::instance_t find_first_child(HANDLE hProcess, std::string child);

			std::vector<instance_t>::iterator begin() {
				return children.begin();
			}

			std::vector<instance_t>::iterator end() {
				return children.end();
			}

			instance_t() : self(0) {}
			explicit instance_t(std::uintptr_t address) : self(address) {}
		private:
			std::vector<instance_t> children;
		};
	}
}