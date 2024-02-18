#pragma once

#include <luisa/runtime/rhi/device_interface.h>
#include <luisa/runtime/raster/raster_state.h>
namespace lc::validation {
class RasterExtImpl;
}// namespace lc::validation
namespace luisa::compute {
template<typename... Args>
class RasterShader;
template<typename VertCallable, typename PixelCallable>
class RasterKernel;
class MeshFormat;
class RasterScene;
class DepthBuffer;

class RasterExt : public DeviceExtension {
    friend class lc::validation::RasterExtImpl;
    friend class DepthBuffer;
    friend class RasterScene;
    template<typename... Args>
    friend class RasterShader;
protected:
    ~RasterExt() noexcept = default;
    [[nodiscard]] virtual DeviceInterface *device() noexcept = 0;
    [[nodiscard]] virtual ResourceCreationInfo create_raster_scene() noexcept = 0;
    [[nodiscard]] virtual void destroy_raster_scene(uint64_t handle) noexcept = 0;
    // shader
    [[nodiscard]] virtual ResourceCreationInfo create_raster_shader(
        const MeshFormat &mesh_format,
        Function vert,
        Function pixel,
        const ShaderOption &shader_option) noexcept = 0;

    [[nodiscard]] virtual ResourceCreationInfo load_raster_shader(
        const MeshFormat &mesh_format,
        luisa::span<Type const *const> types,
        luisa::string_view ser_path) noexcept = 0;
    virtual void destroy_raster_shader(uint64_t handle) noexcept = 0;

    // depth buffer
    [[nodiscard]] virtual ResourceCreationInfo create_depth_buffer(DepthFormat format, uint width, uint height) noexcept = 0;
    virtual void destroy_depth_buffer(uint64_t handle) noexcept = 0;
public:
    static constexpr luisa::string_view name = "RasterExt";

    template<typename VertCallable, typename PixelCallable>
    [[nodiscard]] auto compile(const MeshFormat &mesh_format,
                               RasterKernel<VertCallable, PixelCallable> const &kernel,
                               const ShaderOption &option = {}) noexcept;
    template<typename... Args>
    [[nodiscard]] RasterShader<Args...> load(const MeshFormat &mesh_format,
                                             luisa::string_view file_path) noexcept;
    RasterScene create_scene(
        luisa::span<const PixelFormat> render_formats,
        DepthFormat depth_format) noexcept;
};
}// namespace luisa::compute
