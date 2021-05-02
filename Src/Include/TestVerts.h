#include <vector>

#include "Math/Vector.h"
#include "Renderer/IndexModel.h"

// @brief Contains sample IndexModel for testing purposes
namespace TestVerts
{
    // @brief a test triangle
    IndexModel& Triangle()
    {
        std::vector<Vec3f> verts{{-0.5f, -0.5f, 0.0f},  // lower-left corner
            {0.5f, -0.5f, 0.0f},                        // lower-right corner
            {0.0f, 0.5f, 0.0f}};                        // top-center corner
        std::vector<int> indices{0, 1, 2};
        std::vector<Vec2f> texCoords{{0.0f, 0.0f},  // lower-left corner
            {1.0f, 0.0f},                           // lower-right corner
            {0.5f, 1.0f}};                          // top-center corner

        static IndexModel model{verts, texCoords, std::vector<Vec3f>(),
            indices, indices, indices};
        return model;
    }

    // @brief A test rectangle
    // @todo I seems to have misunderstood how texture work. It doesn't cover the whole screen???
    IndexModel& Rect()
    {
        std::vector<Vec3f> verts{{-1.0f, -0.4f, 0.0f},  // lower-left corner
            {-0.2f, -0.4f, 0.0f},                       // lower-right corner
            {-1.0f, 0.4f, 0.0f},                        // top-left corner
            {-0.2f, 0.4f, 0.0f}};                       // top-right corner
        std::vector<int> indices{0, 1, 2, 2, 1, 3};
        std::vector<Vec2f> texCoords{{0.0f, 0.0f},  // lower-left corner
            {1.0f, 0.0f},                           // lower-right corner
            {0.0f, 1.0f},                           // top-left corner
            {1.0f, 1.0f}};                          // top-right corner

        static IndexModel model{verts, texCoords, std::vector<Vec3f>(),
            indices, indices, indices};
        return model;
    }

    // @brief A test rectangle rotated by 45deg, used to test perspective correct mapping
    IndexModel& RectPerspectiveCorrect()
    {
        std::vector<Vec3f> verts{
            {0.100000001f, -0.282842726f, 0.282842726f},   // lower-left corner
            {0.899999976f, -0.282842726f, 0.282842726f},   // lower-right corner
            {0.100000001f, 0.282842726f, -0.282842726f},   // top-left corner
            {0.899999976f, 0.282842726f, -0.282842726f}};  // top-right corner
        std::vector<int> indices{0, 1, 2, 2, 1, 3};
        std::vector<Vec2f> texCoords{{0.0f, 0.0f},  // lower-left corner
            {1.0f, 0.0f},                           // lower-right corner
            {0.0f, 1.0f},                           // top-left corner
            {1.0f, 1.0f}};                          // top-right corner

        static IndexModel model{verts, texCoords, std::vector<Vec3f>(),
            indices, indices, indices};
        return model;
    }
    
}