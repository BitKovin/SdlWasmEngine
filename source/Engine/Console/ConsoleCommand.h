#pragma once

#include <string>
#include <vector>
#include <functional>

struct ConsoleCommand
{
    std::string name;
    std::string help;
    std::function<void(const std::vector<std::string>&)> func;
};