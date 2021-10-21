#include <algorithm>
#include <cassert>
#include <deque>
#include <iostream>

#include "Renderer/IndexModel.h"
#include "Renderer/Rasterizer.h"
#include "Renderer/Triangle.h"

Vec3f color;

void Rasterizer::Rasterize(uint32_t *pixels, int w, int h, const Model& model, const Mat44f& projMat, const std::vector<Vec3f>& colors)
{
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

            float areaOfParallelogram = ComputeEdge(v0, v1, v2);
            int bbMinX = (int)Helper::Min3(v0.x, v1.x, v2.x);
            int bbMaxX = (int)Helper::Max3(v0.x, v1.x, v2.x);
            int bbMinY = (int)Helper::Min3(v0.y, v1.y, v2.y);
            int bbMaxY = (int)Helper::Max3(v0.y, v1.y, v2.y);

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

#if 0
                    uint8_t r = ClampChannel(colors[j].r * t0 + colors[j + 1].r * t1 + colors[j + 2].r * t2);
                    uint8_t g = ClampChannel(colors[j].g * t0 + colors[j + 1].g * t1 + colors[j + 2].g * t2);
                    uint8_t b = ClampChannel(colors[j].b * t0 + colors[j + 1].b * t1 + colors[j + 2].b * t2);
#endif
                    uint8_t r = ClampChannel(color.r);
                    uint8_t g = ClampChannel(color.g);
                    uint8_t b = ClampChannel(color.b);

                    pixels[x + y * w] = ToColor(r, g, b, 255);
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

    std::vector<float> insidePtAttrs;
    insidePtAttrs.reserve(6);  // At most all 2 tris are formed
    std::vector<float> outsidePtAttrs;
    outsidePtAttrs.reserve(3);  // At most all 3 pts are outside

    float planeD = Math::Dot(planeN, planePt);
    float d0 = Math::Dot(planeN, inTri.verts[0]);
    float d1 = Math::Dot(planeN, inTri.verts[1]);
    float d2 = Math::Dot(planeN, inTri.verts[2]);
    if (d0 >= planeD)
    {
        insidePts.push_back(inTri.verts[0]);
        insidePtAttrs.push_back(inTri.wCoords[0]);
    }
    else
    {
        outsidePts.push_back(inTri.verts[0]);
        outsidePtAttrs.push_back(inTri.wCoords[0]);
    }

    if (d1 >= planeD)
    {
        insidePts.push_back(inTri.verts[1]);
        insidePtAttrs.push_back(inTri.wCoords[1]);
    }
    else
    {
        outsidePts.push_back(inTri.verts[1]);
        outsidePtAttrs.push_back(inTri.wCoords[1]);
    }
    if (d2 >= planeD)
    {
        insidePts.push_back(inTri.verts[2]);
        insidePtAttrs.push_back(inTri.wCoords[2]);
    }
    else
    {
        outsidePts.push_back(inTri.verts[2]);
        outsidePtAttrs.push_back(inTri.wCoords[2]);
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

        color = Vec3f{1.0f, 0.0f, 0.0f};

        float tb, ta;
        // Assuming v0 v1 v2 and v0, v2 are clipped, then new tri is v1 b a, where pt a is
        // intersection in ray v0v1, pt b is instersection in ray v2v1,
        insidePts.push_back(IntersectRayPlane(outsidePts[1], insidePts[0], planeD, planeN, &tb));   // v2v1
        insidePts.push_back(IntersectRayPlane(outsidePts[0], insidePts[0], planeD, planeN, &ta));   // v0v1

        float v1Attribute = insidePtAttrs[0];

        float lineDir1 = insidePtAttrs[0] - outsidePtAttrs[0];
        float aAttribute = outsidePtAttrs[0] + ta * lineDir1;

        float lineDir2 = insidePtAttrs[0] - outsidePtAttrs[1];
        float bAttribute = outsidePtAttrs[1] + tb * lineDir2;

        result.push_back(Triangle{insidePts[0], insidePts[1], insidePts[2], v1Attribute, bAttribute, aAttribute});
    }

    if (outsidePts.size() == 1 && insidePts.size() == 2)
    {
        if (ComputeEdge(Vec3f(0.0f), insidePts[1], insidePts[0]) < 0.0f)
        {
            Vec3f tmp = insidePts[1];
            insidePts[1] = insidePts[0];
            insidePts[0] = tmp;
        }

        color = Vec3f{0.0f, 1.0f, 0.0f};

        // Assuming v0 v1 v2, outsidePts = v0 is clipped, insidePts are v1 v2, a is intersection in
        // ray v0v1, b is intersection in ray v0v2
        float ta, tb;
        Vec3f a = IntersectRayPlane(outsidePts[0], insidePts[0], planeD, planeN, &ta);  // v0v1
        Vec3f b = IntersectRayPlane(outsidePts[0], insidePts[1], planeD, planeN, &tb);  // v0v2

        float v1Attribute = insidePtAttrs[0];
        float v2Attribute = insidePtAttrs[1];

        float lineDir1 = insidePtAttrs[0] - outsidePtAttrs[0];
        float aAttribute = outsidePtAttrs[0] + ta * lineDir1;

        float lineDir2 = insidePtAttrs[1] - outsidePtAttrs[0];
        float bAttribute = outsidePtAttrs[0] + tb * lineDir2;

        // Push v2av1, and v2ba
        result.push_back(Triangle{insidePts[0], insidePts[1], b, v1Attribute, v2Attribute, bAttribute});
        result.push_back(Triangle{insidePts[0], b, a, v1Attribute, bAttribute, aAttribute});
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
    
