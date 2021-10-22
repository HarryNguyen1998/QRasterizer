#include <algorithm>
#include <cassert>
#include <deque>
#include <iostream>

#include "Renderer/IndexModel.h"
#include "Renderer/Rasterizer.h"
#include "Renderer/Triangle.h"

void Rasterizer::Rasterize(uint32_t *pixels, float *zBuffer, int w, int h, const Model& model, const Mat44f& projMat, const std::vector<Vec3f>& colors)
{
    bool shouldDrawDepth = false;
    assert(!model.verts.empty());
    assert(!model.vertIndices.empty());

    for (int i = 0; i < model.vertIndices.size(); i += 3)
    {
        Vec3f v0 = model.verts[model.vertIndices[i]];
        Vec3f v1 = model.verts[model.vertIndices[i + 1]];
        Vec3f v2 = model.verts[model.vertIndices[i + 2]];

        // Back face culling in cam space
        Vec3f surfNormal = Math::Normal(Math::Cross(v2 - v0, v1 - v0));
        if (Math::Dot(v0, surfNormal) >= 0.0f)
            continue;

        // Flat shading. @note z-axis isn't inverted until perspective divide! 
        Vec3f lightDir = Math::Normal(Vec3f{0.0f, -1.0f, -1.0f});
        // @note Since it survives back face culling, surfNormal should be (+)
        float dp = Math::Dot(lightDir, surfNormal);
        Vec3f color = Vec3f{1.0f, 1.0f, 1.0f} * -dp;

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
        clippedTris.push_back(Triangle(v0, v1, v2, wCoords[0], wCoords[1], wCoords[2]));
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
                    newTris = ClipTriangleAgainstPlane({0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 2.0f}, tri);
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

            // To raster space
            v0.x = (v0.x + 1.0f) * w / 2;
            v0.y = (v0.y + 1.0f) * h / 2;
            v1.x = (v1.x + 1.0f) * w / 2;
            v1.y = (v1.y + 1.0f) * h / 2;
            v2.x = (v2.x + 1.0f) * w / 2;
            v2.y = (v2.y + 1.0f) * h / 2;
            assert(v0.z <= 1.0f && v0.z >= 0.0f &&
                v1.z <= 1.0f && v1.z >= 0.0f &&
                v2.z <= 1.0f && v2.z >= 0.0f);
            float oneOverW0 = 1.0f / clippedTris[j].wCoords[0];
            float oneOverW1 = 1.0f / clippedTris[j].wCoords[1];
            float oneOverW2 = 1.0f / clippedTris[j].wCoords[2];

            float areaOfParallelogram = ComputeEdge(v0, v1, v2);
            int bbMinX = (int)Helper::Min3(v0.x, v1.x, v2.x);
            int bbMaxX = (int)Helper::Max3(v0.x, v1.x, v2.x);
            int bbMinY = (int)Helper::Min3(v0.y, v1.y, v2.y);
            int bbMaxY = (int)Helper::Max3(v0.y, v1.y, v2.y);

            // @note If 2 verts are nearly the same x/y coord, bbX/bbY will be outside of screen
            // boundary. I don't know why does this happen, since I thought clipping should have
            // eliminated that. Maybe some bug? Anyways, this seems to fix it
            if (bbMinX > w - 1 || bbMinY > h - 1 || bbMaxX < 0 || bbMaxY < 0)
                continue;
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
                    if (e01 < 0.0f || e12 < 0.0f || e20 < 0.0f) { continue; }

                    float t0 = e12 / areaOfParallelogram;
                    float t1 = e20 / areaOfParallelogram;
                    float t2 = e01 / areaOfParallelogram;
                    float oneOverW = t0 * oneOverW0 + t1 * oneOverW1 + t2 * oneOverW2;

#if 0
                    uint8_t r = ClampChannel(colors[j].r * t0 + colors[j + 1].r * t1 + colors[j + 2].r * t2);
                    uint8_t g = ClampChannel(colors[j].g * t0 + colors[j + 1].g * t1 + colors[j + 2].g * t2);
                    uint8_t b = ClampChannel(colors[j].b * t0 + colors[j + 1].b * t1 + colors[j + 2].b * t2);
#endif
                    uint8_t r = ClampChannel(color.r);
                    uint8_t g = ClampChannel(color.g);
                    uint8_t b = ClampChannel(color.b);

                    // @note If z < zBuffer, the triangle is closer, and update new zBuffer.
                    // Instead, since we use oneOverZ, it's actually inverse, and zBuffer filled
                    // with 0 actually represent the furthest (infinitely)
                    if (oneOverW > zBuffer[x + y * w])
                    {
                        if (shouldDrawDepth)
                        {
                            zBuffer[x + y * w] = oneOverW;
                            uint8_t c = ClampChannel(oneOverW);
                            pixels[x + y * w] = ToColor(c, c, c, 255);
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

    std::vector<Vec3f> insidePts;
    insidePts.reserve(6);  // At most all 2 tris are formed
    std::vector<Vec3f> outsidePts;
    outsidePts.reserve(3);  // At most all 3 pts are outside

    std::vector<float> insidePtW;
    insidePtW.reserve(6);  // At most all 2 tris are formed
    std::vector<float> outsidePtW;
    outsidePtW.reserve(3);  // At most all 3 pts are outside

    float planeD = Math::Dot(planeN, planePt);
    float d0 = Math::Dot(planeN, inTri.verts[0]);
    float d1 = Math::Dot(planeN, inTri.verts[1]);
    float d2 = Math::Dot(planeN, inTri.verts[2]);
    if (d0 >= planeD)
    {
        insidePts.push_back(inTri.verts[0]);
        insidePtW.push_back(inTri.wCoords[0]);
    }
    else
    {
        outsidePts.push_back(inTri.verts[0]);
        outsidePtW.push_back(inTri.wCoords[0]);
    }

    if (d1 >= planeD)
    {
        insidePts.push_back(inTri.verts[1]);
        insidePtW.push_back(inTri.wCoords[1]);
    }
    else
    {
        outsidePts.push_back(inTri.verts[1]);
        outsidePtW.push_back(inTri.wCoords[1]);
    }
    if (d2 >= planeD)
    {
        insidePts.push_back(inTri.verts[2]);
        insidePtW.push_back(inTri.wCoords[2]);
    }
    else
    {
        outsidePts.push_back(inTri.verts[2]);
        outsidePtW.push_back(inTri.wCoords[2]);
    }

    // Triangle is outside
    if (insidePts.size() == 0)
        return result;
    // Triangle is inside
    if (insidePts.size() == 3)
    {
        result.push_back(inTri);
        return result;
    }

    // Swap the pts
    if (insidePts.size() == 1 && outsidePts.size() == 2)
    {
        if (ComputeEdge(Vec3f(0.0f), outsidePts[1], outsidePts[0]) < 0.0f)
        {
            Vec3f tmp = outsidePts[1];
            outsidePts[1] = outsidePts[0];
            outsidePts[0] = tmp;
        }

        float tb, ta;
        // Assuming v0 v1 v2, outsidePt = v0 v1 are clipped, insidePt = v2, a is intersection in
        // ray v0v2, b is intersection in ray v1v2
        Vec3f a = IntersectRayPlane(outsidePts[0], insidePts[0], planeD, planeN, &ta);  // v0v2
        Vec3f b = IntersectRayPlane(outsidePts[1], insidePts[0], planeD, planeN, &tb);  // v1v2

        float lineDir1 = insidePtW[0] - outsidePtW[0];
        float aW = outsidePtW[0] + ta * lineDir1;

        float lineDir2 = insidePtW[0] - outsidePtW[1];
        float bW = outsidePtW[1] + tb * lineDir2;

        // New tri is v2ab
        result.push_back(Triangle{insidePts[0], b, a, insidePtW[0], bW, aW});
    }

    if (outsidePts.size() == 1 && insidePts.size() == 2)
    {
        if (ComputeEdge(Vec3f(0.0f), insidePts[1], insidePts[0]) < 0.0f)
        {
            Vec3f tmp = insidePts[1];
            insidePts[1] = insidePts[0];
            insidePts[0] = tmp;
        }

        // Assuming v0 v1 v2, outsidePt = v2 is clipped, insidePt = v0 v1, a is intersection in ray
        // v2v0, b is intersection in ray v2v1
        float ta, tb;
        Vec3f a = IntersectRayPlane(outsidePts[0], insidePts[0], planeD, planeN, &ta);  // v2v0
        Vec3f b = IntersectRayPlane(outsidePts[0], insidePts[1], planeD, planeN, &tb);  // v2v1

        float lineDir1 = insidePtW[0] - outsidePtW[0];
        float aW = outsidePtW[0] + ta * lineDir1;

        float lineDir2 = insidePtW[1] - outsidePtW[0];
        float bW = outsidePtW[0] + tb * lineDir2;

        // New tris are v0v1b, v0ba
        result.push_back(Triangle{insidePts[0], insidePts[1], b, insidePtW[0], insidePtW[1], bW});
        result.push_back(Triangle{insidePts[0], b, a, insidePtW[0], bW, aW});
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
    
