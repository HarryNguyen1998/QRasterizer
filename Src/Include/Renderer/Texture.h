#include <memory>

// A wrapper that handles texture data, using SDL_Texture
// Do i need an IsEmpty() method, or handle that case in Release is better?
class TextureHandle
{
public:
    bool Acquire();
    void DrawFrame();
    void Draw();
    void Release();

private:
    SDL_Texture* texObj;
};

// 
// Note: 
class TextureManager
{
public:
    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;

    static TextureManager& Instance();
    std::shared_ptr<TextureHandle> Load(const std::string& filePath);

private:
    std::unordered_map<std::string, std::shared_ptr<TextureHandle>> loadedTexs;
};

