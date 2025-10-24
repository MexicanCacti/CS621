#include "../headers/front_end.hpp"


void FrontEnd::printCommandList()
{
    std::vector<Command> commandList = {
        {"CREATE", "TYPE NAME", "TYPE : character\nNAME : string"},
        {"OPEN", "MODE NAME", "MODE : character\nNAME : string"},
        {"CLOSE", "", ""},
        {"DELETE", "NAME", "NAME : string"},
        {"READ", "BYTES", "BYTES : integer"},
        {"WRITE", "BYTES, 'DATA'", "BYTES : integer\n'DATA' : string\nData must be enclosed by ' '"},
        {"SEEK", "BASE, OFFSET", "BASE : integer\nOFFSET : integer"}
    };
    const int commandWidth = 10;
    const int argWidth = 15;
    const int noteWidth = 20;
    std::cout << std::left << "Input command as one string in the following formats" << std::endl;
    std::cout << std::setw(commandWidth) << "Command";
    std::cout << std::setw(argWidth) << "Arg Order";
    std::cout << std::setw(noteWidth) << "Special Notes";
    std::cout << std::string(commandWidth + argWidth + noteWidth, '-') << std::endl;
    for(Command& command : commandList)
    {
        std::cout << std::setw(commandWidth) << command._command;
        std::cout << std::setw(argWidth) << command._argList;
        std::cout << std::setw(noteWidth) << command._notes;
        std::cout << std::endl;
    }
    
}

std::string FrontEnd::promptInput()
{
    return "";
}

InputResult FrontEnd::processInput(std::string& input)
{
    InputResult i;
    return i;
}

void FrontEnd::startInput()
{
    /*
        Prompt user whether to load from file or start new session
    */
    
    InputResult inputResult;
    std::string input;
    while(true)
    {
        printCommandList();
        std::getline(std::cin, input);
        std::cout << input;
    }
}

int main()
{
    std::string root = "root";
    DiskManager diskManager(NUM_BLOCKS, BLOCK_SIZE, USER_DATA_SIZE);
    SystemManager systemManager(diskManager, root);
    FrontEnd frontEnd(systemManager);
    frontEnd.startInput();
}