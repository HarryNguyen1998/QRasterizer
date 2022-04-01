#include <algorithm>
#include <cassert>
#include <deque>
#include <iostream>

#include "Renderer/Model.h"
#include "Renderer/Rasterizer.h"
#include "Renderer/QRenderer.h"
#include "Renderer/Texture.h"
#include "Renderer/Triangle.h"

Vec3f color;

void Rasterizer::Rasterize(uint32_t *pixels, float *zBuffer, int w, int h, const Model& model, const Mat44f& projMat, QRendererMode mode)
{
    assert(!model.verts.empty() && "Uh oh, model is empty!");

    for (int i = 0; i < model.vertIndices.size(); i += 3)
    {
        Vec3f v0 = model.verts[model.vertIndices[i]];
        Vec3f v1 = model.verts[model.vertIndices[i + 1]];
        Vec3f v2 = model.verts[model.vertIndices[i + 2]];
        Vec3f c0 = model.colors[i];
        Vec3f c1 = model.colors[i + 1];
        Vec3f c2 = model.colors[i + 2];

        // Back face culling in cam space
        Vec3f surfNormal = Math::Normal(Math::Cross(v2 - v0, v1 - v0));
        if (Math::Dot(v0, surfNormal) > 0.0f)
            continue;

        // Flat shading. @note z-axis isn't inverted here
        Vec3f lightDir = Math::Normal(Vec3f{0.0f, -1.0f, -1.0f});
        // @note Since it survives back face culling, surfNormal should be (+)
        float dp = Math::Dot(lightDir, surfNormal);
        c0 *= -dp;
        c1 *= -dp;
        c2 *= -dp;

        // To clip space. @note z-axis is inverted here to range [0, w];
        std::vector<float> wCoords = {-v0.z, -v1.z, -v2.z};
        v0 = Math::MultiplyVecMat(v0, projMat);
        v1 = Math::MultiplyVecMat(v1, projMat);
        v2 = Math::MultiplyVecMat(v2, projMat);

        // Clipping
        enum Plane
        {
            kNear,
            kTop,
            kRight,
            kBottom,
            kLeft,
            kCount
        };
        std::deque<Triangle> clippedTris;
        // No uv, so set to 0
        clippedTris.push_back(Triangle(v0, v1, v2, Vec2f(0.0f), Vec2f(0.0f), Vec2f(0.0f),
            wCoords[0], wCoords[1], wCoords[2], c0, c1, c2));
        for (int p = Plane::kNear; p != Plane::kCount; ++p)
        {
            size_t oldCnt = clippedTris.size();
            while (oldCnt-- > 0)
            {
                Triangle tri = clippedTris.front();
                clippedTris.pop_front();
                std::vector<Triangle> newTris;
                switch (p)
                {
                case Plane::kNear:
                {
                    newTris = ClipTriangleAgainstPlane({0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.5f}, tri);
                    break;
                }
                case Plane::kTop:
                {
                    newTris = ClipTriangleAgainstPlane({0.0f, -1.0f, 1.0f}, Vec3f(0.0f), tri);
                    break;
                }
                case Plane::kRight:
                {
                    newTris = ClipTriangleAgainstPlane({-1.0f, 0.0f, 1.0f}, Vec3f(0.0f), tri);
                    break;
                }
                case Plane::kBottom:
                {
                    newTris = ClipTriangleAgainstPlane({0.0f, 1.0f, 1.0f}, Vec3f(0.0f), tri);
                    break;
                }
                case Plane::kLeft:
                {
                    newTris = ClipTriangleAgainstPlane({1.0f, 0.0f, 1.0f}, Vec3f(0.0f), tri);
                    break;
                }
                }   // End switch

                for (auto &newTri : newTris)
                    clippedTris.push_back(std::move(newTri));

            }   // End while

        }
    
        for (int j = 0; j < clippedTris.size(); ++j)
        {
            // Perspective divide to NDC space
            v0 = clippedTris[j].verts[0] / clippedTris[j].wCoords[0];
            v1 = clippedTris[j].verts[1] / clippedTris[j].wCoords[1];
            v2 = clippedTris[j].verts[2] / clippedTris[j].wCoords[2];
            c0 = clippedTris[j].colors[0];
            c1 = clippedTris[j].colors[1];
            c2 = clippedTris[j].colors[2];

            // To raster space
            v0.x = (v0.x + 1.0f) * w / 2;
            v0.y = (v0.y + 1.0f) * h / 2;
            v1.x = (v1.x + 1.0f) * w / 2;
            v1.y = (v1.y + 1.0f) * h / 2;
            v2.x = (v2.x + 1.0f) * w / 2;
            v2.y = (v2.y + 1.0f) * h / 2;
            float oneOverW0 = 1.0f / clippedTris[j].wCoords[0];
            float oneOverW1 = 1.0f / clippedTris[j].wCoords[1];
            float oneOverW2 = 1.0f / clippedTris[j].wCoords[2];

            float areaOfParallelogram = ComputeEdge(v0, v1, v2);
            if (Helper::IsEqual(areaOfParallelogram, 0.0f))
                continue;
            int bbMinX = (int)Helper::Min3(v0.x, v1.x, v2.x);
            int bbMaxX = (int)Helper::Max3(v0.x, v1.x, v2.x);
            int bbMinY = (int)Helper::Min3(v0.y, v1.y, v2.y);
            int bbMaxY = (int)Helper::Max3(v0.y, v1.y, v2.y);

            bbMinX = std::max(0, bbMinX);
            bbMinY = std::max(0, bbMinY);
            bbMaxX = std::min(w - 1, bbMaxX);
            bbMaxY = std::min(h - 1, bbMaxY);

            for (int y = bbMinY; y <= bbMaxY; ++y)
            {
                for (int x = bbMinX; x <= bbMaxX; ++x)
                {
                    Vec3f pt{(float)x, (float)y, 0.0f};

                    // Inside-outside test
                    float e12 = ComputeEdge(v1, v2, pt);
                    float e20 = ComputeEdge(v2, v0, pt);
                    float e01 = ComputeEdge(v0, v1, pt);
                    if (e01 < 0.0f || e12 < 0.0f || e20 < 0.0f)
                        continue;

                    assert(!Helper::IsEqual(areaOfParallelogram, 0.0f));
                    float t0 = e12 / areaOfParallelogram;
                    float t1 = e20 / areaOfParallelogram;
                    float t2 = e01 / areaOfParallelogram;
                    float oneOverW = t0 * oneOverW0 + t1 * oneOverW1 + t2 * oneOverW2;
                    color = (1.0f / oneOverW) * (c0 * oneOverW0 * t0 + c1 * oneOverW1 * t1 + c2 * oneOverW2 * t2);

                    uint8_t r = ClampChannel(color.r);
                    uint8_t g = ClampChannel(color.g);
                    uint8_t b = ClampChannel(color.b);

                    // @note If z < zBuffer, the triangle is closer, and update new zBuffer.
                    // Instead, since we use oneOverZ, it's actually inverse, and zBuffer filled
                    // with 0 actually represent the furthest (infinitely)
                    if (oneOverW > zBuffer[x + y * w])
                    {
                        if (mode == QRendererMode::kZBuffer)
                        {
                            uint8_t c = ClampChannel(oneOverW);
                            pixels[x + y * w] = ToColor(c, c, c, 255);
                            zBuffer[x + y * w] = oneOverW;
                        }
                        else
                        {
                            pixels[x + y * w] = ToColor(r, g, b, 255);
                            zBuffer[x + y * w] = oneOverW;
                        }
                    }
                }
            }
        }   // End of insidePts

    }   // End of vertIndices

    assert(!model.verts.empty() && "Uh oh, model is empty!");

    for (int i = 0; i < model.vertIndices.size(); i += 3)
    {
        Vec3f v0 = model.verts[model.vertIndices[i]];
        Vec3f v1 = model.verts[model.vertIndices[i + 1]];
        Vec3f v2 = model.verts[model.vertIndices[i + 2]];
        Vec3f c0 = model.colors[i];
        Vec3f c1 = model.colors[i + 1];
        Vec3f c2 = model.colors[i + 2];

        // Back face culling in cam space
        Vec3f surfNormal = Math::Normal(Math::Cross(v2 - v0, v1 - v0));
        if (Math::Dot(v0, surfNormal) > 0.0f)
            continue;

        // Flat shading. @note z-axis isn't inverted here
        Vec3f lightDir = Math::Normal(Vec3f{0.0f, -1.0f, -1.0f});
        // @note Since it survives back face culling, surfNormal should be (+)
        float dp = Math::Dot(lightDir, surfNormal);
        c0 *= -dp;
        c1 *= -dp;
        c2 *= -dp;

        // To clip space. @note z-axis is inverted here to range [0, w];
        std::vector<float> wCoords = {-v0.z, -v1.z, -v2.z};
        v0 = Math::MultiplyVecMat(v0, projMat);
        v1 = Math::MultiplyVecMat(v1, projMat);
        v2 = Math::MultiplyVecMat(v2, projMat);

        // Clipping
        enum Plane
        {
            kNear,
            kTop,
            kRight,
            kBottom,
            kLeft,
            kCount
        };
        std::deque<Triangle> clippedTris;
        // No uv, so set to 0
        clippedTris.push_back(Triangle(v0, v1, v2, Vec2f(0.0f), Vec2f(0.0f), Vec2f(0.0f),
            wCoords[0], wCoords[1], wCoords[2], c0, c1, c2));
        for (int p = Plane::kNear; p != Plane::kCount; ++p)
        {
            size_t oldCnt = clippedTris.size();
            while (oldCnt-- > 0)
            {
                Triangle tri = clippedTris.front();
                clippedTris.pop_front();
                std::vector<Triangle> newTris;
                switch (p)
                {
                case Plane::kNear:
                {
                    newTris = ClipTriangleAgainstPlane({0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.5f}, tri);
                    break;
                }
                case Plane::kTop:
                {
                    newTris = ClipTriangleAgainstPlane({0.0f, -1.0f, 1.0f}, Vec3f(0.0f), tri);
                    break;
                }
                case Plane::kRight:
                {
                    newTris = ClipTriangleAgainstPlane({-1.0f, 0.0f, 1.0f}, Vec3f(0.0f), tri);
                    break;
                }
                case Plane::kBottom:
                {
                    newTris = ClipTriangleAgainstPlane({0.0f, 1.0f, 1.0f}, Vec3f(0.0f), tri);
                    break;
                }
                case Plane::kLeft:
                {
                    newTris = ClipTriangleAgainstPlane({1.0f, 0.0f, 1.0f}, Vec3f(0.0f), tri);
                    break;
                }
                }   // End switch

                for (auto &newTri : newTris)
                    clippedTris.push_back(std::move(newTri));

            }   // End while

        }
    
        for (int j = 0; j < clippedTris.size(); ++j)
        {
            // Perspective divide to NDC space
            v0 = clippedTris[j].verts[0] / clippedTris[j].wCoords[0];
            v1 = clippedTris[j].verts[1] / clippedTris[j].wCoords[1];
            v2 = clippedTris[j].verts[2] / clippedTris[j].wCoords[2];
            c0 = clippedTris[j].colors[0];
            c1 = clippedTris[j].colors[1];
            c2 = clippedTris[j].colors[2];

            // To raster space
            v0.x = (v0.x + 1.0f) * w / 2;
            v0.y = (v0.y + 1.0f) * h / 2;
            v1.x = (v1.x + 1.0f) * w / 2;
            v1.y = (v1.y + 1.0f) * h / 2;
            v2.x = (v2.x + 1.0f) * w / 2;
            v2.y = (v2.y + 1.0f) * h / 2;

            if (mode == QRendererMode::kWireframe)
            {
                DrawLine(pixels, ToColor(255, 255, 255, 255), w, (int)v0.x, (int)v1.x, (int)v0.y, (int)v1.y);
                DrawLine(pixels, ToColor(255, 255, 255, 255), w, (int)v1.x, (int)v2.x, (int)v1.y, (int)v2.y);
                DrawLine(pixels, ToColor(255, 255, 255, 255), w, (int)v2.x, (int)v0.x, (int)v2.y, (int)v0.y);
            }
            else
            {
                float oneOverW0 = 1.0f / clippedTris[j].wCoords[0];
                float oneOverW1 = 1.0f / clippedTris[j].wCoords[1];
                float oneOverW2 = 1.0f / clippedTris[j].wCoords[2];

                float areaOfParallelogram = ComputeEdge(v0, v1, v2);
                if (Helper::IsEqual(areaOfParallelogram, 0.0f))
                    continue;
                int bbMinX = (int)Helper::Min3(v0.x, v1.x, v2.x);
                int bbMaxX = (int)Helper::Max3(v0.x, v1.x, v2.x);
                int bbMinY = (int)Helper::Min3(v0.y, v1.y, v2.y);
                int bbMaxY = (int)Helper::Max3(v0.y, v1.y, v2.y);

                bbMinX = std::max(0, bbMinX);
                bbMinY = std::max(0, bbMinY);
                bbMaxX = std::min(w - 1, bbMaxX);
                bbMaxY = std::min(h - 1, bbMaxY);

                for (int y = bbMinY; y <= bbMaxY; ++y)
                {
                    for (int x = bbMinX; x <= bbMaxX; ++x)
                    {
                        Vec3f pt{(float)x, (float)y, 0.0f};

                        // Inside-outside test
                        float e12 = ComputeEdge(v1, v2, pt);
                        float e20 = ComputeEdge(v2, v0, pt);
                        float e01 = ComputeEdge(v0, v1, pt);
                        if (e01 < 0.0f || e12 < 0.0f || e20 < 0.0f)
                            continue;

                        assert(!Helper::IsEqual(areaOfParallelogram, 0.0f));
                        float t0 = e12 / areaOfParallelogram;
                        float t1 = e20 / areaOfParallelogram;
                        float t2 = e01 / areaOfParallelogram;
                        float oneOverW = t0 * oneOverW0 + t1 * oneOverW1 + t2 * oneOverW2;
                        color = (1.0f / oneOverW) * (c0 * oneOverW0 * t0 + c1 * oneOverW1 * t1 + c2 * oneOverW2 * t2);

                        uint8_t r = ClampChannel(color.r);
                        uint8_t g = ClampChannel(color.g);
                        uint8_t b = ClampChannel(color.b);

                        // @note If z < zBuffer, the triangle is closer, and update new zBuffer.
                        // Instead, since we use oneOverZ, it's actually inverse, and zBuffer filled
                        // with 0 actually represent the furthest (infinitely)
                        if (oneOverW > zBuffer[x + y * w])
                        {
                            if (mode == QRendererMode::kZBuffer)
                            {
                                uint8_t c = ClampChannel(oneOverW);
                                pixels[x + y * w] = ToColor(c, c, c, 255);
                                zBuffer[x + y * w] = oneOverW;
                            }
                            else
                            {
                                pixels[x + y * w] = ToColor(r, g, b, 255);
                                zBuffer[x + y * w] = oneOverW;
                            }
                        }
                    }
                }
            }

        }   // End of insidePts

    }   // End of vertIndices

}

