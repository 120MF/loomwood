#include <engine/RmlRenderInterface.hpp>
#include <engine/WindowService.hpp>
#include <log/log.hpp>

#include <entt/entt.hpp>
#include <SDL3/SDL.h>

namespace lw
{

RmlRenderInterface::RmlRenderInterface()
{
    // Premultiplied-alpha blend mode:
    //   dst_rgb   = src_rgb   * 1           + dst_rgb   * (1 - src_alpha)
    //   dst_alpha = src_alpha * 1           + dst_alpha * (1 - src_alpha)
    // Matches the official RmlUi SDL backend. Texture data must be
    // premultiplied before uploading (see premultiplyAlpha()).
    m_blendMode = SDL_ComposeCustomBlendMode(
        SDL_BLENDFACTOR_ONE, SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA, SDL_BLENDOPERATION_ADD,
        SDL_BLENDFACTOR_ONE, SDL_BLENDFACTOR_ONE_MINUS_SRC_ALPHA, SDL_BLENDOPERATION_ADD);
}

// ---------------------------------------------------------------------------
// Geometry
// ---------------------------------------------------------------------------

Rml::CompiledGeometryHandle RmlRenderInterface::CompileGeometry(
    Rml::Span<const Rml::Vertex> vertices,
    Rml::Span<const int>         indices)
{
    CompiledGeometry geom;
    geom.vertices.assign(vertices.begin(), vertices.end());
    geom.indices.assign(indices.begin(), indices.end());

    const auto handle = m_nextHandle++;
    m_geometries.emplace(handle, std::move(geom));
    return handle;
}

void RmlRenderInterface::RenderGeometry(
    Rml::CompiledGeometryHandle handle,
    Rml::Vector2f               translation,
    Rml::TextureHandle          texture)
{
    auto it = m_geometries.find(handle);
    if (it == m_geometries.end())
        return;

    const CompiledGeometry& geom = it->second;
    auto* renderer  = entt::locator<WindowService>::value().renderer();
    auto* sdl_tex   = reinterpret_cast<SDL_Texture*>(texture);
    const int n     = static_cast<int>(geom.vertices.size());

    // Grow staging buffer on demand (never shrinks; amortised allocation).
    if (static_cast<int>(m_stagingVerts.size()) < n)
        m_stagingVerts.resize(n);

    // Copy vertices, apply translation, and convert colour to SDL_FColor.
    // NOTE: RmlUi vertex positions are in LOCAL element space; translation is
    // the element's absolute document position. Using SDL_Vertex copies avoids
    // the viewport-shift approach which clips glyphs with negative local y.
    for (int i = 0; i < n; ++i)
    {
        const auto& src = geom.vertices[i];
        auto& dst       = m_stagingVerts[i];
        dst.position  = { src.position.x + translation.x,
                          src.position.y + translation.y };
        dst.tex_coord = { src.tex_coord.x, src.tex_coord.y };
        dst.color     = { src.colour.red   / 255.0f,
                          src.colour.green / 255.0f,
                          src.colour.blue  / 255.0f,
                          src.colour.alpha / 255.0f };
    }

    SDL_SetRenderDrawBlendMode(renderer, m_blendMode);
    SDL_RenderGeometry(renderer, sdl_tex,
        m_stagingVerts.data(), n,
        geom.indices.data(), static_cast<int>(geom.indices.size()));
}

void RmlRenderInterface::ReleaseGeometry(Rml::CompiledGeometryHandle handle)
{
    m_geometries.erase(handle);
}

// ---------------------------------------------------------------------------
// Textures
// ---------------------------------------------------------------------------

void RmlRenderInterface::premultiplyAlpha(uint8_t* pixels, int count)
{
    // count = number of pixels; each pixel is 4 bytes (RGBA).
    for (int i = 0; i < count; ++i)
    {
        uint8_t* p = pixels + i * 4;
        const uint8_t a = p[3];
        p[0] = static_cast<uint8_t>(static_cast<int>(p[0]) * a / 255);
        p[1] = static_cast<uint8_t>(static_cast<int>(p[1]) * a / 255);
        p[2] = static_cast<uint8_t>(static_cast<int>(p[2]) * a / 255);
    }
}

SDL_Texture* RmlRenderInterface::createTextureFromSurface(SDL_Surface* surface)
{
    auto* renderer = entt::locator<WindowService>::value().renderer();
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surface);
    if (tex)
    {
        SDL_SetTextureBlendMode(tex, m_blendMode);
    }
    return tex;
}

Rml::TextureHandle RmlRenderInterface::LoadTexture(
    Rml::Vector2i&     texture_dimensions,
    const Rml::String& source)
{
    SDL_Surface* surface = SDL_LoadBMP(source.c_str());
    if (!surface)
    {
        get_default_logger()->warn("[RmlUi] LoadTexture failed '{}': {}", source, SDL_GetError());
        return 0;
    }

    // Convert to RGBA32 if needed, then premultiply.
    if (surface->format != SDL_PIXELFORMAT_RGBA32)
    {
        SDL_Surface* conv = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA32);
        SDL_DestroySurface(surface);
        if (!conv) return 0;
        surface = conv;
    }
    premultiplyAlpha(static_cast<uint8_t*>(surface->pixels), surface->w * surface->h);

    texture_dimensions = { surface->w, surface->h };
    SDL_Texture* tex = createTextureFromSurface(surface);
    SDL_DestroySurface(surface);

    if (!tex)
        get_default_logger()->warn("[RmlUi] CreateTexture failed '{}': {}", source, SDL_GetError());
    return reinterpret_cast<Rml::TextureHandle>(tex);
}

Rml::TextureHandle RmlRenderInterface::GenerateTexture(
    Rml::Span<const Rml::byte> source,
    Rml::Vector2i              source_dimensions)
{
    // RmlUi provides raw RGBA8 pixels (e.g. font glyph atlas).
    // We make a writable copy so premultiplyAlpha can modify in-place.
    const int pixel_count = source_dimensions.x * source_dimensions.y;
    std::vector<uint8_t> pixels(source.begin(), source.end());
    premultiplyAlpha(pixels.data(), pixel_count);

    SDL_Surface* surface = SDL_CreateSurfaceFrom(
        source_dimensions.x, source_dimensions.y,
        SDL_PIXELFORMAT_RGBA32,
        pixels.data(),
        source_dimensions.x * 4);
    if (!surface)
        return 0;

    SDL_Texture* tex = createTextureFromSurface(surface);
    SDL_DestroySurface(surface);
    return reinterpret_cast<Rml::TextureHandle>(tex);
}

void RmlRenderInterface::ReleaseTexture(Rml::TextureHandle texture)
{
    if (texture)
        SDL_DestroyTexture(reinterpret_cast<SDL_Texture*>(texture));
}

// ---------------------------------------------------------------------------
// Scissor
// ---------------------------------------------------------------------------

void RmlRenderInterface::EnableScissorRegion(bool enable)
{
    m_scissorEnabled = enable;
    auto* renderer = entt::locator<WindowService>::value().renderer();
    SDL_SetRenderClipRect(renderer, enable ? &m_scissorRegion : nullptr);
}

void RmlRenderInterface::SetScissorRegion(Rml::Rectanglei region)
{
    m_scissorRegion = { region.Left(), region.Top(), region.Width(), region.Height() };
    if (m_scissorEnabled)
    {
        auto* renderer = entt::locator<WindowService>::value().renderer();
        SDL_SetRenderClipRect(renderer, &m_scissorRegion);
    }
}

} // namespace lw

