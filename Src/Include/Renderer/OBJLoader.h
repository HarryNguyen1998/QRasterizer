#pragma once
#include <string>

// Forward declarations
struct Model;

// @brief A globally accessible namespace containing funcs that deal with .obj file format
namespace OBJ
{
    // @brief Simple .obj loader, only parse 3D vertices, 2D texture coordinates, 3D normals, and
    // face data.
    Model LoadFileData(const std::string& filePath);

    // @brief Internal helper function
    static void ParseLine(Model *outMesh, std::string line);
}