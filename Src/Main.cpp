#include "SDL.h"
#include "SDL_image.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>

#include "Math/Vector.h"
#include "Math/Matrix.h"
#include "Renderer/OBJParser.h"
#include "Renderer/IndexModel.h"
#include "Renderer/Texture.h"
#include "Utils/Helper.h"
#include "Utils/Timer.h"
#include "Vertices.h"

// @todo REFACTOR AND SEPARATE THIS into Rasterizer.cpp, etc.

// Renderer: Based on pinhole cam model (view frustum)
// The mode doesn't change anything if device gate and film gate resolution are the same
enum class ResolutionGateMode
{
	kFill,		// Fit resolution gate into film gate (scale down)
	kOverscan,	// Fit film gate into resolution gate (scale up)
};
ResolutionGateMode g_resolutionGateMode = ResolutionGateMode::kOverscan;

void GetCanvasCoord(float focalLength, float zNear, ResolutionGateMode mode,
    float filmGateW, float filmGateH,
    int imgW, int imgH,
    float* canvasT, float* canvasR, float* canvasL, float* canvasB)
{
    float inchToMM = 25.4f;
	float filmAspectRatio = filmGateW / filmGateH;
	float deviceAspectRatio = (float)imgW / imgH;
	*canvasT = ((filmGateH * inchToMM ) / (2 * focalLength)) * zNear;
	*canvasR = *canvasT * filmAspectRatio;

	switch (mode)
	{
	default:
	case ResolutionGateMode::kFill:
	{
		if (filmAspectRatio > deviceAspectRatio)
		{
			*canvasR *= deviceAspectRatio / filmAspectRatio;
		}
		else
		{
			*canvasT *= filmAspectRatio / deviceAspectRatio;
		}
        break;
	}
	case ResolutionGateMode::kOverscan:
	{
		if (filmAspectRatio > deviceAspectRatio)
		{
			*canvasT *= filmAspectRatio / deviceAspectRatio;
		}
		else
		{
			*canvasR *= deviceAspectRatio / filmAspectRatio;
		}
        break;
	}
	}

	*canvasB = -*canvasT;
	*canvasL = -*canvasR;

	printf("Screen window coordinates: %f %f %f %f\n", *canvasB, *canvasL, *canvasT, *canvasR); 
    printf("Film Aspect Ratio: %f\nDevice Aspect Ratio: %f\n", filmAspectRatio, deviceAspectRatio); 
    printf("Angle of view: %f (deg)\n", 2 * atan((filmGateW * inchToMM / 2) / focalLength) * 180 / 3.14159265359f);

}

// Renderer: Determinant 2D between (c-a) and (b-a)
inline float EdgeFunction(const Vec3f& a, const Vec3f& b, const Vec3f& c)
{
    float result = (c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x);
    return result;
}

// Renderer: Pls remember not to pass a vector with z-value of 0.
inline float ComputeDepth(const Vec3f& v0, const Vec3f& v1, const Vec3f& v2, float w0, float w1, float w2)
{
    float depthInverse = (w0 / v0.z) + (w1 / v1.z) + (w2 / v2.z);
    float depth = (1 / depthInverse);
    return depth;
}

// Renderer: Resource: https://www.libsdl.org/release/SDL-1.2.15/docs/html/guidevideo.html
void PutPixel(unsigned char* pixels, unsigned color, int bpp, int pixelStride,
    int x, int y)
{
    unsigned char* p = pixels + (y * pixelStride + x) * bpp;
    *(Uint32 *)p = color;
}

// Renderer: Resource: https://github.com/ssloy/tinyrenderer/wiki/Lesson-1-Bresenham%E2%80%99s-Line-Drawing-Algorithm
void DrawLine(unsigned* pixels, unsigned color, int pxStride,
    int x0, int y0, int x1, int y1)
{
    bool steep = false; 
    // Transpose the img if line is steep
    if (std::abs(x0-x1)<std::abs(y0-y1)) 
    { 
        std::swap(x0, y0); 
        std::swap(x1, y1); 
        steep = true; 
    } 

    // Only need to care about octant 0-1, 6-7
    if (x0>x1) 
    { 
        std::swap(x0, x1); 
        std::swap(y0, y1); 
    } 

    int dx = x1 - x0; 
    int derror2 = std::abs(y1 - y0) * 2; 
    int error2 = 0; 
    int y = y0; 
    int yDir = y1 > y0 ? 1 : -1;
    for (int x = x0; x <= x1; ++x)
    { 
        if (steep) { PutPixel((unsigned char*)pixels, color, 4, pxStride, y, x); } 
        else { PutPixel((unsigned char*)pixels, color, 4, pxStride, x, y); } 

        error2 += derror2; 
        if (error2 > dx) 
        { 
            y += yDir; 
            error2 -= 2 * dx; 
        } 
    } 
}

