#pragma once

#include <luisa/runtime/rhi/command.h>
#include <luisa/runtime/raster/raster_state.h>
#include <luisa/backends/ext/registry.h>
#include <luisa/runtime/rhi/command_encoder.h>

namespace luisa::compute {
struct VertexBufferView {
    uint64_t handle;
    uint64_t offset;
    uint size;
    uint stride;
};
enum class IndexFormat {
    UInt16,
    UInt32,
};
struct IndexBufferView {
    uint64_t handle;
    uint size;
    IndexFormat format;
};
class LC_RUNTIME_API BuildRasterSceneCommand final : public CustomCommand {
    friend lc::validation::Stream;
public:
    struct Modification {
        luisa::fixed_vector<VertexBufferView, 2> vertex_buffers;
        luisa::variant<IndexBufferView, uint> index_buffer;
        ShaderDispatchCmdEncoder encoder;
        uint instance;
        RasterState state;
        uint flag;
        static constexpr auto flag_instance = 1u << 0u;
        static constexpr auto flag_shader = 1u << 1u;
        static constexpr auto flag_index_buffer = 1u << 2u;
        static constexpr auto flag_vertex_buffer = 1u << 3u;
        static constexpr auto flag_state = 1u << 4u;
        static constexpr auto flag_all = ~0u;
    };

private:
    uint64_t _handle;
    uint32_t _element_count;
    luisa::vector<Modification> _modifications;
    luisa::fixed_vector<PixelFormat, 8> _render_formats;
    DepthFormat _depth_format;

public:
    [[nodiscard]] auto handle() const noexcept { return _handle; }
    [[nodiscard]] auto element_count() const noexcept { return _element_count; }
    [[nodiscard]] auto modifications() const noexcept { return luisa::span{_modifications}; }
    [[nodiscard]] auto render_formats() const noexcept { return luisa::span{_render_formats}; }
    [[nodiscard]] auto depth_format() const noexcept { return _depth_format; }
    BuildRasterSceneCommand(
        uint64_t handle, uint32_t element_count,
        luisa::vector<Modification> modifications,
        luisa::span<PixelFormat const> render_formats,
        DepthFormat depth_format) noexcept
        : _handle(handle),
          _element_count(element_count),
          _modifications(std::move(modifications)),
          _depth_format(depth_format) {
        _render_formats.push_back_uninitialized(render_formats.size());
        std::memcpy(_render_formats.data(), render_formats.data(), render_formats.size_bytes());
    }
    BuildRasterSceneCommand(BuildRasterSceneCommand const &) = delete;
    BuildRasterSceneCommand &operator=(BuildRasterSceneCommand const &) = delete;
    BuildRasterSceneCommand &operator=(BuildRasterSceneCommand &&) = default;
    BuildRasterSceneCommand(BuildRasterSceneCommand &&) = default;
    [[nodiscard]] uint64_t uuid() const noexcept override { return to_underlying(CustomCommandUUID::RASTER_BUILD_SCENE); }
    LUISA_MAKE_COMMAND_COMMON(StreamTag::GRAPHICS)
};

class LC_RUNTIME_API DrawRasterSceneCommand final : public CustomCommand {
    friend lc::validation::Stream;
private:
    uint64_t _scene_handle;
    float4x4 _view;
    float4x4 _projection;
    luisa::fixed_vector<Argument::Texture, 8> _rtv_texs;
    Argument::Texture _dsv_tex;
    Viewport _viewport;

public:
    DrawRasterSceneCommand(
        uint64_t scene_handle,
        float4x4 const &view,
        float4x4 const &projection,
        luisa::span<Argument::Texture const> rtv_texs,
        Argument::Texture dsv_tex,
        Viewport viewport) noexcept
        : _scene_handle(scene_handle),
          _view(view),
          _projection(projection),
          _dsv_tex(dsv_tex),
          _viewport(viewport) {
    }
    [[nodiscard]] auto scene_handle() const noexcept { return _scene_handle; }
    [[nodiscard]] auto view() const noexcept { return _view; }
    [[nodiscard]] auto projection() const noexcept { return _projection; }
    [[nodiscard]] auto rtv_texs() const noexcept { return luisa::span{_rtv_texs}; }
    [[nodiscard]] auto const &dsv_tex() const noexcept { return _dsv_tex; }
    [[nodiscard]] auto viewport() const noexcept { return _viewport; }
    [[nodiscard]] uint64_t uuid() const noexcept override { return to_underlying(CustomCommandUUID::RASTER_DRAW_SCENE); }
    LUISA_MAKE_COMMAND_COMMON(StreamTag::GRAPHICS)
};

class ClearDepthCommand final : public CustomCommand {
    friend lc::validation::Stream;
    uint64_t _handle;
    float _value;

public:
    explicit ClearDepthCommand(uint64_t handle, float value) noexcept
        : _handle{handle}, _value(value) {
    }
    [[nodiscard]] auto handle() const noexcept { return _handle; }
    [[nodiscard]] auto value() const noexcept { return _value; }
    [[nodiscard]] uint64_t uuid() const noexcept override { return to_underlying(CustomCommandUUID::RASTER_CLEAR_DEPTH); }

    LUISA_MAKE_COMMAND_COMMON(StreamTag::GRAPHICS)
};

}// namespace luisa::compute
