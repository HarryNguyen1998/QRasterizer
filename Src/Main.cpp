#include <iostream>
#include <memory>
#include <vector>

#include "Math/Matrix.h"
#include "QApp.h"
#include "Renderer/Model.h"
#include "Renderer/OBJLoader.h"

void RunTest(QApp& app);
void RunExample(QApp& app);

int main(int argc, char **argv)
{
    QApp& app = QApp::Instance();
    if (app.Init("Rasterizer thingy", 800, 600)) {
        RunExample(app);
        //RunTest(app);
    }
    else { std::cout << "App couldn't be initialized. Shutting down...\n"; }

    app.Shutdown();
    return 0;

}

void RunExample(QApp& app)
{
    {
        Model suzanne{OBJ::LoadFileData("Assets/suzanne.obj")};
        app.LoadModel(suzanne);
        app.LoadTexture("Assets/bricks2.jpg");
    }

#if 0
    {
        Model plane{OBJ::LoadFileData("Assets/plane.obj")};
        Mat44f transMat = Math::InitTranslation(0.0f, -1.5f, 0.0f);
        for (int i = 0; i < plane.verts.size(); ++i)
            plane.verts[i] = Math::MultiplyVecMat(plane.verts[i], transMat);
        app.LoadModel(plane);
        app.LoadTexture("Assets/wood.jpg");
    }
#endif

#if 0
    {
        Model cube{OBJ::LoadFileData("Assets/cube.obj")};
        app.LoadModel(cube);
        app.LoadTexture("Assets/bricks.jpg");
    }
#endif

    app.Start();
}


void RunTest(QApp& app)
{

#if 0
    // A triangle located on top right
    {
        Model topRightTri
        {
            InputWindingOrder::kCW,
            std::vector<Vec3f> {            // positions
                {-0.5f, 0.5f, 1.0f},
                {0.5f, 0.5f, 0.0f},
                {0.0f, -0.5f, -1.0f},
            },
            std::vector<Vec3f> {            // color
                {1.0f, 0.0f, 0.0f},
                {0.0f, 1.0f, 0.0f},
                {0.0f, 0.0f, 1.0f},
            },
            std::vector<int> {              // Indices
                0, 1, 2
            },
        };


        Mat44f transMat = Math::InitTranslation(1.5f, 1.0f, 0.0f);
        for (int i = 0; i < topRightTri.verts.size(); ++i)
            topRightTri.verts[i] = Math::MultiplyVecMat(topRightTri.verts[i], transMat);
        app.LoadModel(topRightTri);
    }
#endif

    {
        Model tri{
            InputWindingOrder::kCW,
            std::vector<Vec3f> {
                {-0.5f, -0.5f, 0.0f},
                {0.0f, 0.5f, -0.5f},
                {0.5f, -0.5f, 0.0f},
            },
            std::vector<Vec3f> {
                {1.0f, 0.0f, 0.0f},
                {0.0f, 1.0f, 0.0f},
                {0.0f, 0.0f, 1.0f},
            },
            std::vector<int> {
                0, 1, 2,
            },
        };
        Mat44f transMat = Math::InitTranslation(1.5f, 1.0f, 0.0f);
        for (int i = 0; i < tri.verts.size(); ++i)
            tri.verts[i] = Math::MultiplyVecMat(tri.verts[i], transMat);
        app.LoadModel(tri);
    }

#if 0
    {
        Model quad{
            InputWindingOrder::kCCW,
            std::vector<Vec3f> {
                {-1.0f, 1.0f, 1.0f},
                {-1.0f, 1.0f, -1.0f},
                {1.0f, 1.0f, -1.0f},
                {1.0f, 1.0f, 1.0f},
            },
            std::vector<Vec3f>(),
            std::vector<int> {
                0, 2, 1,
                0, 3, 2,
            },
            std::vector<Vec2f> {
                {0.0f, 1.0f},
                {0.0f, 0.0f},
                {1.0f, 0.0f},
                {1.0f, 1.0f},
            },
            std::vector<int> {
                0, 2, 1,
                0, 3, 2,
            },
        };

        Mat44f transMat = Math::InitTranslation(-1.5f, -2.0f, 0.0f);
        for (int i = 0; i < quad.verts.size(); ++i)
            quad.verts[i] = Math::MultiplyVecMat(quad.verts[i], transMat);
        app.LoadModel(quad);
        app.LoadTexture("Assets/checkerboard.jpg");
    }
#endif

    app.Start();
}

