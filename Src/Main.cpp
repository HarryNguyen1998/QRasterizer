#include "Math/Vector.h"
#include "Math/Matrix.h"
#include "Vertices.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <fstream>
#include <cmath>

using namespace Helper;
using std::ofstream;

// The mode doesn't change anything if device gate and film gate resolution are the same
enum class ResolutionGateMode
{
	kFill,		// Fit resolution gate into film gate (scale down)
	kOverscan,	// Fit film gate into resolution gate (scale up)
};
ResolutionGateMode g_resolutionGateMode = ResolutionGateMode::kOverscan;

// Based on pinhole cam model
void ComputeCanvasCoord(float focalLength, float zNear, ResolutionGateMode mode,
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

void ConvertToRaster(const Vec3f pWorld, Vec3f* pRaster, const Mat44f &worldToCamera,
	float canvasB, float canvasL, float canvasT, float canvasR,
	unsigned imageW, unsigned imageH, float zNear)
{
	bool isVisible = true;

	// Convert from world space to cam space
	Vec3f pCamera;
	worldToCamera.MultiplyVecMat(pWorld, &pCamera);
	// Perspective divide
	Vec2f pScreen{ pCamera.x / -pCamera.z * zNear, pCamera.y / -pCamera.z * zNear };
	// From screen space, convert to NDC space [-1, 1]
	Vec2f pNDC;
    pNDC.x = (2 * pScreen.x) / (canvasR - canvasL) - (canvasR + canvasL) / (canvasR - canvasL);
    pNDC.y = (2 * pScreen.y) / (canvasT - canvasB) - (canvasT + canvasB) / (canvasT - canvasB);
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
    ComputeCanvasCoord(g_focalLength, g_zNear, g_resolutionGateMode,
        g_filmGateW, g_filmGateH, g_imgW, g_imgH,
        &canvasB, &canvasL, &canvasT, &canvasR);

    Math::Vec3<unsigned char>* frameBuffer = new Math::Vec3<unsigned char>[g_imgW * g_imgH];
    std::fill(frameBuffer, frameBuffer + g_imgW * g_imgH, Math::Vec3<unsigned char>{0x19});

    float* depthBuffer{new float[g_imgW * g_imgH]};
    std::fill(depthBuffer, depthBuffer + g_imgW * g_imgH, g_zFar);
    
    Timer t;
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
        ConvertToRaster(v0, &v0Raster, g_worldToCam, canvasB, canvasL, canvasT, canvasR, g_imgW, g_imgH, g_zNear);
        ConvertToRaster(v1, &v1Raster, g_worldToCam, canvasB, canvasL, canvasT, canvasR, g_imgW, g_imgH, g_zNear);
        ConvertToRaster(v2, &v2Raster, g_worldToCam, canvasB, canvasL, canvasT, canvasR, g_imgW, g_imgH, g_zNear);

        // Is triangle within screen boundary?
        float xMin = Min3(v0Raster.x, v1Raster.x, v2Raster.x);
        float yMin = Min3(v0Raster.y, v1Raster.y, v2Raster.y);
        float xMax = Max3(v0Raster.x, v1Raster.x, v2Raster.x);
        float yMax = Max3(v0Raster.y, v1Raster.y, v2Raster.y);
        if (xMin > (float)g_imgW - 1 || xMax < 0.0f || yMin > (float)g_imgH - 1 || yMax < 0.0f) { continue; }

        Math::Vec2<unsigned> bbMin, bbMax;
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
                ofs1 << w0 << " " << w1 << " " << w2 << "\n";
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
                        Vec3f v0Cam, v1Cam, v2Cam;
                        g_worldToCam.MultiplyVecMat(v0, &v0Cam);
                        g_worldToCam.MultiplyVecMat(v1, &v1Cam);
                        g_worldToCam.MultiplyVecMat(v2, &v2Cam);
                        
                        Vec3f ptCam;
                        ptCam.x = w0 * (v0Cam.x / -v0Cam.z) + w1 * (v1Cam.x / -v1Cam.z) + w2 * (v2Cam.x / -v2Cam.z);
                        ptCam.y = w0 * (v0Cam.y / -v0Cam.z) + w1 * (v1Cam.y / -v1Cam.z) + w2 * (v2Cam.y / -v2Cam.z);
                        ptCam.z = -1;
                        ptCam *= z;

                        // View direction: vec from cam to pt of the currently shaded triangle. Because cam pos is
                        // (0,0,0), if we compute the shaded pt shadedPt in cam space, view direction = -shadedPt
                        Vec3f viewDir{-ptCam.x, -ptCam.y, -ptCam.z};
                        viewDir.Normalize();

                        Vec3f triNormal = (v1Cam - v0Cam).Cross(v2Cam - v0Cam);
                        triNormal.Normalize();

                        float nDotView = std::max(0.0f, triNormal.Dot(viewDir));
                        const int m = 10;
                        float checker = (std::fmod(st.x * m, 1.0f) > 0.5f) ^ (std::fmod(st.y * m, 1.0f) < 0.5f);
                        float c = 0.3f * (1.0f - checker) + 0.7f * checker;
                        nDotView *= c;

                        frameBuffer[y *g_imgW + x][0] = (nDotView * 255.0f);
                        frameBuffer[y *g_imgW + x][1] = (nDotView * 255.0f);
                        frameBuffer[y *g_imgW + x][2] = (nDotView * 255.0f);
                        
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
    }

    t.Stop();

    ofstream ofs{"./cow.ppm", ofstream::out | ofstream::binary};
    ofs << "P6\n" << g_imgW << " " << g_imgH << "\n255\n";
    ofs.write((char*)frameBuffer, g_imgW * g_imgH * 3);
    ofs.close();

    delete[] frameBuffer;
    delete[] depthBuffer;
}

