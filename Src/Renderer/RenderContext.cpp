#include "SDL.h"    // Uint32

#include <cmath>

#include "Renderer/RenderContext.h"
#include "Renderer/IndexModel.h"
#include "Renderer/Texture.h"
#include "Utils/Helper.h"

unsigned char RenderContext::DecodeGamma(float value)
{
    int quantizedValue = (int)(value * 255 + 0.5f);
    return g_gammaDecodedTable[quantizedValue];
}

void RenderContext::DrawTriangle(SDL_Renderer* renderer, const QTexture& bitmap, float* zBuffer, Vec3f v0, Vec3f v1, Vec3f v2,
    Vec2f uv0, Vec2f uv1, Vec2f uv2, const QTexture& tex)
{
    // Renderer settings. We make sure that the obj is shifted to -z coord.
    float zNear = 0.125f;
    float zFar = 100.0f;
    float fovY = 60.0f * (float)M_PI / 180.0f;
    float imgAspectRatio = (float)bitmap.GetW() / bitmap.GetH();
    Mat44f viewMat = Math::InitTranslation(0.0f, 0.0f, -3.0f);
    Mat44f projMat = Math::InitPersp(fovY, imgAspectRatio, zNear, zFar);

    // x, y in range [-1, 1], z in range [0, 1]
#if 0
    v0 = MultiplyPtMat(v0, viewMat * projMat);
    v1 = MultiplyPtMat(v1, viewMat * projMat);
    v2 = MultiplyPtMat(v2, viewMat * projMat);
#endif

    Vec3f v0Raster = NDCToRaster(v0, bitmap.GetW(), bitmap.GetH());
    Vec3f v1Raster = NDCToRaster(v1, bitmap.GetW(), bitmap.GetH());
    Vec3f v2Raster = NDCToRaster(v2, bitmap.GetW(), bitmap.GetH());

    // Prep for edge test
    Vec3f edge0 = v2Raster - v1Raster;
    Vec3f edge1 = v0Raster - v2Raster;
    Vec3f edge2 = v1Raster - v0Raster;
    float areaTriTimes2 = ComputeEdge(v0Raster, v1Raster, v2Raster);

    Vec2i bbMin, bbMax;
    bbMin.x = std::max(0, (int)Helper::Min3(v0Raster.x, v1Raster.x, v2Raster.x));
    bbMin.y = std::max(0, (int)Helper::Min3(v0Raster.y, v1Raster.y, v2Raster.y));
    bbMax.x = std::min((int)bitmap.GetW(), (int)Helper::Max3(v0Raster.x, v1Raster.x, v2Raster.x));
    bbMax.y = std::min((int)bitmap.GetH(), (int)Helper::Max3(v0Raster.y, v1Raster.y, v2Raster.y));

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
            if (zBuffer[y * bitmap.GetW() + x] >= px.z)
            {
                zBuffer[y * bitmap.GetW() + x] = px.z;
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

