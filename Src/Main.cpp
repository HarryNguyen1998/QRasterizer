#include "Math/Vector.h"
#include "Math/Matrix.h"

#include <iostream>
#include <fstream>
#include <cmath>

// We have 146 vertices, each vertex coord in (x, y, z)
const Vec3f g_verts[146] = { 
    Vec3f{  -2.5703f,   0.78053f,  -2.4e-5f}, Vec3f{ -0.89264f,  0.022582f,  0.018577f}, 
    Vec3f{   1.6878f, -0.017131f,  0.022032f}, Vec3f{   3.4659f,  0.025667f,  0.018577f}, 
    Vec3f{  -2.5703f,   0.78969f, -0.001202f}, Vec3f{ -0.89264f,   0.25121f,   0.93573f}, 
    Vec3f{   1.6878f,   0.25121f,    1.1097f}, Vec3f{   3.5031f,   0.25293f,   0.93573f}, 
    Vec3f{  -2.5703f,    1.0558f, -0.001347f}, Vec3f{ -0.89264f,    1.0558f,    1.0487f}, 
    Vec3f{   1.6878f,    1.0558f,    1.2437f}, Vec3f{   3.6342f,    1.0527f,    1.0487f}, 
    Vec3f{  -2.5703f,    1.0558f,       0.0f}, Vec3f{ -0.89264f,    1.0558f,       0.0f}, 
    Vec3f{   1.6878f,    1.0558f,       0.0f}, Vec3f{   3.6342f,    1.0527f,       0.0f}, 
    Vec3f{  -2.5703f,    1.0558f,  0.001347f}, Vec3f{ -0.89264f,    1.0558f,   -1.0487f}, 
    Vec3f{   1.6878f,    1.0558f,   -1.2437f}, Vec3f{   3.6342f,    1.0527f,   -1.0487f}, 
    Vec3f{  -2.5703f,   0.78969f,  0.001202f}, Vec3f{ -0.89264f,   0.25121f,  -0.93573f}, 
    Vec3f{   1.6878f,   0.25121f,   -1.1097f}, Vec3f{   3.5031f,   0.25293f,  -0.93573f}, 
    Vec3f{   3.5031f,   0.25293f,       0.0f}, Vec3f{  -2.5703f,   0.78969f,       0.0f}, 
    Vec3f{   1.1091f,    1.2179f,       0.0f}, Vec3f{    1.145f,     6.617f,       0.0f}, 
    Vec3f{   4.0878f,    1.2383f,       0.0f}, Vec3f{  -2.5693f,    1.1771f, -0.081683f}, 
    Vec3f{  0.98353f,    6.4948f, -0.081683f}, Vec3f{ -0.72112f,    1.1364f, -0.081683f}, 
    Vec3f{   0.9297f,     6.454f,       0.0f}, Vec3f{  -0.7929f,     1.279f,       0.0f}, 
    Vec3f{  0.91176f,    1.2994f,       0.0f} };

// Triangle index array. Each successive group of 3 ints are the pos of each vertex in the vertex
// array that form a triangle. Ex: g_trisIndices[0], g_trisIndices[1] and g_trisIndices[2] are 3
// vertices in g_verts that form a triangle, then 3, 4, 5, and so on. So, only these vertex
// combinations should make up triangles.
const unsigned g_numTris = 51;
const unsigned g_trisIndices[g_numTris * 3] = {
	4,   0,   5,   0,   1,   5,   1,   2,   5,   5,   2,   6,   3,   7,   2,
	2,   7,   6,   5,   9,   4,   4,   9,   8,   5,   6,   9,   9,   6,  10,
	7,  11,   6,   6,  11,  10,   9,  13,   8,   8,  13,  12,  10,  14,   9,
	9,  14,  13,  10,  11,  14,  14,  11,  15,  17,  16,  13,  12,  13,  16,
	13,  14,  17,  17,  14,  18,  15,  19,  14,  14,  19,  18,  16,  17,  20,
	20,  17,  21,  18,  22,  17,  17,  22,  21,  18,  19,  22,  22,  19,  23,
	20,  21,   0,  21,   1,   0,  22,   2,  21,  21,   2,   1,  22,  23,   2,
	2,  23,   3,   3,  23,  24,   3,  24,   7,  24,  23,  15,  15,  23,  19,
	24,  15,   7,   7,  15,  11,   0,  25,  20,   0,   4,  25,  20,  25,  16,
	16,  25,  12,  25,   4,  12,  12,   4,   8,  26,  27,  28,  29,  30,  31,
	32,  34,  33 };

// Physical camera model
float g_focalLength = 35.0f;	// mm
// inch
float g_filmApertureWidth = 0.885f;
float g_filmApertureHeight = 0.885f;
static const float inchTomm = 25.4f;
float g_zNear = 0.01f;
float g_zFar = 1000.0f;
// Img resolution in pixels
const int g_imgWidth = 1024;
const int g_imgHeight = 1024;

// The mode doesn't change anything if device gate and film gate resolution are the same
enum class ResolutionGateMode
{
	kFill,		// Fit resolution gate into film gate (scale down)
	kOverscan,	// Fit film gate into resolution gate (scale up)
};
ResolutionGateMode g_resolutionGateMode = ResolutionGateMode::kOverscan;

