#pragma once
#include <string>

// Forward declarations
struct IndexModel;

namespace OBJ
{
    // The func assumes that the filePath is valid
    IndexModel LoadFileData(const std::string& filePath);
}