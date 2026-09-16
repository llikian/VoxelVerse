/***************************************************************************************************
 * @file  main.cpp
 * @brief Contains the main program of the project
 **************************************************************************************************/

#include <iostream>
#include "applications/Application.hpp"
#include "engine/Context.hpp"

int main() {
    try {
        Context::init();
        Application app;
        app.run();
    } catch(const std::exception& exception) {
        std::cerr << "ERROR : " << exception.what() << '\n';
        return -1;
    }

    return 0;
}
