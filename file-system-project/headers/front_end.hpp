#pragma once
#include "../headers/system_manager.hpp"
#include "../utils/input_result.hpp"
#include "../utils/command.hpp"
#include <iostream>
#include <string>
#include <iomanip>
#include <vector>


class FrontEnd {
    private:
        SystemManager& _systemManager;
        std::deque<std::string> tokenizeInput(std::string& input);
        static bool getStringToken(std::deque<std::string>& tokens, std::string& str);
        static bool getCharToken(std::deque<std::string>& tokens, char& ch);
        static bool getIntToken(std::deque<std::string>& tokens, int& i);
        static bool tokensLeft(std::deque<std::string>& tokens) {return !tokens.empty();}
        void printCommandList();
        std::string promptInput();
        InputResult processInput(std::string& input);
        STATUS_CODE runInput(InputResult& processedInput);

    public:
        FrontEnd(SystemManager& systemManager) : _systemManager(systemManager) {};
        void startInput();
};