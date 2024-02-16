#pragma once

#include <luisa/runtime/shader.h>
#include <luisa/runtime/raster/raster_state.h>
#include <luisa/runtime/raster/depth_buffer.h>
#include <luisa/backends/ext/raster_ext_interface.h>

namespace luisa::compute {

class Accel;
class BindlessArray;

namespace detail {

template<typename T>
struct PixelDst : public std::false_type {};

template<typename T>
struct PixelDst<Image<T>> : public std::true_type {
    static ShaderDispatchCommandBase::Argument::Texture get(Image<T> const &v) noexcept {
        return {v.handle(), 0};
    }
};

template<typename T>
struct PixelDst<ImageView<T>> : public std::true_type {
    static ShaderDispatchCommandBase::Argument::Texture get(ImageView<T> const &v) noexcept {
        return {v.handle(), v.level()};
    }
};

template<typename T, typename... Args>
static constexpr bool LegalDst() noexcept {
    constexpr bool r = PixelDst<T>::value;
    if constexpr (sizeof...(Args) == 0) {
        return r;
    } else if constexpr (!r) {
        return false;
    } else {
        return LegalDst<Args...>();
    }
}

}// namespace detail
namespace detail {
LC_RUNTIME_API void rastershader_check_vertex_func(Function func) noexcept;
LC_RUNTIME_API void rastershader_check_pixel_func(Function func) noexcept;
}// namespace detail

// TODO: @Maxwell fix this please
template<typename... Args>
class RasterShader : public Resource {

private:
    friend class Device;
    RasterExt *_raster_ext{};
    luisa::vector<Function::Binding> _bindings;
    size_t _uniform_size{};
#ifndef NDEBUG
    MeshFormat _mesh_format;
#endif
    // JIT Shader
    // clang-format off

    RasterShader(DeviceInterface *device,
                 RasterExt* raster_ext,
                 const MeshFormat &mesh_format,
                 Function vert,
                 Function pixel,
                 const ShaderOption &option)noexcept
        : Resource(
              device,
              Tag::RASTER_SHADER,
              raster_ext->create_raster_shader(
                  mesh_format,
//                  raster_state,
//                  rtv_format,
//                  dsv_format,
                  vert,
                  pixel,
                  option)),
                  _raster_ext{raster_ext},
         _uniform_size{ShaderDispatchCmdEncoder::compute_uniform_size(
                detail::shader_argument_types<Args...>())}
#ifndef NDEBUG
        ,_mesh_format(mesh_format)
#endif
        {
#ifndef NDEBUG
            detail::rastershader_check_vertex_func(vert);
            detail::rastershader_check_pixel_func(pixel);
#endif
            auto vert_bindings = vert.bound_arguments().subspan(1);
            auto pixel_bindings = pixel.bound_arguments().subspan(1);
            _bindings.reserve(vert_bindings.size() + pixel_bindings.size());
            for(auto&& i : vert_bindings){
                _bindings.emplace_back(i);
            }
            for(auto&& i : pixel_bindings){
                _bindings.emplace_back(i);
            }
        }
    // AOT Shader
    RasterShader(
        DeviceInterface *device,
        RasterExt* raster_ext,
        const MeshFormat &mesh_format,
        string_view file_path)noexcept
        : Resource(
              device,
              Tag::RASTER_SHADER,
              // TODO
              raster_ext->load_raster_shader(
                mesh_format,
                detail::shader_argument_types<Args...>(),
                file_path)),
            _raster_ext{raster_ext},
            _uniform_size{ShaderDispatchCmdEncoder::compute_uniform_size(
                detail::shader_argument_types<Args...>())}
#ifndef NDEBUG
        ,_mesh_format(mesh_format)
#endif
        {
        }
    // clang-format on

public:
    RasterShader() noexcept = default;
    RasterShader(RasterShader &&) noexcept = default;
    RasterShader(RasterShader const &) noexcept = delete;
    RasterShader &operator=(RasterShader &&rhs) noexcept {
        _move_from(std::move(rhs));
        return *this;
    }
    RasterShader &operator=(RasterShader const &) noexcept = delete;
    ~RasterShader() noexcept override {
        if (*this) { _raster_ext->destroy_raster_shader(handle()); }
    }
    using Resource::operator bool;
};

}// namespace luisa::compute