void Rasterizer::Rasterize(uint32_t *pixels, float *zBuffer, QTexture *texture, int w, int h, const Model& model, const Mat44f& projMat, QRendererMode mode)
{
    texture->LockTexture();
    assert(!model.verts.empty() && "Uh oh, model is empty!");
    assert(texture && "Uh oh, texture is empty!");

    for (int i = 0; i < model.vertIndices.size(); i += 3)
    {
        Vec3f v0 = model.verts[model.vertIndices[i]];
        Vec3f v1 = model.verts[model.vertIndices[i + 1]];
        Vec3f v2 = model.verts[model.vertIndices[i + 2]];

        Vec2f uv0 = model.texCoords[model.uvIndices[i]];
        Vec2f uv1 = model.texCoords[model.uvIndices[i + 1]];
        Vec2f uv2 = model.texCoords[model.uvIndices[i + 2]];

        // Back face culling in cam space
        Vec3f surfNormal = Math::Normal(Math::Cross(v2 - v0, v1 - v0));
        if (Math::Dot(v0, surfNormal) > 0.0f)
            continue;

        // Flat shading. @note z-axis isn't inverted until perspective divide! 
        Vec3f lightDir = Math::Normal(Vec3f{0.0f, -1.0f, -1.0f});
        // @note Since it survives back face culling, surfNormal should be (+)
        float dp = Math::Dot(lightDir, surfNormal);
        color = Vec3f{1.0f, 1.0f, 1.0f} * -dp;

        // To clip space
        std::vector<float> wCoords = {-v0.z, -v1.z, -v2.z};
        v0 = Math::MultiplyVecMat(v0, projMat);
        v1 = Math::MultiplyVecMat(v1, projMat);
        v2 = Math::MultiplyVecMat(v2, projMat);

        // Clipping
        enum Plane
        {
            kNear,
            kTop,
            kRight,
            kBottom,
            kLeft,
            kCount
        };
        std::deque<Triangle> clippedTris;
        clippedTris.push_back(Triangle(v0, v1, v2, uv0, uv1, uv2, wCoords[0], wCoords[1], wCoords[2]));
        for (int p = Plane::kNear; p != Plane::kCount; ++p)
        {
            size_t oldCnt = clippedTris.size();
            while (oldCnt-- > 0)
            {
                Triangle tri = clippedTris.front();
                clippedTris.pop_front();
                std::vector<Triangle> newTris;
                switch (p)
                {
                case Plane::kNear:
                {
                    newTris = ClipTriangleAgainstPlane({0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.5f}, tri);
                    break;
                }
                case Plane::kTop:
                {
                    newTris = ClipTriangleAgainstPlane({0.0f, -1.0f, 1.0f}, Vec3f(0.0f), tri);
                    break;
                }
                case Plane::kRight:
                {
                    newTris = ClipTriangleAgainstPlane({-1.0f, 0.0f, 1.0f}, Vec3f(0.0f), tri);
                    break;
                }
                case Plane::kBottom:
                {
                    newTris = ClipTriangleAgainstPlane({0.0f, 1.0f, 1.0f}, Vec3f(0.0f), tri);
                    break;
                }
                case Plane::kLeft:
                {
                    newTris = ClipTriangleAgainstPlane({1.0f, 0.0f, 1.0f}, Vec3f(0.0f), tri);
                    break;
                }
                }   // End switch

                for (auto &newTri : newTris)
                    clippedTris.push_back(std::move(newTri));

            }   // End while

        }
    
        for (int j = 0; j < clippedTris.size(); ++j)
        {
            // Perspective divide to NDC space
            v0 = clippedTris[j].verts[0] / clippedTris[j].wCoords[0];
            v1 = clippedTris[j].verts[1] / clippedTris[j].wCoords[1];
            v2 = clippedTris[j].verts[2] / clippedTris[j].wCoords[2];

            // @note Texture wrap?
#if 0
            uv0 = Vec2f{fmod(clippedTris[j].texCoords[0].e[0], 1.0f), fmod(clippedTris[j].texCoords[0].e[1], 1.0f)};
            uv1 = Vec2f{fmod(clippedTris[j].texCoords[1].e[0], 1.0f), fmod(clippedTris[j].texCoords[1].e[1], 1.0f)};
            uv2 = Vec2f{fmod(clippedTris[j].texCoords[2].e[0], 1.0f), fmod(clippedTris[j].texCoords[2].e[1], 1.0f)};
#endif
            uv0 = clippedTris[j].texCoords[0];
            uv1 = clippedTris[j].texCoords[1];
            uv2 = clippedTris[j].texCoords[2];

            // To raster space
            v0.x = (v0.x + 1.0f) * w / 2;
            v0.y = (v0.y + 1.0f) * h / 2;
            v1.x = (v1.x + 1.0f) * w / 2;
            v1.y = (v1.y + 1.0f) * h / 2;
            v2.x = (v2.x + 1.0f) * w / 2;
            v2.y = (v2.y + 1.0f) * h / 2;

            if (mode == QRendererMode::kWireframe)
            {
                DrawLine(pixels, ToColor(255, 255, 255, 255), w, (int)v0.x, (int)v1.x, (int)v0.y, (int)v1.y);
                DrawLine(pixels, ToColor(255, 255, 255, 255), w, (int)v1.x, (int)v2.x, (int)v1.y, (int)v2.y);
                DrawLine(pixels, ToColor(255, 255, 255, 255), w, (int)v2.x, (int)v0.x, (int)v2.y, (int)v0.y);
            }
            else
            {
                float oneOverW0 = 1.0f / clippedTris[j].wCoords[0];
                float oneOverW1 = 1.0f / clippedTris[j].wCoords[1];
                float oneOverW2 = 1.0f / clippedTris[j].wCoords[2];

                float areaOfParallelogram = ComputeEdge(v0, v1, v2);
                if (Helper::IsEqual(areaOfParallelogram, 0.0f))
                    continue;
                int bbMinX = (int)Helper::Min3(v0.x, v1.x, v2.x);
                int bbMaxX = (int)Helper::Max3(v0.x, v1.x, v2.x);
                int bbMinY = (int)Helper::Min3(v0.y, v1.y, v2.y);
                int bbMaxY = (int)Helper::Max3(v0.y, v1.y, v2.y);

                bbMinX = std::max(0, bbMinX);
                bbMinY = std::max(0, bbMinY);
                bbMaxX = std::min(w - 1, bbMaxX);
                bbMaxY = std::min(h - 1, bbMaxY);

                for (int y = bbMinY; y <= bbMaxY; ++y)
                {
                    for (int x = bbMinX; x <= bbMaxX; ++x)
                    {
                        Vec3f pt{(float)x, (float)y, 0.0f};

                        // Inside-outside test
                        float e12 = ComputeEdge(v1, v2, pt);
                        float e20 = ComputeEdge(v2, v0, pt);
                        float e01 = ComputeEdge(v0, v1, pt);
                        if (e01 < 0.0f || e12 < 0.0f || e20 < 0.0f)
                            continue;

                        assert(!Helper::IsEqual(areaOfParallelogram, 0.0f));
                        float t0 = e12 / areaOfParallelogram;
                        float t1 = e20 / areaOfParallelogram;
                        float t2 = e01 / areaOfParallelogram;
                        float oneOverW = t0 * oneOverW0 + t1 * oneOverW1 + t2 * oneOverW2;
                        Vec2f uv = (1.0f / oneOverW) * (uv0 * oneOverW0 * t0 + uv1 * oneOverW1 * t1 + uv2 * oneOverW2 * t2);

                        int uvX = std::min(texture->GetW() - 1, (int)(uv.x * texture->GetW() + 0.5f));
                        int uvY = std::min(texture->GetH() - 1, (int)(uv.y * texture->GetH() + 0.5f));
                        uint32_t myColor = texture->GetTexels()[uvX + uvY * texture->GetW()];
                        // @note If z < zBuffer, the triangle is closer, and update new zBuffer.
                        // Instead, since we use oneOverZ, it's actually inverse, and zBuffer filled
                        // with 0 actually represent the furthest (infinitely)
                        if (oneOverW > zBuffer[x + y * w])
                        {
                            if (mode == QRendererMode::kZBuffer)
                            {
                                uint8_t c = ClampChannel(oneOverW);
                                pixels[x + y * w] = ToColor(c, c, c, 255);
                                zBuffer[x + y * w] = oneOverW;
                            }
                            else
                            {
                                pixels[x + y * w] = myColor;
                                zBuffer[x + y * w] = oneOverW;
                            }
                        }
                    }
                }
            }

        }   // End of insidePts

    }   // End of vertIndices

    texture->UnlockTexture();
}