// Renderer: Test DrawLine() by drawing line around the center of the img, offset by 18deg.
void TestDrawLine(SDL_Surface* surface, int w, int h)
{
    int x0 = w / 2 - 1;
    int y0 = h / 2 - 1;
    // We don't want to draw the line outside of w*h
    float length = (x0 > y0) ? (float)y0 : (float)x0;

    Math::Matrix<float, 2> rotMat{Math::InitIdentity<float, 2>()};
    float angle = 0.0f;
    float pi = (float)M_PI;
    unsigned red = SDL_MapRGB(surface->format, 255, 0, 0);
    unsigned green = SDL_MapRGB(surface->format, 0, 255, 0);
    unsigned white = SDL_MapRGB(surface->format, 255, 255, 255);

    while (angle < 2 * pi)
    {
        rotMat(0, 0) = cos(angle);
        rotMat(0, 1) = sin(angle);
        rotMat(1, 0) = -sin(angle);
        rotMat(1, 1) = cos(angle);
        Vec2f endPf = MultiplyVecMat(Vec2f{length, 0.0f}, rotMat);
        int x1 = (int)endPf.x + x0;
        int y1 = (int)endPf.y + y0;
        if (Helper::IsEqual(angle, 0.0f) || Helper::IsEqual(angle, pi))
        {
            DrawLine((unsigned*)surface->pixels, red, w, x0, y0, x1, y1);
        }
        else if (Helper::IsEqual(angle, pi / 2) || Helper::IsEqual(angle, (3 * pi / 2)))
        {
            DrawLine((unsigned*)surface->pixels, green, w, x0, y0, x1, y1);
        }
        else
        {
            DrawLine((unsigned*)surface->pixels, white, w, x0, y0, x1, y1);
        }

        angle += (pi / 10);
    }
}

// Renderer (our rasterizer): Gamma decoded LUT with gamma = 2.2, 8-bit.
// @see https://scantips.com/lights/gamma3.html
constexpr unsigned char g_gammaDecodedTable[256] = 
{    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   1,
     1,   1,   1,   1,   1,   1,   1,   1,   1,   2,   2,   2,   2,   2,   2,   2,
     3,   3,   3,   3,   3,   4,   4,   4,   4,   5,   5,   5,   5,   6,   6,   6,
     6,   7,   7,   7,   8,   8,   8,   9,   9,   9,  10,  10,  11,  11,  11,  12,
    12,  13,  13,  13,  14,  14,  15,  15,  16,  16,  17,  17,  18,  18,  19,  19,
    20,  20,  21,  22,  22,  23,  23,  24,  25,  25,  26,  26,  27,  28,  28,  29,
    30,  30,  31,  32,  33,  33,  34,  35,  35,  36,  37,  38,  39,  39,  40,  41,
    42,  43,  43,  44,  45,  46,  47,  48,  49,  49,  50,  51,  52,  53,  54,  55,
    56,  57,  58,  59,  60,  61,  62,  63,  64,  65,  66,  67,  68,  69,  70,  71,
    73,  74,  75,  76,  77,  78,  79,  81,  82,  83,  84,  85,  87,  88,  89,  90,
    91,  93,  94,  95,  97,  98,  99, 100, 102, 103, 105, 106, 107, 109, 110, 111,
   113, 114, 116, 117, 119, 120, 121, 123, 124, 126, 127, 129, 130, 132, 133, 135,
   137, 138, 140, 141, 143, 145, 146, 148, 149, 151, 153, 154, 156, 158, 159, 161,
   163, 165, 166, 168, 170, 172, 173, 175, 177, 179, 181, 182, 184, 186, 188, 190,
   192, 194, 196, 197, 199, 201, 203, 205, 207, 209, 211, 213, 215, 217, 219, 221,
   223, 225, 227, 229, 231, 234, 236, 238, 240, 242, 244, 246, 248, 251, 253, 255
};

