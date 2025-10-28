#include "../headers/front_end.hpp"

bool FrontEnd::containsInvalidChars(std::string& input)
{
    for(char& c : input)
    {
        for(unsigned int i = 0 ; invalidFileChars[i] != '\0'; ++i)
        {
            if(c == invalidFileChars[i]) return true;
        }
    }
    return false;
}

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
    try
    {
        i = std::stoi(tokens.front());
        tokens.pop_front();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << ": Error converting integer to string\n";
        return false;
    }
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
        {"DISPLAY", "", {"Displays File System Structure"}},
        {"QUIT", "", {"Exits Program"}}
    };
    const int commandWidth = 10;
    const int argWidth = 15;
    const int noteWidth = 20;
    std::cout << std::left << "\n\nInput command as one string in the following formats, Command must be capitalized!" << std::endl;
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
        case CommandCode::QUIT:
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
    STATUS_CODE writeStatus;
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
                std::cout << "[READ]: \"" << readResult.second << std::endl;
            }
            return readResult.first;
        case CommandCode::WRITE:
            writeStatus = _systemManager.WRITE(processedInput._intArg1, processedInput._stringArg);
            if(writeStatus == OUT_OF_MEMORY)
            {
                std::cout << "[WARNING]: NOT ALL BYTES COULD BE WRITTEN DUE TO INSUFFICIENT DISK SPACE!" << std::endl;
            }
            return writeStatus;
        case CommandCode::SEEK:
            return _systemManager.SEEK(processedInput._intArg1, processedInput._intArg2);
        case CommandCode::DISPLAY:
            return _systemManager.displayFileSystem();
        case CommandCode::QUIT:
            return QUIT_PROGRAM;
        default:
            return BAD_ARG;
    }
}

void FrontEnd::endProgram()
{
    STATUS_CODE status = _systemManager.displayFileSystem();
    if(!status == SUCCESS)
    {
        std::cout << "\n[Error]: Error displaying file system";
    }
    std::cout << "\nWould you like to save the state of the file system? [Y/N]\n";
    std::string input = promptInput();
    while(input.empty() || ( toupper(input[0]) != 'Y' && toupper(input[0]) != 'N') )
    {
        std::cout << "\n[Error]: First character of \'" << input << "\'" << "is not Y or N. Please input 'Y' or 'N'\n";
        input = promptInput();
    }
    if(input[0] == 'Y')
    {
        while(true)
        {
            std::cout << "\nEnter name of the file to save to:";
            std::cout << "\nFile name cannot include: [ ";
            for(char& c : invalidFileChars)
            {
                std::cout << c << " ";
            }
            std::cout << "]\n";
            input = promptInput();
            if(!containsInvalidChars(input)) break;
            std::cout << "[Error]: " << input << " contains an invalid char.\n";
        }
        input.append(".txt");
        std::cout << "Saving file system to " << input << "...\n";
        _systemManager.SAVE(input);
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
        if(status == QUIT_PROGRAM) break;
        if(status != SUCCESS)
        {
            std::cout << "[ERROR CODE]: " << statusToString(status) << std::endl;
            continue;
        }
    }
    endProgram();
}

int main()
{
    DiskManager diskManager(NUM_BLOCKS, BLOCK_SIZE, USER_DATA_SIZE);
    SystemManager systemManager(diskManager);
    FrontEnd frontEnd(systemManager);
    frontEnd.startInput();
}