uint32_t Rasterizer::ToColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    uint32_t color = (a << 24) | (b << 16) | (g << 8) | r;
    return color;
}

uint8_t Rasterizer::ClampChannel(float channel)
{
    return (uint8_t)std::max(0.0f, std::min(1.0f, channel) * 255.0f + 0.5f);
}

void Rasterizer::DrawLine(uint32_t* pixels, uint32_t color, int w, int x0, int x1, int y0, int y1)
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
        if (isSteep) { pixels[y + x * w] = color; }
        else { pixels[x + y * w] = color; }
        e2 += 2 * dy;
        if (e2 > 0)
        {
            e2 -= 2 * dx;
            y += yDir;
        }
    }

}

void Rasterizer::TestDrawLine(uint32_t * pixels, int scrW, int scrH)
{
    constexpr float pi = 3.141592653589f;
    constexpr float angle = pi / 6;
    float rotMat[4] = {cos(angle), sin(angle), -sin(angle), cos(angle)};

    uint32_t red = ToColor(255, 0, 0, 255);
    uint32_t green = ToColor(0, 255, 0, 255);

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
            DrawLine(pixels, red, scrW, x0, x1, y0, y1);
        }
        else
        {
            DrawLine(pixels, green, scrW, x0, x1, y0, y1);
        }

        endP[0] = newEndP[0];
        endP[1] = newEndP[1];
    }

}

