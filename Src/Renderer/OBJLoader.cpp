#include <cassert>
#include <fstream>
#include <sstream> // std::stringstream

#include "Renderer/OBJLoader.h"
#include "Renderer/Model.h"
#include "Utils/Helper.h"

namespace OBJ
{
    Model LoadFileData(const std::string& filePath)
    {
        Model mesh;
        std::ifstream ifs{filePath};
        if (!ifs) { assert(0 == 1 && "Uh oh, file can't be opened."); }
        std::string line;

        while (std::getline(ifs, line))
        {
            if (line.empty()) { continue; }
            ParseLine(&mesh, line);
        }

        ifs.close();
        return mesh;
    }

    void ParseLine(Model *outMesh, std::string line)
    {
        std::istringstream iss{std::move(line)};
        std::vector<std::string> tokens;
        std::string token;
        while (iss >> token)
            tokens.push_back(token);
        assert(!tokens.empty());

        if (tokens[0].compare("v") == 0)    // Vert pos
            outMesh->verts.push_back(Vec3f{stof(tokens[1]), stof(tokens[2]), stof(tokens[3])});
        else if (tokens[0].compare("vt") == 0)  // Tex coord
            outMesh->texCoords.push_back(Vec2f{stof(tokens[1]), stof(tokens[2])});
        else if (tokens[0].compare("vn") == 0)  // Normal
            outMesh->normals.push_back(Vec3f{stof(tokens[1]), stof(tokens[2]), stof(tokens[3])});
        else if (tokens[0].compare("f") == 0)   // Indices
        {
            for (int i = 1; i < tokens.size(); ++i)
            {
                iss.clear();
                iss.str(tokens[i]);
                std::vector<std::string> faceData;
                std::string index;
                while (std::getline(iss, index, '/'))
                    faceData.push_back(index);
                assert(!faceData.empty());

                // OBJ index is 1-based
                outMesh->vertIndices.push_back(stoi(faceData[0]) - 1);

                // texture coordinate but no normal index
                if (faceData.size() == 2)
                    outMesh->uvIndices.push_back(stoi(faceData[1]) - 1);

                // Either all of them, or normal index but no texture coordinate
                if (faceData.size() == 3)
                {
                    if (!faceData[1].empty())
                        outMesh->uvIndices.push_back(stoi(faceData[1]) - 1);
                    outMesh->nIndices.push_back(stoi(faceData[2]) - 1);
                }
                
            }
        }
    }

}
