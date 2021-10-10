#include <iostream>

#include "QApp.h"

int main(int argc, char **argv)
{
    QApp& app = QApp::Instance();
    if (app.Init(800, 600)) { app.Start(); }
    else { std::cout << "App couldn't be initialized properly. Shutting down...\n"; }

    app.Shutdown();
    return 0;

}

