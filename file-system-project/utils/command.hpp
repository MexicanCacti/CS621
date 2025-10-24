#pragma once
#include <string>
#include <iostream>

struct Command
{
    std::string _command;
    std::string _argList;
    std::string _notes;

    Command() = default;
    Command(std::string c, std::string a, std::string n) :
        _command(c), _argList(a), _notes(n) {};
};