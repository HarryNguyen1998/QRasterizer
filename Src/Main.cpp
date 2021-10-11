#include <iostream>

#include "QApp.h"

int main(int argc, char **argv)
{
    QApp& app = QApp::Instance();
    if (app.Init("Rasterizer thingy", 800, 600)) { app.Start(); }
    else { std::cout << "App couldn't be initialized. Shutting down...\n"; }

    app.Shutdown();
    return 0;

}

