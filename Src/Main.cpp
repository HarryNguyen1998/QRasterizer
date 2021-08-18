#include <iostream>

#include "QApp.h"

// @brief Main entry point of the program
int main(int argc, char **argv)
{
    QApp& app = QApp::Instance();
    if (app.Init()) { app.Start(); }
    else { std::cout << "App couldn't be initialized properly. Shutting down...\n"; }

    app.Shutdown();
    return 0;

}

