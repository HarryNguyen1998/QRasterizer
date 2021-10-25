#include "Renderer/RenderContext.h"
unsigned char RenderContext::DecodeGamma(float value)
{
    int quantizedValue = (int)(value * 255 + 0.5f);
    return g_gammaDecodedTable[quantizedValue];
}

