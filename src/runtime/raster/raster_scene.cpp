#include <luisa/runtime/raster/raster_scene.h>
#include <luisa/core/logging.h>
namespace luisa::compute {
RasterScene::RasterScene(
    DeviceInterface *device,
    luisa::span<const PixelFormat> render_formats,
    DepthFormat depth_format) noexcept
    : Resource(device, Tag::RASTER_SCENE, static_cast<RasterExt *>(device->extension(RasterExt::name))->create_raster_scene()) {
    LUISA_ASSERT(render_formats.size() <= 8, "Render format count must be less than 8.");
    _render_formats.push_back_uninitialized(render_formats.size());
    std::memcpy(_render_formats.data(), render_formats.data(), render_formats.size_bytes());
}
RasterScene::~RasterScene() noexcept {
    if (*this) {
        static_cast<RasterExt *>(device()->extension(RasterExt::name))->destroy_raster_scene(handle());
    }
}
RasterScene::RasterScene(RasterScene &&rhs) noexcept
    : Resource(std::move(rhs)),
      _render_formats(std::move(rhs._render_formats)),
      _depth_format(rhs._depth_format),
      _modifications(std::move(rhs._modifications)),
      _instance_count(rhs._instance_count) {
    rhs._instance_count = 0;
}
}// namespace luisa::compute