// Gamma correct.
unsigned char DecodeGamma(float value)
{
    int quantizedValue = (int)(value * 255 + 0.5f);
    return g_gammaDecodedTable[quantizedValue];
}

// Renderer
void DrawDepth(SDL_Surface* surface, const std::vector<float>& zBuffer, int x, int y, int pxStride)
{
    float intensity = (1.0f - zBuffer[x + y * pxStride]) * 0.5f;
    auto decodedColor = DecodeGamma(intensity);
    unsigned color = SDL_MapRGB(surface->format, decodedColor, decodedColor, decodedColor);
    PutPixel((unsigned char*)surface->pixels, color, 4, pxStride, x, y);
}

// Renderer: Pinhole camera settings
float g_focalLength = 35.0f;	// mm
float g_filmGateW = 0.885f;     // inch
float g_filmGateH = 0.885f;
const Mat44f g_worldToCam = {0.707107f, -0.331295f, 0.624695f, 0.0f,
    0.0f, 0.883452f, 0.468521f, 0.0f,
    -0.707107f, -0.331295f, 0.624695f, 0.0f,
    -1.63871f, -5.747777f, -40.400412f, 1.0f};

// Renderer
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

// Renderer: Cam
Mat44f LookAt(Vec3f eye, Vec3f center, Vec3f up)
{
    Mat44f result;
    Vec3f z = Normal(eye - center);
    Vec3f x = Normal(Cross(up, z));
    Vec3f y = Normal(Cross(z, x));
    Mat44f minV{Math::InitIdentity<float, 4>()};
    Mat44f tr{Math::InitIdentity<float, 4>()};
    for (int i = 0; i < 3; ++i)
    {
        minV(0, i) = x[i];
        minV(1, i) = y[i];
        minV(2, i) = z[i];
        tr(i, 3) = -center[i];
    }
    result = minV * tr;

    return result;
}

// Renderer
Vec3f NDCToRaster(const Vec3f& v, int w, int h)
{
    Vec3f result;
    result.x = (1 + v.x) * 0.5f * w;
    // In raster space, y is down so we need to invert the direction
    result.y = (1 - v.y) * 0.5f * h;
    result.z = v.z;
    return result;
}

// App
static bool isPaused;

// Input
void DoKeyDown(SDL_KeyboardEvent* e)
{
    if (e->repeat == 0)
    {
        if (e->keysym.sym == SDLK_b)
        {
            isPaused = !isPaused;
        }
    }
}

// App: Img resolution in pixels
constexpr int imgW = 800;
constexpr int imgH = 800;
constexpr int bpp = 4;

// App: setup
SDL_Window* g_window;
TextureManager& g_texManager = TextureManager::Instance();