bool ComputePixelCoordinates(const Vec3f pWorld, Vec2i &pRaster, const Mat44f &worldToCamera,
	float canvasB, float canvasL, float canvasT, float canvasR,
	unsigned imageWidth, unsigned imageHeight, float zNear)
{
	bool isVisible = true;

	// Convert from world space to cam space
	Vec3f pCamera;
	worldToCamera.MultiplyVecMat(pWorld, &pCamera);
	// Perspective divide
	Vec2f pScreen{ pCamera.x / -pCamera.z * zNear, pCamera.y / -pCamera.z * zNear };
	// From screen space, convert to NDC space.
	Vec2f pNDC;
	pNDC.x = (pScreen.x + canvasR) * (1.0f / (canvasR * 2));
	pNDC.y = (pScreen.y + canvasT) * (1.0f / (canvasT * 2));
	// From NDC space to raster space.
	pRaster.x = (int)(pNDC.x * imageWidth);
	pRaster.y = (int)((1.0f - pNDC.y) * imageHeight);

	if (pScreen.x < canvasL || pScreen.x > canvasR ||
		pScreen.y < canvasB || pScreen.y > canvasT)
	{
		isVisible = false;
	}

	return isVisible;
}

// (c-a) Cross (b-a)
inline float EdgeFunction(const Vec3f& a, const Vec3f& b, const Vec3f& c)
{
    float result = (c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x);
    return result;
}
typedef unsigned char RGB[3];

// Pls remember not to pass a vector with z-value of 0.
inline float ComputeDepth(const Vec3f& v0, const Vec3f& v1, const Vec3f& v2, float w0, float w1, float w2)
{
    float depthInverse = (w0 / v0.z) + (w1 / v1.z) + (w2 / v2.z);
    float depth = (1 / depthInverse);
    return depth;
}


