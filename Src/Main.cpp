#include <iostream>

#include "QApp.h"

#if 0
// @brief Test if depth calculation is correct by drawing the depth buffer on screen
void DrawDepth(SDL_Surface* surface, const std::vector<float>& zBuffer, int x, int y, int pxStride)
{
    float intensity = (1.0f - zBuffer[x + y * pxStride]) * 0.5f;
    auto decodedColor = DecodeGamma(intensity);
    unsigned color = SDL_MapRGB(surface->format, decodedColor, decodedColor, decodedColor);
    PutPixel((unsigned char*)surface->pixels, color, 4, pxStride, x, y);
}

// @brief Draw wireframe of the obj on screen
void DrawWireframe(SDL_Surface* surface, const IndexModel& model)
{
    int w = surface->w;
    int h = surface->h;
    for (int i = 0; i < model.vertIndices.size(); i += 3)
    {
        Vec3f v0Raster = model.verts[model.vertIndices[i]];
        Vec3f v1Raster = model.verts[model.vertIndices[i + 1]];
        Vec3f v2Raster = model.verts[model.vertIndices[i + 2]];

        int x0 = (int)((v0Raster.x + 1) / 2.0f * w);
        int y0 = (int)((1 - v0Raster.y) / 2.0f * h);

        int x1 = (int)((v1Raster.x + 1) / 2.0f * w);
        int y1 = (int)((1 - v1Raster.y) / 2.0f * h);

        int x2 = (int)((v2Raster.x + 1) / 2.0f * w);
        int y2 = (int)((1 - v2Raster.y) / 2.0f * h);

        // No clipping yet!
        int xMin = Helper::Min3(x0, x1, x2);
        int yMin = Helper::Min3(y0, y1, y2);
        int xMax = Helper::Max3(x0, x1, x2);
        int yMax = Helper::Max3(y0, y1, y2);
        if (xMax > w - 1 || xMin < 0 || yMax > h - 1 || yMin < 0) { continue; }

        DrawLine((unsigned*)surface->pixels, SDL_MapRGB(surface->format, 255, 255, 255),
            w, x0, y0, x1, y1);
        DrawLine((unsigned*)surface->pixels, SDL_MapRGB(surface->format, 255, 255, 255),
            w, x1, y1, x2, y2);
        DrawLine((unsigned*)surface->pixels, SDL_MapRGB(surface->format, 255, 255, 255),
            w, x2, y2, x0, y0);
    }
}
#endif

// @brief Main entry point of the program
int main(int argc, char** argv)
{
#if 0
    IndexModel model = OBJ::LoadFileData("Assets/AfricanHead.obj");
#endif

    QApp& app = QApp::Instance();
    if (app.Init()) { app.Start(); }
    else { std::cout << "App couldn't be initialized properly. Shutting down...\n"; }

    app.Shutdown();
    return 0;

}

