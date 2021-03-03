#include "Math/Vector.h"
#include "Math/Matrix.h"
#include "Vertices.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using std::ofstream;

// The mode doesn't change anything if device gate and film gate resolution are the same
enum class ResolutionGateMode
{
	kFill,		// Fit resolution gate into film gate (scale down)
	kOverscan,	// Fit film gate into resolution gate (scale up)
};
ResolutionGateMode g_resolutionGateMode = ResolutionGateMode::kOverscan;

// Based on pinhole cam model
void GetCanvasCoord(float focalLength, float zNear, ResolutionGateMode mode,
    float filmGateW, float filmGateH,
    int imgW, int imgH,
    float* canvasB, float* canvasL, float* canvasT, float* canvasR)
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

Mat44f GetProjetionMatrix(float t, float r, float l, float b,
    float n, float f)
{
    Mat44f result{};
    result(0, 0) = (2 * n) / (r - l);
    result(1, 1) = (2 * n) / (t - b);
    result(2, 0) = (r + l) / (r - l);
    result(2, 1) = (t + b) / (t - b);
    result(2, 2) = -(f + n) / (f - n);
    result(2, 3) = -1.0f;
    result(3, 2) = -(2 * f * n) / (f - n);

    return result;
}

void ConvertToRaster(const Vec3f pWorld, Vec3f* pRaster, const Mat44f &worldToCamera,
	float b, float l, float t, float r,
	unsigned imageW, unsigned imageH, float zNear, float zFar)
{
    Vec3f pCamera = MultiplyVecMat(pWorld, worldToCamera);
    Mat44f projMat = GetProjetionMatrix(t, r, l, b, zNear, zFar);
    Vec3f pNDC = MultiplyPtMat(pCamera, projMat);
	// From NDC space to raster space.
	pRaster->x = (pNDC.x + 1.0f) / 2 * imageW;
	pRaster->y = (1.0f - pNDC.y) / 2 * imageH;
    pRaster->z = -pCamera.z;
}

// (c-a) Cross (b-a)
inline float EdgeFunction(const Vec3f& a, const Vec3f& b, const Vec3f& c)
{
    float result = (c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x);
    return result;
}

// Pls remember not to pass a vector with z-value of 0.
inline float ComputeDepth(const Vec3f& v0, const Vec3f& v1, const Vec3f& v2, float w0, float w1, float w2)
{
    float depthInverse = (w0 / v0.z) + (w1 / v1.z) + (w2 / v2.z);
    float depth = (1 / depthInverse);
    return depth;
}

// Func implementation is based on when dx and dy are (+), and the line is gentle (dx > dy). To account for other
// case, swap x and y when passing in arguments
void DrawLine2Axis(Math::Vector<unsigned char, 3>* pixels, const Math::Vector<unsigned char, 3>& color,
    int xStep, int yStep,
    int dx, int dy)
{
    // Draw starting pt
    *pixels = color;
    int dTimes2 = 2 * dy - dx;

    // Not --dx because dx could be 0, e.g., a point!!
    while (dx--)
    {
        if (dTimes2 > 0)
        {
            pixels += xStep + yStep;
            dTimes2 += 2 * (dy - dx);
        }
        else
        {
            pixels += xStep;
            dTimes2 += 2 * dy;
        }

        *pixels = color;
    }
}

// Func implementation is based on when drawing a horizontal line. To account for other case, swap x and y when
// passing in arguments
void DrawLineSingleAxis(Math::Vector<unsigned char, 3>* pixels, const Math::Vector<unsigned char, 3>& color,
    int xStep, int dx)
{
    // Draw starting pt
    *pixels = color;
    while (dx--)
    {
        pixels += xStep;
        *pixels = color;
    }
}

// Resource: https://blog.demofox.org/2015/01/17/bresenhams-drawing-algorithms/
void DrawLine(Math::Vector<unsigned char, 3>* pixels, const Math::Vector<unsigned char, 3>& color, int pixelStride,
    int x0, int x1, int y0, int y1)
{
    int dx = x1 - x0;
    int dy = y1 - y0;
    // Steep/high line, y is major axis
    if (std::abs(dy) > std::abs(dx))    
    {
        if (dy < 0)
        {
            dy = -dy;
            dx = -dx;
            std::swap(x0, x1);
            std::swap(y0, y1);
        }
        auto* startPixels = &pixels[y0 * pixelStride + x0];

        if (dx > 0) { DrawLine2Axis(startPixels, color, pixelStride, 1, dy, dx); }
        else if (dx < 0) { DrawLine2Axis(startPixels, color, pixelStride, -1, dy, -dx); }
        else { DrawLineSingleAxis(startPixels, color, pixelStride, dy); }
    }

    // Gentle line, x is major axis
    else 
    {
        if (dx < 0)
        {
            dx = -dx;
            dy = -dy;
            std::swap(x0, x1);
            std::swap(y0, y1);
        }
        auto* startPixels = &pixels[y0 * pixelStride + x0];

        if (dy > 0) { DrawLine2Axis(startPixels, color, 1, pixelStride, dx, dy); }
        else if (dy < 0) { DrawLine2Axis(startPixels, color, 1, -pixelStride, dx, -dy); }
        else { DrawLineSingleAxis(startPixels, color, 1, dx); }
    }
        
}

