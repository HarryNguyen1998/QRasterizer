#include "SDL.h"    // Uint32

#include <cmath>

#include "Renderer/RenderContext.h"
#include "Renderer/IndexModel.h"
#include "Renderer/Texture.h"
#include "Utils/Helper.h"

void RenderContext::GetCanvasCoord(float* canvasT, float* canvasR, float* canvasL, float* canvasB,
    float focalLength, float zNear, ResolutionGateMode mode,
    float filmGateW, float filmGateH, int imgW, int imgH)
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

#if 0
	printf("Screen window coordinates: %f %f %f %f\n", *canvasB, *canvasL, *canvasT, *canvasR); 
    printf("Film Aspect Ratio: %f\nDevice Aspect Ratio: %f\n", filmAspectRatio, deviceAspectRatio); 
    printf("Angle of view: %f (deg)\n", 2 * atan((filmGateW * inchToMM / 2) / focalLength) * 180 / 3.14159265359f);
#endif
}

// The z-comp of Cross(c-a, b-a)
float RenderContext::EdgeFunction(const Vec3f& a, const Vec3f& b, const Vec3f& c)
{
    float result = (c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x);
    return result;
}

float RenderContext::ComputeDepth(const Vec3f& v0, const Vec3f& v1, const Vec3f& v2, float w0, float w1, float w2)
{
    float depthInverse = (w0 / v0.z) + (w1 / v1.z) + (w2 / v2.z);
    float depth = (1 / depthInverse);
    return depth;
}

void RenderContext::PutPixel(unsigned char* pixels, unsigned color, int bpp, int pixelStride,
    int x, int y)
{
    unsigned char* p = pixels + (y * pixelStride + x) * bpp;
    *(Uint32 *)p = color;
}

void RenderContext::DrawLine(unsigned* pixels, unsigned color, int pxStride,
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

unsigned char RenderContext::DecodeGamma(float value)
{
    int quantizedValue = (int)(value * 255 + 0.5f);
    return g_gammaDecodedTable[quantizedValue];
}

Mat44f RenderContext::LookAt(Vec3f eye, Vec3f center, Vec3f up)
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

Vec3f RenderContext::NDCToRaster(const Vec3f& v, int w, int h)
{
    Vec3f result;
    result.x = (1 + v.x) * 0.5f * w;
    // In raster space, y is down so we need to invert the direction
    result.y = (1 - v.y) * 0.5f * h;
    result.z = v.z;
    return result;
}

#if 0
void RenderContext::DrawTriangle(unsigned char* pxBuffer, float* zBuffer, Vec3f v0, Vec3f v1, Vec3f v2, unsigned color)
{
    Mat44f viewMat = Math::InitTranslation(0.0f, 0.0f, -3.0f);
    Mat44f projMat = Math::InitPersp(camSettings.fovY, camSettings.imgAspectRatio, camSettings.zNear, camSettings.zFar);

    // x, y in range [-1, 1], z in range [0, 1]
    v0 = MultiplyPtMat(v0, viewMat * projMat);
    v1 = MultiplyPtMat(v1, viewMat * projMat);
    v2 = MultiplyPtMat(v2, viewMat * projMat);

    Vec3f v0Raster = NDCToRaster(v0, m_w, m_h);
    Vec3f v1Raster = NDCToRaster(v1, m_w, m_h);
    Vec3f v2Raster = NDCToRaster(v2, m_w, m_h);

    // Prep for edge test
    Vec3f edge0 = v2Raster - v1Raster;
    Vec3f edge1 = v0Raster - v2Raster;
    Vec3f edge2 = v1Raster - v0Raster;
    float areaTriTimes2 = EdgeFunction(v0Raster, v1Raster, v2Raster);

    Vec2i bbMin, bbMax;
    bbMin.x = std::max(0, (int)Helper::Min3(v0Raster.x, v1Raster.x, v2Raster.x));
    bbMin.y = std::max(0, (int)Helper::Min3(v0Raster.y, v1Raster.y, v2Raster.y));
    bbMax.x = std::min((int)m_w, (int)Helper::Max3(v0Raster.x, v1Raster.x, v2Raster.x));
    bbMax.y = std::min((int)m_h, (int)Helper::Max3(v0Raster.y, v1Raster.y, v2Raster.y));

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
            if (zBuffer[y * m_w + x] >= px.z)
            {
                zBuffer[y * m_w + x] = px.z;
                PutPixel(pxBuffer, color, bpp, imgW, (int)px.x, (int)px.y);
            }

        }
    }
}
#endif

void RenderContext::DrawTriangles(SDL_Renderer* renderer, const TextureWrapper& bitmap, float* zBuffer,
    const IndexModel& model, const TextureWrapper& tex)
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

        DrawTriangle(renderer, bitmap, zBuffer, v0, v1, v2, uv0, uv1, uv2, tex);

    }
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
    v0 = MultiplyPtMat(v0, viewMat * projMat);
    v1 = MultiplyPtMat(v1, viewMat * projMat);
    v2 = MultiplyPtMat(v2, viewMat * projMat);

    Vec3f v0Raster = NDCToRaster(v0, bitmap.Width(), bitmap.Height());
    Vec3f v1Raster = NDCToRaster(v1, bitmap.Width(), bitmap.Height());
    Vec3f v2Raster = NDCToRaster(v2, bitmap.Width(), bitmap.Height());

    // Prep for edge test
    Vec3f edge0 = v2Raster - v1Raster;
    Vec3f edge1 = v0Raster - v2Raster;
    Vec3f edge2 = v1Raster - v0Raster;
    float areaTriTimes2 = EdgeFunction(v0Raster, v1Raster, v2Raster);

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
            
            // Affine texture mapping
            Vec2f uv = uv0 * w0 + uv1 * w1 + uv2 * w2;

            // = sign here because we want to draw the img next frame too
            if (zBuffer[y * bitmap.Width() + x] >= px.z)
            {
                zBuffer[y * bitmap.Width() + x] = px.z;
                tex.RenderPixel(renderer, (int)(uv.x * tex.Width()), (int)(uv.y * tex.Height()),
                    (int)px.x, (int)px.y);
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

void RenderContext::SetCamToDefault(float imgAspectRatio)
{
    camSettings.focalLength = 35.0f;	// mm
    camSettings.filmGateW   = 0.885f;   // inch
    camSettings.filmGateH   = 0.885f;
    camSettings.zNear       = 0.125f;
    camSettings.zFar        = 100.0f;
    camSettings.fovY        = 60.0f * (float)M_PI / 180.0f;
    camSettings.imgAspectRatio = imgAspectRatio;
}

