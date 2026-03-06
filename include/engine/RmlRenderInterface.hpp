#ifndef LOOMWOOD_RML_RENDER_INTERFACE_HPP
#define LOOMWOOD_RML_RENDER_INTERFACE_HPP

#include <RmlUi/Core/RenderInterface.h>
#include <SDL3/SDL.h>
#include <unordered_map>
#include <vector>
#include <memory>

namespace lw
{

class RmlRenderInterface final : public Rml::RenderInterface
{
public:
    RmlRenderInterface();

    // --- Geometry ---
    Rml::CompiledGeometryHandle CompileGeometry(
        Rml::Span<const Rml::Vertex> vertices,
        Rml::Span<const int>         indices) override;

    void RenderGeometry(
        Rml::CompiledGeometryHandle handle,
        Rml::Vector2f               translation,
        Rml::TextureHandle          texture) override;

    void ReleaseGeometry(Rml::CompiledGeometryHandle handle) override;

    // --- Textures ---
    Rml::TextureHandle LoadTexture(
        Rml::Vector2i&     texture_dimensions,
        const Rml::String& source) override;

    Rml::TextureHandle GenerateTexture(
        Rml::Span<const Rml::byte> source,
        Rml::Vector2i              source_dimensions) override;

    void ReleaseTexture(Rml::TextureHandle texture) override;

    // --- Scissor ---
    void EnableScissorRegion(bool enable) override;
    void SetScissorRegion(Rml::Rectanglei region) override;

private:
    // Converts RGBA pixel data in-place to premultiplied alpha.
    static void premultiplyAlpha(uint8_t* pixels, int count);

    SDL_Texture* createTextureFromSurface(SDL_Surface* surface);

    struct CompiledGeometry
    {
        std::vector<Rml::Vertex> vertices;
        std::vector<int>         indices;
    };

    std::unordered_map<Rml::CompiledGeometryHandle, CompiledGeometry> m_geometries;
    Rml::CompiledGeometryHandle m_nextHandle = 1;

    // Reusable per-frame vertex staging buffer.
    std::vector<SDL_Vertex> m_stagingVerts;

    // Premultiplied-alpha blend mode (matches official RmlUi SDL backend).
    SDL_BlendMode m_blendMode = SDL_BLENDMODE_BLEND;

    bool      m_scissorEnabled = false;
    SDL_Rect  m_scissorRegion  = {};
};

} // namespace lw

#endif // LOOMWOOD_RML_RENDER_INTERFACE_HPP
