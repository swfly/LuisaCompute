#include <luisa/core/stl/algorithm.h>
#include <luisa/runtime/raster/raster_scene.h>
#include <luisa/core/logging.h>
#include <luisa/backends/ext/raster_cmd.h>
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
luisa::unique_ptr<Command> RasterScene::build() noexcept {
    _check_is_valid();
    std::lock_guard lock{_mtx};
    if (_instance_count == 0) { LUISA_ERROR_WITH_LOCATION(
        "Building acceleration structure without instances."); }
    // collect modifications
    luisa::vector<Modification> modifications;
    modifications.push_back_uninitialized(_modifications.size());
    luisa::transform(_modifications.begin(), _modifications.end(), modifications.begin(),
                     [](auto &&pair) noexcept -> auto && { return std::move(pair.second); });
    _modifications.clear();
    return luisa::make_unique<BuildRasterSceneCommand>(
        handle(), static_cast<uint>(_instance_count),
        std::move(modifications),
        luisa::span{_render_formats},
        _depth_format);
}
luisa::unique_ptr<Command> RasterScene::build(
    luisa::span<const PixelFormat> render_formats,
    DepthFormat depth_format) noexcept {
    std::lock_guard lock{_mtx};
    _render_formats.clear();
    LUISA_ASSERT(render_formats.size() <= 8, "Render format count must be less than 8.");
    _render_formats.push_back_uninitialized(render_formats.size());
    std::memcpy(_render_formats.data(), render_formats.data(), render_formats.size_bytes());
    return build();
}
luisa::unique_ptr<Command> RasterScene::draw(
    float4x4 const &view,
    float4x4 const &projection,
    luisa::span<const ImageView<float>> rtv_texs,
    DepthBuffer const *depth,
    Viewport viewport) const noexcept {
    _check_is_valid();
    LUISA_ASSERT(rtv_texs.size() == _render_formats.size(), "Render target size {} and render format size {} mismatch.", rtv_texs.size(), _render_formats.size());
    {
        auto r = _render_formats.begin();
        for (auto &i : rtv_texs) {
            if (i.format() != *r) [[unlikely]] {
                LUISA_ERROR("Render target format ({} and {}) mismatch.", luisa::to_string(i.format()), luisa::to_string(*r));
            }
            ++r;
        }
    }
    auto dsv_format = depth ? depth->format() : DepthFormat::None;
    LUISA_ASSERT(dsv_format == _depth_format, "Depth format mismatch.");
    auto depth_arg = [&]() {
        if (depth) {
            return Argument::Texture{
                depth->handle(), 0};
        } else {
            return Argument::Texture{
                invalid_resource_handle, 0};
        }
    }();

    luisa::fixed_vector<Argument::Texture, 8> texs;
    texs.push_back_uninitialized(rtv_texs.size());
    luisa::transform(
        rtv_texs.begin(), rtv_texs.end(), texs.begin(),
        [&](ImageView<float> const &t) {
            return Argument::Texture{
                .handle = t.handle(),
                .level = t.level()};
        });

    return luisa::make_unique<DrawRasterSceneCommand>(
        handle(),
        view,
        projection,
        texs,
        depth_arg,
        viewport);
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