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
        void printCommandList();
        std::string promptInput();
        InputResult processInput(std::string& input);

    public:
        FrontEnd(SystemManager& systemManager) : _systemManager(systemManager) {};
        void startInput();
};