// v0 v1 v2 are assumed in CW order
void DrawTriangle(unsigned char* pxBuffer, float* zBuffer, Vec3f v0, Vec3f v1, Vec3f v2, unsigned color)
{
    // Renderer settings
    float zNear = 0.125f;
    float zFar = 100.0f;
    float fovY = 60.0f * (float)M_PI / 180.0f;
    float imgAspectRatio = (float)imgW / imgH;
    Mat44f viewMat = Math::InitTranslation(0.0f, 0.0f, -3.0f);
    Mat44f projMat = Math::InitPersp(fovY, imgAspectRatio, zNear, zFar);

    // x, y in range [-1, 1], z in range [0, 1]
    v0 = MultiplyPtMat(v0, viewMat * projMat);
    v1 = MultiplyPtMat(v1, viewMat * projMat);
    v2 = MultiplyPtMat(v2, viewMat * projMat);

    Vec3f v0Raster = NDCToRaster(v0, imgW, imgH);
    Vec3f v1Raster = NDCToRaster(v1, imgW, imgH);
    Vec3f v2Raster = NDCToRaster(v2, imgW, imgH);

    // Prep for edge test
    Vec3f edge0 = v2Raster - v1Raster;
    Vec3f edge1 = v0Raster - v2Raster;
    Vec3f edge2 = v1Raster - v0Raster;
    float areaTriTimes2 = EdgeFunction(v0Raster, v1Raster, v2Raster);

    Vec2i bbMin, bbMax;
    bbMin.x = std::max(0, (int)Helper::Min3(v0Raster.x, v1Raster.x, v2Raster.x));
    bbMin.y = std::max(0, (int)Helper::Min3(v0Raster.y, v1Raster.y, v2Raster.y));
    bbMax.x = std::min((int)imgW, (int)Helper::Max3(v0Raster.x, v1Raster.x, v2Raster.x));
    bbMax.y = std::min((int)imgH, (int)Helper::Max3(v0Raster.y, v1Raster.y, v2Raster.y));

    for (int y = bbMin.y; y <= bbMax.y; ++y)
    {
        for (int x = bbMin.x; x <= bbMax.x; ++x)
        {
            Vec3f px{x + 0.5f, y + 0.5f, 0.0f};
            float e12 = EdgeFunction(v1Raster, v2Raster, px);
            float e20 = EdgeFunction(v2Raster, v0Raster, px);
            float e01 = EdgeFunction(v0Raster, v1Raster, px);

            // Top-left rule and edge test combined (using CW winding order)
            // Top edge: horizontal, to the right: y == 0 && x > 0
            // Left edge: goes up: y > 0
            // Ex: w0=0 then px lies on v1v2, so test for top left, else test if inside tri
            bool overlap = true;
            overlap &= Helper::IsEqual(e12, 0.0f) ?
                (Helper::IsEqual(edge0.y, 0.0f) && edge0.x > 0) || edge0.y > 0.0f :
            (e12 > 0.0f);

            overlap &= Helper::IsEqual(e20, 0.0f) ?
                (Helper::IsEqual(edge1.y, 0.0f) && edge1.x > 0) || edge1.y > 0.0f :
            (e20 > 0.0f);

            overlap &= Helper::IsEqual(e01, 0.0f) ?
                (Helper::IsEqual(edge2.y, 0.0f) && edge2.x > 0) || edge2.y > 0.0f :
            (e01 > 0.0f);
            if (!overlap) { continue; }

            float w0 = e12 / areaTriTimes2;
            float w1 = e20 / areaTriTimes2;
            float w2 = e01 / areaTriTimes2;
            px.z = w0 * v0Raster.z + w1 * v1Raster.z + w2 * v2Raster.z;
            // = sign here because we want to draw the img next frame too
            if (zBuffer[y * imgW + x] >= px.z)
            {
                zBuffer[y * imgW + x] = px.z;
                PutPixel(pxBuffer, color, bpp, imgW, (int)px.x, (int)px.y);
            }

        }
    }
}

// Renderer
void DrawTriangles(const IndexModel& model, unsigned char* pxBuffer, float* zBuffer)
{
    unsigned format = SDL_GetWindowPixelFormat(g_window);
    SDL_PixelFormat* mappingFormat = SDL_AllocFormat(format);
    
    Vec3f lightDir{0.0f, 0.0f, -1.0f};

    for (int i = 0, n = (int)model.vertIndices.size(); i < n; i += 3)
    {
        // NOTE: passed in verts must be in CCW order (like .obj file), so that after they are
        // converted to raster space (y points downward), they will appear in CW order.
        // We assume cam is pointing towards -z dir. If z is +, it's reversed (see pinhole cam)
        Vec3f v0 = model.verts[model.vertIndices[i]];
        Vec3f v1 = model.verts[model.vertIndices[i + 1]];
        Vec3f v2 = model.verts[model.vertIndices[i + 2]];

        // Apply lighting
        Vec3f faceNormal = Normal(Cross(v2 - v0, v1 - v0));
        float intensity = Dot(faceNormal, lightDir);
        if (intensity <= 0.0f) { continue; }
        unsigned char light = DecodeGamma(std::min(1.0f, intensity));
        unsigned color = SDL_MapRGBA(mappingFormat, light, light, light, 0);

        DrawTriangle(pxBuffer, zBuffer, v0, v1, v2, color);

    }
}

template<typename T>
struct Foo
{
    template<typename U>
    Foo(U&& v) : v{static_cast<U&&>(v)} {}

    T v;
};

