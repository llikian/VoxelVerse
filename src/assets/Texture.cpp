/***************************************************************************************************
 * @file  Texture.cpp
 * @brief Implementation of the Texture class
 **************************************************************************************************/

#include "assets/Texture.hpp"

#include "assets/AssetManager.hpp"
#include "glad/glad.h"

/**
 * @brief Returns the internal_format and format corresponding to a specific amount of channels.
 * @param channels_amount The amount of channels.
 * @param srgb Whether the internal format should be SRGB.
 * @param internal_format The internal format corresponding to the amount of channels. Uses unsigned bytes as the type.
 * @param format The format corresponding to the amount of channels.
 */
static void get_formats_from_channels_amount(unsigned int channels_amount,
                                             bool srgb,
                                             unsigned int& internal_format,
                                             unsigned int& format) {
    switch(channels_amount) {
        case 3:
            internal_format = srgb ? GL_SRGB8 : GL_RGB8;
            format = GL_RGB;
            break;
        case 4:
            internal_format = srgb ? GL_SRGB8_ALPHA8 : GL_RGBA8;
            format = GL_RGBA;
            break;
        case 1:
            internal_format = GL_R8;
            format = GL_RED;
            break;
        case 2:
            internal_format = GL_RG8;
            format = GL_RG;
            break;
        default: throw std::runtime_error("Unexpected amount of channels in image:" + std::to_string(channels_amount));
    }
}

Texture::Texture() : id(0), b_has_transparency(false) { }

Texture::Texture(const Texture& texture) : id(texture.id), b_has_transparency(texture.has_transparency()) { }

Texture& Texture::operator=(const Texture& texture) {
    id = texture.id;
    b_has_transparency = texture.b_has_transparency;
    return *this;
}

void Texture::init() {
    glGenTextures(1, &id);
}

void Texture::free() {
    if(id == 0) { return; }
    glDeleteTextures(1, &id);
    id = 0;
}

void Texture::create(unsigned int internal_format,
                     unsigned int format,
                     unsigned int type,
                     unsigned int width,
                     unsigned int height,
                     const void* data) {
    init();
    bind();

    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, type, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    b_has_transparency = format == GL_RGBA
                         || format == GL_BGRA
                         || format == GL_RGBA_INTEGER
                         || format == GL_BGRA_INTEGER;
}

void Texture::create(const Image& image, bool srgb) {
    unsigned int internal_format;
    unsigned int format;
    get_formats_from_channels_amount(image.get_channels_amount(), srgb, internal_format, format);

    create(internal_format, format, GL_UNSIGNED_BYTE, image.get_width(), image.get_height(), image.get_data());
}

void Texture::create(const std::filesystem::path& path, bool flip_vertically, bool srgb) {
    create(Image(path, flip_vertically), srgb);
}

void Texture::create(const vec3& color) {
    create(GL_RGB32F, GL_RGB, GL_FLOAT, 1, 1, &color.x);
}

void Texture::create(unsigned char r, unsigned char g, unsigned char b) {
    unsigned char color[3] { r, g, b };
    create(GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE, 1, 1, color);
}

void Texture::create(const tinygltf::Image& image, const tinygltf::Sampler& sampler, bool srgb) {
    if(image.uri.empty()) { throw std::runtime_error("Unhandled case: tinygltf::Image without URI."); }

    const Texture* asset_manager_texture = AssetManager::get_texture_ptr(image.uri);
    if(asset_manager_texture != nullptr) {
        id = asset_manager_texture->get_id();
        b_has_transparency = asset_manager_texture->has_transparency();
        return;
    }

    unsigned int internal_format;
    unsigned int format;
    get_formats_from_channels_amount(image.component, srgb, internal_format, format);

    create(internal_format, format, image.pixel_type, image.width, image.height, image.image.data());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, sampler.wrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, sampler.wrapT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, sampler.minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, sampler.magFilter);

    AssetManager::add_texture(image.uri, *this);
}

void Texture::bind(unsigned int texture_unit) const {
    glActiveTexture(GL_TEXTURE0 + texture_unit);
    glBindTexture(GL_TEXTURE_2D, id);
}

bool Texture::is_default_texture() const {
    return id == 0;
}

unsigned int Texture::get_id() const {
    return id;
}

bool Texture::has_transparency() const {
    return b_has_transparency;
}
