#pragma once
#include <string>

// Forward declarations
struct IndexModel;

// @brief A globally accessible namespace containing funcs that deal with .obj file format
namespace OBJ
{
    // @todo error-check filePath
    IndexModel LoadFileData(const std::string& filePath);
}