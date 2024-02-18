#pragma once
#include <luisa/backends/ext/raster_ext_interface.h>
#include <luisa/dsl/raster/raster_kernel.h>
#include <luisa/runtime/raster/raster_scene.h>
namespace luisa::compute {
template<typename VertCallable, typename PixelCallable>
auto RasterExt::compile(const MeshFormat &mesh_format,
                        RasterKernel<VertCallable, PixelCallable> const &kernel,
                        const ShaderOption &option) noexcept {
    using Shader = typename RasterKernel<VertCallable, PixelCallable>::RasterShaderType;
    return Shader{
        device(),
        this,
        mesh_format,
        kernel.vert(),
        kernel.pixel(),
        option};
}
template<typename... Args>
RasterShader<Args...> RasterExt::load(const MeshFormat &mesh_format,
                                      luisa::string_view file_path) noexcept {
    return RasterShader<Args...>{
        device(),
        this,
        mesh_format,
        file_path};
}
inline RasterScene RasterExt::create_scene(
    luisa::span<const PixelFormat> render_formats,
    DepthFormat depth_format) noexcept {
    return RasterScene{
        device(), render_formats,
        depth_format};
}
}// namespace luisa::compute