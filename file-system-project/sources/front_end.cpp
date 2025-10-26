#include "../headers/front_end.hpp"

std::deque<std::string> FrontEnd::tokenizeInput(std::string& input)
{
    std::deque<std::string> tokens;
    size_t startPos = 0;
    size_t splitPos = 0;
    splitPos = input.find(' ', startPos);
    tokens.push_back(input.substr(startPos, splitPos - startPos));
    startPos = splitPos + 1;
    if(hashCommand(tokens.front()) == CommandCode::WRITE)
    {
        // Get the Bytes
        splitPos = input.find(' ', startPos);
        tokens.push_back(input.substr(startPos, splitPos - startPos));
        startPos = splitPos + 1;

        // Get the Data
        tokens.push_back(input.substr(startPos));
    }
    else
    {
        while(splitPos != std::string::npos)
        {
            splitPos = input.find(' ', startPos);
            tokens.push_back(input.substr(startPos, splitPos - startPos));
            startPos = splitPos + 1;
        }
    }
    return tokens;
}

bool FrontEnd::getStringToken(std::deque<std::string>& tokens, std::string& str)
{
    if(tokens.empty()) return false;
    str = tokens.front();
    tokens.pop_front();
    return true;
}

bool FrontEnd::getCharToken(std::deque<std::string>& tokens, char& ch)
{
    if(tokens.empty() || tokens.front().size() != 1) return false;
    ch = tokens.front()[0];
    tokens.pop_front();
    return true;
}

bool FrontEnd::getIntToken(std::deque<std::string>& tokens, int& i)
{
    if(tokens.empty()) return false;
    i = std::stoi(tokens.front());
    tokens.pop_front();
    return true;
}

void FrontEnd::printCommandList()
{
    std::vector<Command> commandList = {
        {"CREATE", "TYPE NAME", {"TYPE\t: character","NAME\t: string"}},
        {"OPEN", "MODE NAME", {"MODE\t: character", "NAME\t: string"}},
        {"CLOSE", "", {}},
        {"DELETE", "NAME", {"NAME\t: string"}},
        {"READ", "BYTES", {"BYTES\t: integer"}},
        {"WRITE", "BYTES 'DATA'", {"BYTES\t: integer", "'DATA'\t: string", "DATA must be enclosed by ' '"}},
        {"SEEK", "BASE OFFSET", {"BASE\t: integer", "OFFSET\t: integer"}},
        {"DISPLAY", "", {"Displays File System Structure"}}
    };
    const int commandWidth = 10;
    const int argWidth = 15;
    const int noteWidth = 20;
    std::cout << std::left << "\nInput command as one string in the following formats" << std::endl;
    std::cout << std::setw(commandWidth) << "Command";
    std::cout << std::setw(argWidth) << "Arg Order";
    std::cout << std::setw(noteWidth) << "Special Notes";
    std::cout << std::endl << std::string(commandWidth + argWidth + noteWidth, '-') << std::endl;
    for(Command& command : commandList)
    {
        std::cout << std::setw(commandWidth) << command._command;
        std::cout << std::setw(argWidth) << command._argList;
        if(!command._notes.empty())
        {
            std::cout << std::setw(noteWidth) << command._notes[0] << std::endl;
        }
        for(unsigned int i = 1 ; i < command._notes.size(); ++i)
        {
            std::cout << std::string(commandWidth + argWidth, ' ') << std::setw(noteWidth) << command._notes[i] << std::endl;
        }
        std::cout << std::endl;
    }
    if(_systemManager.hasOpenFile())
    {
        std::cout << "Open File Info:\n";
        std::cout << "File Name: " << _systemManager.getFileName() << std::endl;
        std::cout << "File Pointer: " << _systemManager.getFilePointer();
        std::cout << "\tFile Mode: " << _systemManager.getFileMode() << std::endl;
    }
    
}

std::string FrontEnd::promptInput()
{
    std::string input;
    std::getline(std::cin, input);
    return input;
}

