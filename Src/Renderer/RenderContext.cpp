#include "SDL.h"    // Uint32

#include <cmath>

#include "Renderer/RenderContext.h"
#include "Renderer/IndexModel.h"
#include "Renderer/Texture.h"
#include "Utils/Helper.h"

void RenderContext::DrawPt(unsigned char* pixels, unsigned color, int bpp, int pixelStride,
    int x, int y)
{
    unsigned char* p = pixels + (y * pixelStride + x) * bpp;
    *(unsigned *)p = color;
}

void RenderContext::DrawLine(unsigned char* pixels, unsigned color, int bpp, int pixelStride,
    int x0, int x1, int y0, int y1)
{
    bool isSteep = false;
    if (std::abs(y1 - y0) > std::abs(x1 - x0))
    {
        isSteep = true;
        std::swap(x0, y0);
        std::swap(x1, y1);
    }

    if (x0 > x1)
    {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    int dx = x1 - x0;
    int dy = y1 - y0;
    int yDir = 1;
    if (dy < 0)
    {
        dy = -dy;
        yDir = -1;
    }
    int e2 = 2 * dy - dx;
    int y = y0;
    for (int x = x0; x <= x1; ++x)
    {
        if (isSteep) { DrawPt(pixels, color, bpp, pixelStride, y, x); }
        else { DrawPt(pixels, color, bpp, pixelStride, x, y); }
        e2 += 2 * dy;
        if (e2 > 0)
        {
            e2 -= 2 * dx;
            y += yDir;
        }
    }

}

void RenderContext::TestDrawLine(unsigned char *pixels, int scrW, int scrH, const SDL_PixelFormat *format)
{
    float pi = (float)M_PI;
    float angle = pi / 6;
    float rotMat[4] = {cos(angle), sin(angle), -sin(angle), cos(angle)};

    unsigned red = SDL_MapRGBA(format, 255, 0, 0, 255);
    unsigned green = SDL_MapRGBA(format, 0, 255, 0, 255);

    int x0 = scrW / 2 - 1;
    int y0 = scrH / 2 - 1;
    // Define length as right before width/height (choose the smaller val so it doesn't extend outside of screen)
    float length = (x0 > y0) ? (float)y0 : (float)x0;
    float endP[2] = {length, 0.0f};

    for (int i = 0; i < 12; ++i)
    {
        float newEndP[2]{endP[0] * rotMat[0] + endP[1] * rotMat[2], endP[0] * rotMat[1] + endP[1] * rotMat[3]};
        int x1 = (int)endP[0] + x0;
        int y1 = (int)endP[1] + y0;
        if (x1 - x0 == 0 || y1 - y0 == 0)
        {
            DrawLine(pixels, red, 4, scrW, x0, x1, y0, y1);
        }
        else
        {
            DrawLine(pixels, green, 4, scrW, x0, x1, y0, y1);
        }

        endP[0] = newEndP[0];
        endP[1] = newEndP[1];
    }

}

#if 0
// @brief Test if depth calculation is correct by drawing the depth buffer on screen
void DrawDepth(SDL_Surface* surface, const std::vector<float>& zBuffer, int x, int y, int pxStride)
{
    float intensity = (1.0f - zBuffer[x + y * pxStride]) * 0.5f;
    auto decodedColor = DecodeGamma(intensity);
    unsigned color = SDL_MapRGB(surface->format, decodedColor, decodedColor, decodedColor);
    PutPixel((unsigned char*)surface->pixels, color, 4, pxStride, x, y);
}

// @brief Draw wire frame of the obj on screen
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

float RenderContext::ComputeDepth(const Vec3f& v0, const Vec3f& v1, const Vec3f& v2, float w0, float w1, float w2)
{
    float depthInverse = (w0 / v0.z) + (w1 / v1.z) + (w2 / v2.z);
    float depth = (1 / depthInverse);
    return depth;
}

unsigned char RenderContext::DecodeGamma(float value)
{
    int quantizedValue = (int)(value * 255 + 0.5f);
    return g_gammaDecodedTable[quantizedValue];
}

void RenderContext::DrawTriangle(SDL_Renderer* renderer, const TextureWrapper& bitmap, float* zBuffer, Vec3f v0, Vec3f v1, Vec3f v2,
    Vec2f uv0, Vec2f uv1, Vec2f uv2, const TextureWrapper& tex)
{
    // Renderer settings. We make sure that the obj is shifted to -z coord.
    float zNear = 0.125f;
    float zFar = 100.0f;
    float fovY = 60.0f * (float)M_PI / 180.0f;
    float imgAspectRatio = (float)bitmap.Width() / bitmap.Height();
    Mat44f viewMat = Math::InitTranslation(0.0f, 0.0f, -3.0f);
    Mat44f projMat = Math::InitPersp(fovY, imgAspectRatio, zNear, zFar);

    // x, y in range [-1, 1], z in range [0, 1]
#if 0
    v0 = MultiplyPtMat(v0, viewMat * projMat);
    v1 = MultiplyPtMat(v1, viewMat * projMat);
    v2 = MultiplyPtMat(v2, viewMat * projMat);
#endif

    Vec3f v0Raster = NDCToRaster(v0, bitmap.Width(), bitmap.Height());
    Vec3f v1Raster = NDCToRaster(v1, bitmap.Width(), bitmap.Height());
    Vec3f v2Raster = NDCToRaster(v2, bitmap.Width(), bitmap.Height());

    // Prep for edge test
    Vec3f edge0 = v2Raster - v1Raster;
    Vec3f edge1 = v0Raster - v2Raster;
    Vec3f edge2 = v1Raster - v0Raster;
    float areaTriTimes2 = ComputeEdge(v0Raster, v1Raster, v2Raster);

    Vec2i bbMin, bbMax;
    bbMin.x = std::max(0, (int)Helper::Min3(v0Raster.x, v1Raster.x, v2Raster.x));
    bbMin.y = std::max(0, (int)Helper::Min3(v0Raster.y, v1Raster.y, v2Raster.y));
    bbMax.x = std::min((int)bitmap.Width(), (int)Helper::Max3(v0Raster.x, v1Raster.x, v2Raster.x));
    bbMax.y = std::min((int)bitmap.Height(), (int)Helper::Max3(v0Raster.y, v1Raster.y, v2Raster.y));

    for (int y = bbMin.y; y <= bbMax.y; ++y)
    {
        for (int x = bbMin.x; x <= bbMax.x; ++x)
        {
            Vec3f px{x + 0.5f, y + 0.5f, 0.0f};
            float e12 = ComputeEdge(v1Raster, v2Raster, px);
            float e20 = ComputeEdge(v2Raster, v0Raster, px);
            float e01 = ComputeEdge(v0Raster, v1Raster, px);

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
            
            // Affine texture mapping
            Vec2f uv = uv0 * w0 + uv1 * w1 + uv2 * w2;

            // = sign here because we want to draw the img next frame too
            if (zBuffer[y * bitmap.Width() + x] >= px.z)
            {
                zBuffer[y * bitmap.Width() + x] = px.z;
#if 0
                tex.RenderPixel(renderer, (int)(uv.x * tex.Width()), (int)(uv.y * tex.Height()),
                    (int)px.x, (int)px.y);
#endif
#if 0
                unsigned format = SDL_GetWindowPixelFormat(g_window);
                SDL_PixelFormat* mappingFormat = SDL_AllocFormat(format);
                unsigned color = SDL_MapRGBA(mappingFormat, 0xff, 0xff, 0xff, 0xff);
                PutPixel(pxBuffer, color, bpp, imgW, (int)px.x, (int)px.y);
#endif
            }

        }
    }
}

#if 0
void RenderContext::DrawTriangles(const IndexModel& model, unsigned char* pxBuffer, float* zBuffer, const TextureWrapper& tex)
{
    Vec3f lightDir{0.0f, 0.0f, -1.0f};

    for (int i = 0, n = (int)model.vertIndices.size(); i < n; i += 3)
    {
        // @remark passed in verts must be in CCW order (like .obj file), so that after they are
        // converted to raster space (y points downward), they will appear in CW order.
        // We assume cam is pointing towards -z dir. If z is +, it's reversed (see pinhole cam)
        Vec3f v0 = model.verts[model.vertIndices[i]];
        Vec3f v1 = model.verts[model.vertIndices[i + 1]];
        Vec3f v2 = model.verts[model.vertIndices[i + 2]];

#if 0
        // Apply lighting
        Vec3f faceNormal = Normal(Cross(v2 - v0, v1 - v0));
        float intensity = Dot(faceNormal, lightDir);
        if (intensity <= 0.0f) { continue; }

        unsigned char light = DecodeGamma(std::min(1.0f, intensity));
        unsigned color = SDL_MapRGBA(mappingFormat, light, light, light, 0);
#endif

        Vec2f uv0 = model.texCoords[model.uvIndices[i]];
        Vec2f uv1 = model.texCoords[model.uvIndices[i + 1]];
        Vec2f uv2 = model.texCoords[model.uvIndices[i + 2]];

        DrawTriangle(pxBuffer, zBuffer, v0, v1, v2,
            uv0, uv1, uv2, tex);

    }
}
#endif

