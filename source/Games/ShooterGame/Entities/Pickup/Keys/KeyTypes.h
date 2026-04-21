#pragma once

#include <cstdint>

enum class DoorKey : uint8_t
{
	None = 0,
	Red = 1,
	Green = 2,
	Blue = 3,
	Secret = 4,
	Special = 5
};