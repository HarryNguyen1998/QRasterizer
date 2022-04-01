#include <iostream>

#include "Renderer/Model.h"

int main(int argc, char **argv)
{
#if 0
    std::vector<Model> models;

    // A triangle located on top left
    {
        std::vector<Vec3f> tri {
            {-0.5f, -0.5f, 0.0f},
            {0.0f, 0.5f, -0.5f},
            {0.5f, -0.5f, 0.0f},
        };
        Mat44f transMat = Math::InitTranslation(-1.5f, 1.0f, 0.0f);
        for (int i = 0; i < tri.size(); ++i)
            tri[i] = Math::MultiplyVecMat(tri[i], transMat);
        std::vector<int> indices{
            0, 2, 1
        };
        models.emplace_back(std::move(tri), std::move(indices));
    }

    // A textured triangle located on top right
    {
        std::vector<Vec3f> tri {
            {-0.5f, 0.5f, 1.0f},
            {0.5f, 0.5f, 0.0f},
            {0.0f, -0.5f, -1.0f},
        };
        Mat44f transMat = Math::InitTranslation(1.5f, 1.0f, 0.0f);
        for (int i = 0; i < tri.size(); ++i)
            tri[i] = Math::MultiplyVecMat(tri[i], transMat);
        std::vector<int> indices{
            0, 2, 1
        };
        std::vector<Vec2f> texCoords{
            {0.0f, 0.0f},
            {1.0f, 0.0f},
            {0.5f, 1.0f},
        };
        models.emplace_back(std::move(tri), indices, std::move(texCoords), indices);
    }

    {
        std::vector<Vec3f> quad{
            {-1.0f, -1.0f, 0.0f},
            {-1.0f, 1.0f, 0.0f},
            {1.0f, 1.0f, 0.0f},
            {1.0f, -1.0f, 0.0f},
        };
        Mat44f transMat = Math::InitTranslation(-1.5f, -1.0f, 0.0f);
        for (int i = 0; i < quad.size(); ++i)
            quad[i] = Math::MultiplyVecMat(quad[i], transMat);
        std::vector<int> indices{
            0, 2, 1,
            0, 3, 2,
        };
        std::vector<Vec2f> texCoords
        {
            {0.0f, 1.0f},
            {0.0f, 0.0f},
            {1.0f, 0.0f},
            {1.0f, 1.0f},
        };
        std::vector<int> texIndices
        {
            0, 2, 1,
            0, 3, 2,
        };
        models.emplace_back(std::move(quad), std::move(indices), std::move(texCoords), std::move(texIndices));
    }

    {
        std::vector<Vec3f> quad{
            {-1.0f, 1.0f, 1.0f},
            {-1.0f, 1.0f, -1.0f},
            {1.0f, 1.0f, -1.0f},
            {1.0f, 1.0f, 1.0f},
        };
        Mat44f transMat = Math::InitTranslation(1.5f, -2.0f, 0.0f);
        for (int i = 0; i < quad.size(); ++i)
            quad[i] = Math::MultiplyVecMat(quad[i], transMat);
        std::vector<int> indices{
            0, 2, 1,
            0, 3, 2,
        };
        std::vector<Vec2f> texCoords
        {
            {0.0f, 1.0f},
            {0.0f, 0.0f},
            {1.0f, 0.0f},
            {1.0f, 1.0f},
        };
        std::vector<int> texIndices
        {
            0, 2, 1,
            0, 3, 2,
        };
        models.emplace_back(std::move(quad), std::move(indices), std::move(texCoords), std::move(texIndices));
    }

    std::vector<Vec3f> colors {
        {1.0f, 0.0f, 0.0f}, // red
        {0.0f, 1.0f, 0.0f}, // green
        {0.0f, 0.0f, 1.0f}, // blue
    };

    // Render triangle using color
    m_qrenderer->Render(changedModels[0], colors, drawMode);
    // Render triangle using texture
    m_qrenderer->Render(textureManager.Load("Assets/checkerboard.jpg", m_qrenderer->GetRenderer()).get(),
        changedModels[1], drawMode);
    // Render quads using texture
    m_qrenderer->Render(textureManager.Load("Assets/bricks.jpg", m_qrenderer->GetRenderer()).get(),
        changedModels[2], drawMode);
    m_qrenderer->Render(textureManager.Load("Assets/bricks2.jpg", m_qrenderer->GetRenderer()).get(),
        changedModels[3], drawMode);

    QRenderer renderer;
    renderer.Render(QRendererMode::kDrawLine, model, texture);
#endif

    Model tri{
        InputWindingOrder::kCW,
        std::vector<Vec3f> {
            {-0.5f, -0.5f, 0.0f},
            {0.0f, 0.5f, -0.5f},
            {0.5f, -0.5f, 0.0f},
        },
        std::vector<Vec3f> {
            {1.0f, 0.0f, 0.0f},
            {0.0f, 1.0f, 0.0f},
            {0.0f, 0.0f, 1.0f},
        },
        std::vector<int> {
            0, 1, 2,
        },
    };
    int i = 0;

#if 0
    QRasterizer rasterizer;
    rasterizer.SetPixelShader<PixelShader>();

    rasterizer.DrawTriangle(v0, v1, v2);
#endif
}