InputResult FrontEnd::processInput(std::string& input)
{
    std::deque<std::string> tokens = tokenizeInput(input);
    if(tokens.empty()) return InputResult(BAD_ARG, "EMPTY INPUT STRING");

    std::string command = tokens.front();
    tokens.pop_front();
    InputResult processedInput;
    processedInput._command = command;
    std::string DATA;
    switch (hashCommand(processedInput._command))
    {
        case CommandCode::CREATE:
            if (!getCharToken(tokens, processedInput._charArg) || !getStringToken(tokens, processedInput._stringArg) || tokensLeft(tokens))
            {
                return {BAD_ARG, "INVALID CREATE ARGS"};
            }
            break;
        case CommandCode::OPEN:
            if (!getCharToken(tokens, processedInput._charArg) || !getStringToken(tokens, processedInput._stringArg) || tokensLeft(tokens))
            {
                return {BAD_ARG, "INVALID OPEN ARGS"};
            }
            break;
        case CommandCode::CLOSE:
            if (tokensLeft(tokens))
            {
                return {BAD_ARG, "INVALID CLOSE ARGS"};
            }
            break;
        case CommandCode::DELETE:
            if (!getStringToken(tokens, processedInput._stringArg) || tokensLeft(tokens))
            {
                return {BAD_ARG, "INVALID DELETE ARGS"};
            }
            break;
        case CommandCode::READ:
            if (!getIntToken(tokens, processedInput._intArg1) || tokensLeft(tokens))
            {
                return {BAD_ARG, "INVALID READ ARGS"};
            }
            break;
        case CommandCode::WRITE:
            if(!getIntToken(tokens, processedInput._intArg1)) return {BAD_ARG, "INVALID WRITE ARGS"};
            while(tokensLeft(tokens))
            {
                std::string tokenString;
                if(!getStringToken(tokens, tokenString)) return {BAD_ARG, "ERROR PARSING DATA"};
                DATA.append(tokenString);
            }
            processedInput._stringArg = DATA;
            if(processedInput._stringArg[0] != '\'' || processedInput._stringArg[processedInput._stringArg.length() - 1] != '\'') return {BAD_ARG, "DATA MUST BE ENCLOSED WITH \'"};
            processedInput._stringArg = processedInput._stringArg.substr(1, processedInput._stringArg.size() - 2);
            break;
        case CommandCode::SEEK:
            if (!getIntToken(tokens, processedInput._intArg1) || !getIntToken(tokens, processedInput._intArg2) || tokensLeft(tokens))
            {
                return {BAD_ARG, "INVALID SEEK ARGS"};
            }
            break;
        case CommandCode::DISPLAY:
            if(tokensLeft(tokens))
            {
                return {BAD_ARG, "INVALID DISPLAY ARGS"};
            }
            break;
        default:
            return {BAD_ARG, "NO MATCHING COMMAND"};
    }
    processedInput._status = SUCCESS;
    return processedInput;
}

STATUS_CODE FrontEnd::runInput(InputResult& processedInput)
{
    std::pair<STATUS_CODE, std::string> readResult;
    switch (hashCommand(processedInput._command))
    {
        case CommandCode::CREATE:
            return _systemManager.CREATE(processedInput._charArg, processedInput._stringArg);
        case CommandCode::OPEN:
            return _systemManager.OPEN(processedInput._charArg, processedInput._stringArg);
        case CommandCode::CLOSE:
            return _systemManager.CLOSE();
        case CommandCode::DELETE:
            return _systemManager.DELETE(processedInput._stringArg);
        case CommandCode::READ:
            readResult = _systemManager.READ(processedInput._intArg1);
            if(readResult.first == SUCCESS)
            {
                std::cout << "[READ]: " << readResult.second << std::endl;
            }
            return readResult.first;
        case CommandCode::WRITE:
            return _systemManager.WRITE(processedInput._intArg1, processedInput._stringArg);
        case CommandCode::SEEK:
            return _systemManager.SEEK(processedInput._intArg1, processedInput._intArg2);
        case CommandCode::DISPLAY:
            return _systemManager.displayFileSystem();
        default:
            return BAD_ARG;
    }
}

void FrontEnd::startInput()
{
    /*
        Prompt user whether to load from file or start new session
    */
    
    InputResult processedInput;
    std::string input;
    STATUS_CODE status;
    while(true)
    {
        printCommandList();
        input = promptInput();
        processedInput = processInput(input);
        if(processedInput._status != SUCCESS)
        {
            std::cout << "Invalid Input!\n";
            std::cout << "[ERROR CODE]: " << statusToString(processedInput._status);
            std::cout << "\t[MESSAGE]: " << processedInput._errorMessage << std::endl;
            continue;
        }
        status = runInput(processedInput);
        if(status != SUCCESS)
        {
            std::cout << "Error running command!\n";
            std::cout << "[ERROR CODE]: " << statusToString(status) << std::endl;
            continue;
        }
    }
}

int main()
{
    std::string root = "root";
    DiskManager diskManager(NUM_BLOCKS, BLOCK_SIZE, USER_DATA_SIZE);
    SystemManager systemManager(diskManager);
    FrontEnd frontEnd(systemManager);
    frontEnd.startInput();
}