// Pinhole camera settings
float g_focalLength = 35.0f;	// mm
float g_filmGateW = 0.885f;     // inch
float g_filmGateH = 0.885f;

float g_zNear = 1.0f;
float g_zFar = 1000.0f;

// Img resolution in pixels
const unsigned g_imgW = 640;
const unsigned g_imgH = 480;
const Mat44f g_worldToCam = {0.707107f, -0.331295f, 0.624695f, 0.0f,
    0.0f, 0.883452f, 0.468521f, 0.0f,
    -0.707107f, -0.331295f, 0.624695f, 0.0f,
    -1.63871f, -5.747777f, -40.400412f, 1.0f};


int main()
{
    float canvasB, canvasL, canvasT, canvasR;
    GetCanvasCoord(g_focalLength, g_zNear, g_resolutionGateMode,
        g_filmGateW, g_filmGateH, g_imgW, g_imgH,
        &canvasB, &canvasL, &canvasT, &canvasR);

    std::vector<Math::Vector<unsigned char, 3>> frameBuffer(g_imgW * g_imgH, Math::Vector<unsigned char, 3>{50, 50, 50});
    std::vector<float> depthBuffer(g_imgW * g_imgH, FLT_MAX);
    
    Helper::Timer t;
    t.SetManual(true);
    t.Start();

    for (int i = 0; i < g_numTris; ++i)
    {
        const Vec3f& v0 = g_verts[g_trisIndices[i * 3]];
        const Vec3f& v1 = g_verts[g_trisIndices[i * 3 + 1]];
        const Vec3f& v2 = g_verts[g_trisIndices[i * 3 + 2]];

        const Vec2f& st0 = g_st[g_stIndices[i * 3]];
        const Vec2f& st1 = g_st[g_stIndices[i * 3 + 1]];
        const Vec2f& st2 = g_st[g_stIndices[i * 3 + 2]];

        Vec3f v0Raster, v1Raster, v2Raster;
        ConvertToRaster(v0, &v0Raster, g_worldToCam, canvasB, canvasL, canvasT, canvasR, g_imgW, g_imgH, g_zNear, g_zFar);
        ConvertToRaster(v1, &v1Raster, g_worldToCam, canvasB, canvasL, canvasT, canvasR, g_imgW, g_imgH, g_zNear, g_zFar);
        ConvertToRaster(v2, &v2Raster, g_worldToCam, canvasB, canvasL, canvasT, canvasR, g_imgW, g_imgH, g_zNear, g_zFar);

        // Is triangle within screen boundary? No clipping yet
        float xMin = Helper::Min3(v0Raster.x, v1Raster.x, v2Raster.x);
        float yMin = Helper::Min3(v0Raster.y, v1Raster.y, v2Raster.y);
        float xMax = Helper::Max3(v0Raster.x, v1Raster.x, v2Raster.x);
        float yMax = Helper::Max3(v0Raster.y, v1Raster.y, v2Raster.y);
        if (xMin > (float)g_imgW - 1 || xMax < 0.0f || yMin > (float)g_imgH - 1 || yMax < 0.0f) { continue; }
        Vec2i v0i{(int)v0Raster.x, (int)v0Raster.y};
        Vec2i v1i{(int)v1Raster.x, (int)v1Raster.y};
        Vec2i v2i{(int)v2Raster.x, (int)v2Raster.y};
        Math::Vector<unsigned char, 3> color{255, 255, 255};
#if 0
        frameBuffer[v0i.y * g_imgW + v0i.x] = color;
        frameBuffer[v1i.y * g_imgW + v1i.x] = color;
        frameBuffer[v2i.y * g_imgW + v2i.x] = color;
#endif
        DrawLine(frameBuffer.data(), color, g_imgW, v0i.x, v1i.x, v0i.y, v1i.y);
        DrawLine(frameBuffer.data(), color, g_imgW, v1i.x, v2i.x, v1i.y, v2i.y);
        DrawLine(frameBuffer.data(), color, g_imgW, v2i.x, v0i.x, v2i.y, v0i.y);

#if 0
        Math::Vector<unsigned, 2> bbMin, bbMax;
        // Don't cast to unsigned, because xMin could be (-)
        bbMin.x = std::max(0, (int)std::floor(xMin));
        bbMin.y = std::max(0, (int)std::floor(yMin));
        bbMax.x = std::min((int)g_imgW - 1, (int)std::floor(xMax));
        bbMax.y = std::min((int)g_imgH - 1, (int)std::floor(yMax));

        float wTri = EdgeFunction(v0Raster, v1Raster, v2Raster);
        Vec3f pMin{bbMin.x + 0.5f, bbMin.y + 0.5f, 0.0f};
        float edge12Row = EdgeFunction(v1Raster, v2Raster, pMin);
        float edge20Row = EdgeFunction(v2Raster, v0Raster, pMin);
        float edge01Row = EdgeFunction(v0Raster, v1Raster, pMin);
        for (unsigned y = bbMin.y; y <= bbMax.y; ++y)
        {
            float edge12 = edge12Row;
            float edge20 = edge20Row;
            float edge01 = edge01Row;
            Vec3f v12 = v2Raster - v1Raster;
            Vec3f v20 = v0Raster - v2Raster;
            Vec3f v01 = v1Raster - v0Raster;
            for (unsigned x = bbMin.x; x <= bbMax.x; ++x)
            {
                // CW winding order
#if 0
                Vec3f pixel{x + 0.5f, y + 0.5f, 0.0f};
                float w0 = EdgeFunction(v1Raster, v2Raster, pixel);
                float w1 = EdgeFunction(v2Raster, v0Raster, pixel);
                float w2 = EdgeFunction(v0Raster, v1Raster, pixel);
#endif
                // Top-left rule
                bool overlap = true;
                overlap &= (edge12 == 0 ? ((v12.y == 0 && v12.x > 0) || v12.y > 0) : edge12 >= 0.0f);
                overlap &= (edge20 == 0 ? ((v20.y == 0 && v20.x > 0) || v20.y > 0) : edge20 >= 0.0f);
                overlap &= (edge01 == 0 ? ((v01.y == 0 && v01.x > 0) || v01.y > 0) : edge01 >= 0.0f);
                if (overlap)
                {
                    float w0 = edge12 / wTri;
                    float w1 = edge20 / wTri;
                    float w2 = edge01 / wTri;
                    float z = ComputeDepth(v0Raster, v1Raster, v2Raster, w0, w1, w2);
                    // Depth buffer test
                    if (z < depthBuffer[y * g_imgW + x])
                    {
                        depthBuffer[y * g_imgW + x] = z;

                        Vec2f st0Attribute = st0 * (1 / v0Raster.z);
                        Vec2f st1Attribute = st1 * (1 / v1Raster.z);
                        Vec2f st2Attribute = st2 * (1 / v2Raster.z);
                        Vec2f st = (st0Attribute * w0 + st1Attribute * w1 + st2Attribute * w2) * z;

                        // Facing ratio
                        // Compute pt in cam space, the pt is treated as vertex attribute
                        Vec3f v0Cam = MultiplyVecMat(v0, g_worldToCam);
                        Vec3f v1Cam = MultiplyVecMat(v1, g_worldToCam);
                        Vec3f v2Cam = MultiplyVecMat(v2, g_worldToCam);
                        
                        Vec3f ptCam;
                        ptCam.x = w0 * (v0Cam.x / -v0Cam.z) + w1 * (v1Cam.x / -v1Cam.z) + w2 * (v2Cam.x / -v2Cam.z);
                        ptCam.y = w0 * (v0Cam.y / -v0Cam.z) + w1 * (v1Cam.y / -v1Cam.z) + w2 * (v2Cam.y / -v2Cam.z);
                        ptCam.z = -1;
                        ptCam *= z;

                        // View direction: vec from cam to pt of the currently shaded triangle. Because cam pos is
                        // (0,0,0), if we compute the shaded pt shadedPt in cam space, view direction = -shadedPt
                        Vec3f viewDir{-ptCam.x, -ptCam.y, -ptCam.z};
                        Vec3f viewDirNormalized = Math::Normal<float, 3>(viewDir);

                        Vec3f triNormal = Math::Cross<float, 3>(v1Cam - v0Cam, v2Cam - v0Cam);
                        triNormal = Math::Normal<float, 3>(triNormal);

                        float nDotView = std::max(0.0f, Math::Dot<float, 3>(triNormal, viewDirNormalized));
                        const float m = 10.0f;
                        float checker = (float)((std::fmod(st.x * m, 1.0f) > 0.5f) ^ (std::fmod(st.y * m, 1.0f) < 0.5f));
                        float c = 0.3f * (1.0f - checker) + 0.7f * checker;
                        nDotView *= c;

                        frameBuffer[y * g_imgW + x][0] = (unsigned char)(nDotView * 255.0f);
                        frameBuffer[y * g_imgW + x][1] = (unsigned char)(nDotView * 255.0f);
                        frameBuffer[y * g_imgW + x][2] = (unsigned char)(nDotView * 255.0f);
                        
                    }
                }
                // 1px to the right
                edge12 += v12.y;
                edge20 += v20.y;
                edge01 += v01.y;

            }
            // 1px row below
            edge12Row -= v12.x;
            edge20Row -= v20.x;
            edge01Row -= v01.x;

        }
#endif
    }

    t.Stop();

    ofstream ofs{"./cow.ppm", ofstream::out | ofstream::binary};
    ofs << "P6\n" << g_imgW << " " << g_imgH << "\n255\n";
    ofs.write((char*)frameBuffer.data(), g_imgW * g_imgH * 3);
    ofs.close();

}