// NOTE: it seems coords of .obj file are in NDC space -> Convert to Raster space. A px, bb, w, zBuffer,
// are all floats, only final PutPixel is floored to int
// Orthographic projection keeps x, y unchanged, and it seems zBuffer stays the same
// Obj file keeps x, y, z in range [-1, 1], which is canoncial viewing volume
// Current progression: Simple .obj file > Draw Wireframe (bresenham) > Rasterization (orthographic and back face
// culling) > z-buffer (don't need perspective correction)
int main(int argc, char** argv)
{
    IndexModel model = OBJ::LoadFileData("Assets/AfricanHead.obj");
    SDL_Init(SDL_INIT_VIDEO);
    std::string title{"Rasterizer"};
    g_window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, imgW, imgH, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(g_window, -1, 0);
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, imgW, imgH);

    // Init jpeg file
    int imgFlags = IMG_INIT_JPG;
    if (!(IMG_Init(imgFlags) & imgFlags))
    {
        std::cout << "SDL_image can't be initialized! SDL_image error: " << IMG_GetError();
        return -1;
    }
    TextureWrapper tex;
    tex.Acquire("Assets/bricks.jpg", renderer);

    std::vector<unsigned char> bg(imgW * imgH * bpp, 50);
    std::vector<unsigned char> frameBuffer(imgW * imgH * bpp, 50);
    std::vector<float> zBuffer(imgW * imgH, FLT_MAX);
    std::vector<float> bgZ(imgW * imgH, FLT_MAX);

    bool isRunning = true;
    Timer frameTimer;
    frameTimer.Reset();
    float accumulatedTime = 0.0f;
    while (isRunning)
    {
        for (SDL_Event e; SDL_PollEvent(&e);)
        {
            switch (e.type)
            {
            case SDL_QUIT:
            {
                isRunning = false;
                break;
            }
            case SDL_KEYDOWN:
            {
                DoKeyDown(&e.key);
                break;
            }
            }
        }

        if (isPaused) { continue; }
        frameTimer.Tick();
        float dt = (float)frameTimer.GetDeltaTime();
        accumulatedTime += dt;
        // Spiral of Doom protection
        if (accumulatedTime > dt * 8)
        {
            accumulatedTime = dt;
        }

        // Display frame statistics every sec
        static int totalFrameCnt = 0;
        static float total = 0.0f;
        ++totalFrameCnt;
        static double timeElapsed = 0.0;
        if (frameTimer.GetTotalTime() - timeElapsed > 1.0)
        {
            float fps = 1.0f / dt;
            std::string toPrint = title + " - Current frame time: " + std::to_string(dt) + "s - " + std::to_string(fps) +
                "fps - Avg frame time: " + std::to_string(frameTimer.GetTotalTime() / totalFrameCnt) + "s over " +
                std::to_string(totalFrameCnt) + "frames";
            SDL_SetWindowTitle(g_window, toPrint.c_str());
            ++timeElapsed;
        }

        // Clear screen
        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 50);
        SDL_RenderClear(renderer);
        frameBuffer = bg;
        zBuffer = bgZ;

#if 0
        std::vector<Vec3f> verts{{-1.0f, -1.0f, 0.0f},
            {0.0f, 1.0f, 0.0f},
            {1.0f, -1.0f, 0.0f},};
        std::vector<int> vertIndex{0, 1, 2, 2, 1, 0};
        IndexModel myModel{verts, vertIndex};
#endif

#if 0
        while (accumulatedTime >= dt)
        {
            Update(dt);
            accumulatedTime -= dt;
        }
#endif
#if 0
        // Physics loop
        float rotatedPerSec = (float)M_PI * dt / 4;
        Mat44f rotMat = Math::InitRotation(0.0f, rotatedPerSec, 0.0f);
        for (int i = 0, n = (int)model.verts.size(); i < n; ++i)
        {
            model.verts[i] = MultiplyVecMat(model.verts[i], rotMat);
        }
#endif

        // Render loop
#if 0
        // Peek into the future and generate the output
        Render(accumulatedTime / dt);
#endif
#if 0
        DrawTriangles(model, frameBuffer.data(), zBuffer.data());
        SDL_UpdateTexture(texture, nullptr, frameBuffer.data(), imgW * bpp);

        // Render texture to screen and update the screen
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
#endif
        SDL_RendererFlip flip = SDL_FLIP_NONE;
        SDL_Rect clip{tex.Width() / 6, tex.Height() / 6, tex.Width() / 2, tex.Height() / 2};
        tex.Render(renderer, imgW / 4, imgH / 4, &clip, flip);
        SDL_RenderPresent(renderer);

    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(g_window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}