using std::ofstream;
int main()
{
#if 0
	// Compute canvas coords
	float filmAspectRatio = g_filmApertureWidth / g_filmApertureHeight;
	float deviceAspectRatio = (float)g_imgWidth / g_imgHeight;
	float canvasT = ((g_filmApertureHeight * inchTomm) / (2 * g_focalLength)) * g_zNear;
	float canvasR = canvasT * filmAspectRatio;

	switch (g_resolutionGateMode)
	{
	default:
	case ResolutionGateMode::kFill:
	{
		if (filmAspectRatio > deviceAspectRatio)
		{
			canvasR *= deviceAspectRatio / filmAspectRatio;
		}
		else
		{
			canvasT *= filmAspectRatio / deviceAspectRatio;
		}
	}
	case ResolutionGateMode::kOverscan:
	{
		if (filmAspectRatio > deviceAspectRatio)
		{
			canvasT *= filmAspectRatio / deviceAspectRatio;
		}
		else
		{
			canvasR *= deviceAspectRatio / filmAspectRatio;
		}
	}
	}

	float canvasB = -canvasT;
	float canvasL = -canvasR;
    float canvasWidth = 2, canvasHeight = 2;

	printf("Screen window coordinates: %f %f %f %f\n", canvasB, canvasL, canvasT, canvasR); 
    printf("Film Aspect Ratio: %f\nDevice Aspect Ratio: %f\n", filmAspectRatio, deviceAspectRatio); 
    printf("Angle of view: %f (deg)\n", 2 * atan((g_filmApertureWidth * inchTomm / 2) / g_focalLength) * 180 / 3.14159265359f);

	ofstream ofs{"./proj.svg"};
	ofs << "<svg version=\"1.1f\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" xmlns=\"http://www.w3.org/2000/svg\" height=\"512\" width=\"512\">\n";

	// We exported the cam matrix from Maya. We need to compute its inverse, which is the mat used
	// in ComputePixelCoord()
    Mat44f cameraToWorld(-0.95424f, 0.0f, 0.299041f, 0.0f, 0.0861242f, 0.95763f, 0.274823f, 0.0f, -0.28637f, 0.288002f, -0.913809f, 0.0f, -3.734612f, 7.610426f, -14.152769f, 1.0f); 
    Mat44f worldToCamera = cameraToWorld.Inverse(); 
    std::cerr << worldToCamera << std::endl;

	for (uint32_t i = 0; i < g_numTris; ++i) {
		const Vec3f &v0World = g_verts[g_trisIndices[i * 3]];
		const Vec3f &v1World = g_verts[g_trisIndices[i * 3 + 1]];
		const Vec3f &v2World = g_verts[g_trisIndices[i * 3 + 2]];
		Vec2i v0Raster, v1Raster, v2Raster;

		bool isVisible = true;
		isVisible &= ComputePixelCoordinates(v0World, v0Raster, worldToCamera, canvasB, canvasL, canvasT, canvasR, g_imgWidth, g_imgHeight, g_zNear);
		isVisible &= ComputePixelCoordinates(v1World, v1Raster, worldToCamera, canvasB, canvasL, canvasT, canvasR, g_imgWidth, g_imgHeight, g_zNear);
		isVisible &= ComputePixelCoordinates(v2World, v2Raster, worldToCamera, canvasB, canvasL, canvasT, canvasR, g_imgWidth, g_imgHeight, g_zNear);

		int val = isVisible ? 0 : 255;
		ofs << "<line x1=\"" << v0Raster.x << "\" y1=\"" << v0Raster.y << "\" x2=\"" << v1Raster.x << "\" y2=\"" << v1Raster.y << "\" style=\"stroke:rgb(" << val << ",0,0);stroke-width:1\" />\n";
		ofs << "<line x1=\"" << v1Raster.x << "\" y1=\"" << v1Raster.y << "\" x2=\"" << v2Raster.x << "\" y2=\"" << v2Raster.y << "\" style=\"stroke:rgb(" << val << ",0,0);stroke-width:1\" />\n";
		ofs << "<line x1=\"" << v2Raster.x << "\" y1=\"" << v2Raster.y << "\" x2=\"" << v0Raster.x << "\" y2=\"" << v0Raster.y << "\" style=\"stroke:rgb(" << val << ",0,0);stroke-width:1\" />\n";
	}
	ofs << "</svg>\n";
	ofs.close();
#endif
    RGB* frameBuffer = new RGB[g_imgWidth * g_imgHeight];
    memset(frameBuffer, 0x0, g_imgWidth * g_imgHeight * 3);

    float* depthBuffer{new float[g_imgWidth * g_imgHeight]};
    std::fill(depthBuffer, depthBuffer + g_imgWidth * g_imgHeight, FLT_MAX);

    // These are our rasterized coord, in CCW winding order.
    Vec3f v0{491.407f, 411.407f, 20.0f}; 
    Vec3f v1{148.593f, 68.5928f, 20.0f}; 
    Vec3f v2{148.593f, 411.407f, 20.0f}; 
    Vec3f c0{1.0f, 0.0f, 0.0f}; 
    Vec3f c1{0.0f, 1.0f, 0.0f}; 
    Vec3f c2{0.0f, 0.0f, 1.0f}; 

    // Our white tri, which should overlap the colored tri.
    Vec3f v3{400.0f, 600.0f, 1.0f};
    Vec3f v4{269.0f, 350.0f, 1.0f};
    Vec3f v5{269.0f, 600.0f, 1.0f};

    float wTriangle = EdgeFunction(v0, v1, v2);
    float wTriangle2 = EdgeFunction(v3, v4, v5);
    for (int j = 0; j < g_imgHeight; ++j)
    {
        for (int i = 0; i < g_imgWidth; ++i)
        {
            Vec3f p{ i + 0.5f, j + 0.5f, 0.0f };
            float w0 = EdgeFunction(v1, v2, p) / wTriangle;
            float w1 = EdgeFunction(v2, v0, p) / wTriangle;
            float w2 = EdgeFunction(v0, v1, p) / wTriangle;

            float w3 = EdgeFunction(v4, v5, p) / wTriangle2;
            float w4 = EdgeFunction(v5, v3, p) / wTriangle2;
            float w5 = EdgeFunction(v3, v4, p) / wTriangle2;

            // Check if pixel overlaps the tri. If we use barycentric coords directly, we don't care
            // about winding order.
            bool doesOverlap = false;
            float r, g, b;
            if (w0 >= 0 && w1 >= 0 && w2 >= 0)
            {
                doesOverlap = true;
                float depthVal = ComputeDepth(v0, v1, v2, w0, w1, w2);
                if (depthVal < depthBuffer[j * g_imgWidth + i])
                {
                    r = c0[0] * w0 + c1[0] * w1 + c2[0] * w2;
                    g = c0[1] * w0 + c1[1] * w1 + c2[1] * w2;
                    b = c0[2] * w0 + c1[2] * w1 + c2[2] * w2;
                    depthBuffer[j * g_imgWidth + i] = depthVal;
                }
            }

            if (w3 >= 0 && w4 >= 0 && w5 >= 0)
            {
                doesOverlap = true;
                float depthVal = ComputeDepth(v3, v4, v5, w3, w4, w5);
                if (depthVal < depthBuffer[j * g_imgWidth + i])
                {
                    r = 1.0f;
                    g = 1.0f;
                    b = 1.0f;
                    depthBuffer[j * g_imgWidth + i] = depthVal;
                }
            }

            if (doesOverlap)
            {
                frameBuffer[j * g_imgWidth + i][0] = (unsigned char)(r * 255.0f);
                frameBuffer[j * g_imgWidth + i][1] = (unsigned char)(g * 255.0f);
                frameBuffer[j * g_imgWidth + i][2] = (unsigned char)(b * 255.0f);
            }
        }
    }

    ofstream ofs{"./tri.ppm", ofstream::out | ofstream::binary};
    ofs << "P6\n" << g_imgWidth << " " << g_imgHeight << "\n255\n";
    ofs.write((char*)frameBuffer, g_imgWidth * g_imgHeight * 3);
    ofs.close();

    delete[] frameBuffer;
    delete[] depthBuffer;
}
