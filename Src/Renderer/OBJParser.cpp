#include <fstream>
#include <sstream> // std::stringstream

#include "Renderer/OBJParser.h"
#include "Renderer/IndexModel.h"
#include "Utils/Helper.h"

namespace OBJ
{
    IndexModel LoadFileData(const std::string& filePath)
    {
        // @todo Handle error, e.g., exception, if file extension isn't .obj 
        IndexModel mesh;
        std::ifstream ifs{filePath};
        std::string line;

        while (std::getline(ifs, line))
        {
            if (line.size() == 0) { continue; }

            std::istringstream iss{std::move(line)};
            std::vector<std::string> tokens = Helper::Split(line, ' ');

            if (tokens[0].compare("v") == 0)    // Vert pos
            {
                mesh.verts.push_back(Vec3f{stof(tokens[1]),
                    stof(tokens[2]),
                    stof(tokens[3])});
            }
            else if (tokens[0].compare("vt") == 0)  // Tex coord
            {
                mesh.texCoords.push_back(Vec2f{stof(tokens[1]),
                    stof(tokens[2])});
            }
            else if (tokens[0].compare("vn") == 0)  // Normal
            {
                mesh.normals.push_back(Vec3f{stof(tokens[1]),
                    stof(tokens[2]),
                    stof(tokens[3])});
            }
            else if (tokens[0].compare("f") == 0)   // Indices
            {
                for (int i = 1; i < tokens.size(); ++i)
                {
                    std::vector<std::string> faceData = Helper::Split(tokens[i], '/');
                    // OBJ is 1-based index
                    mesh.vertIndices.push_back(stoi(faceData[0]) - 1);
                    if (!faceData[1].empty()) { mesh.uvIndices.push_back(stoi(faceData[1]) - 1); }
                    if (!faceData[2].empty()) { mesh.nIndices.push_back(stoi(faceData[2]) - 1); }
                }
            }
        }

        ifs.close();
        return mesh;
    }
}
