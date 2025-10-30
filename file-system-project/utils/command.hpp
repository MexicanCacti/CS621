#pragma once
#include <string>
#include <iostream>
#include <vector>

enum class CommandCode
{
    CREATE,
    OPEN,
    CLOSE,
    DELETE,
    READ,
    WRITE,
    SEEK,
    DISPLAY,
    QUIT,
    UNKNOWN
};

CommandCode hashCommand(const std::string& command)
{
    if(command == "CREATE")     return CommandCode::CREATE;
    if(command == "OPEN")       return CommandCode::OPEN;
    if(command == "CLOSE")      return CommandCode::CLOSE;
    if(command == "DELETE")     return CommandCode::DELETE;
    if(command == "READ")       return CommandCode::READ;
    if(command == "WRITE")      return CommandCode::WRITE;
    if(command == "SEEK")       return CommandCode::SEEK;
    if(command == "DISPLAY")    return CommandCode::DISPLAY;
    if(command == "QUIT")       return CommandCode::QUIT;
    return CommandCode::UNKNOWN;
}

struct Command
{
    std::string _command;
    std::string _argList;
    std::vector<std::string> _notes;

    Command() = default;
    Command(std::string c, std::string a, std::vector<std::string> n) :
        _command(c), _argList(a), _notes(n) {};
};