float Rasterizer::ComputeEdge(const Vec3f& a, const Vec3f& b, const Vec3f& c)
{
    float result = (c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x);
    return result;
}

std::vector<Triangle> Rasterizer::ClipTriangleAgainstPlane(Vec3f planeN, Vec3f planePt, const Triangle& inTri)
{
    planeN = Math::Normal(planeN);
    std::vector<Triangle> result;

    std::vector<Point> insidePts;
    std::vector<Point> outsidePts;
    insidePts.reserve(6);     // At most 2 new tris are created
    outsidePts.reserve(3);    // At most the whole tri is clipped

    float planeD = Math::Dot(planeN, planePt);
    float d0 = Math::Dot(planeN, inTri.verts[0]);
    float d1 = Math::Dot(planeN, inTri.verts[1]);
    float d2 = Math::Dot(planeN, inTri.verts[2]);
    if (d0 >= planeD)
        insidePts.emplace_back(inTri.verts[0], inTri.texCoords[0], inTri.wCoords[0], inTri.colors[0]);
    else
        outsidePts.emplace_back(inTri.verts[0], inTri.texCoords[0], inTri.wCoords[0], inTri.colors[0]);

    if (d1 >= planeD)
        insidePts.emplace_back(inTri.verts[1], inTri.texCoords[1], inTri.wCoords[1], inTri.colors[1]);
    else
        outsidePts.emplace_back(inTri.verts[1], inTri.texCoords[1], inTri.wCoords[1], inTri.colors[1]);

    if (d2 >= planeD)
        insidePts.emplace_back(inTri.verts[2], inTri.texCoords[2], inTri.wCoords[2], inTri.colors[2]);
    else
        outsidePts.emplace_back(inTri.verts[2], inTri.texCoords[2], inTri.wCoords[2], inTri.colors[2]);

    // Triangle is outside
    if (insidePts.size() == 0)
        return result;
    if (insidePts.size() == 3)
    {
        result.push_back(inTri);
        return result;
    }

    if (insidePts.size() == 1 && outsidePts.size() == 2)
    {
        color = Vec3f{1.0f, 0.0f, 0.0f};

        float tb, ta;
        // Assuming v0 v1 v2, outsidePt = v0 v1 are clipped, insidePt = v2, a is intersection in
        // ray v0v2, b is intersection in ray v1v2
        Vec3f a = IntersectRayPlane(outsidePts[0].pos, insidePts[0].pos, planeD, planeN, &ta);  // v0v2
        Vec3f b = IntersectRayPlane(outsidePts[1].pos, insidePts[0].pos, planeD, planeN, &tb);  // v1v2

        float aW = Helper::Interpolate<float, float>(outsidePts[0].wCoord, insidePts[0].wCoord, ta);
        float bW = Helper::Interpolate<float, float>(outsidePts[1].wCoord, insidePts[0].wCoord, tb);

        Vec2f aTex = Helper::Interpolate<Vec2f, float>(outsidePts[0].texCoord, insidePts[0].texCoord, ta);
        Vec2f bTex = Helper::Interpolate<Vec2f, float>(outsidePts[1].texCoord, insidePts[0].texCoord, tb);

        Vec3f aColor = Helper::Interpolate<Vec3f, float>(outsidePts[0].color, insidePts[0].color, ta);
        Vec3f bColor = Helper::Interpolate<Vec3f, float>(outsidePts[1].color, insidePts[0].color, tb);
        
        // New tri is v2ab
        Vec3f n = Math::Cross(b - insidePts[0].pos, a - insidePts[0].pos);
        if (Math::Dot(insidePts[0].pos, n) <= 0.0f)
        {
            result.push_back(Triangle{insidePts[0].pos, b, a,
                insidePts[0].texCoord, bTex, aTex,
                insidePts[0].wCoord, bW, aW,
                insidePts[0].color, bColor, aColor});
        }
        else
        {
            result.push_back(Triangle{insidePts[0].pos, a, b,
                insidePts[0].texCoord, aTex, bTex,
                insidePts[0].wCoord, aW, bW,
                insidePts[0].color, aColor, bColor});
        }
    }

    if (outsidePts.size() == 1 && insidePts.size() == 2)
    {
        color = Vec3f{0.0f, 1.0f, 0.0f};

        // Assuming v0 v1 v2, outsidePt = v2 is clipped, insidePt = v0 v1, a is intersection in ray
        // v2v0, b is intersection in ray v2v1
        float ta, tb;
        Vec3f a = IntersectRayPlane(outsidePts[0].pos, insidePts[0].pos, planeD, planeN, &ta);  // v2v0
        Vec3f b = IntersectRayPlane(outsidePts[0].pos, insidePts[1].pos, planeD, planeN, &tb);  // v2v1

        float aW = Helper::Interpolate<float, float>(outsidePts[0].wCoord, insidePts[0].wCoord, ta);
        float bW = Helper::Interpolate<float, float>(outsidePts[0].wCoord, insidePts[1].wCoord, tb);

        Vec2f aTex = Helper::Interpolate<Vec2f, float>(outsidePts[0].texCoord, insidePts[0].texCoord, ta);
        Vec2f bTex = Helper::Interpolate<Vec2f, float>(outsidePts[0].texCoord, insidePts[1].texCoord, tb);

        Vec3f aColor = Helper::Interpolate<Vec3f, float>(outsidePts[0].color, insidePts[0].color, ta);
        Vec3f bColor = Helper::Interpolate<Vec3f, float>(outsidePts[0].color, insidePts[1].color, tb);

        Vec3f n = Math::Cross(insidePts[1].pos - insidePts[0].pos, a - insidePts[0].pos);
        if (Math::Dot(insidePts[0].pos, n) <= 0.0f)
        {
            result.push_back(Triangle{insidePts[0].pos, insidePts[1].pos, a,
                insidePts[0].texCoord, insidePts[1].texCoord, aTex,
                insidePts[0].wCoord, insidePts[1].wCoord, aW,
                insidePts[0].color, insidePts[1].color, aColor});
            result.push_back(Triangle{insidePts[1].pos, b, a,
                insidePts[1].texCoord, bTex, aTex,
                insidePts[1].wCoord, bW, aW,
                insidePts[1].color, bColor, aColor});
        }
        else
        {
            result.push_back(Triangle{insidePts[0].pos, a, insidePts[1].pos,
                insidePts[0].texCoord, aTex, insidePts[1].texCoord,
                insidePts[0].wCoord, aW, insidePts[1].wCoord,
                insidePts[0].color, aColor, insidePts[1].color});
            result.push_back(Triangle{insidePts[1].pos, a, b,
                insidePts[1].texCoord, aTex, bTex,
                insidePts[1].wCoord, aW, bW,
                insidePts[1].color, aColor, bColor});
        }
   }

    // 1 new tri or 2 new tri
    assert(result.size() == 1 || result.size() == 2);
    return result;
}

Vec3f Rasterizer::IntersectRayPlane(const Vec3f& p0, const Vec3f& p1, float planeD, const Vec3f& planeN, float *outT)
{
    Vec3f lineDir = p1 - p0;
    float t = (planeD - Math::Dot(p0, planeN)) / Math::Dot(lineDir, planeN);
    if (outT)
        *outT = t;
    Vec3f intersectPt = p0 + t * lineDir;
    return intersectPt;
}
    
