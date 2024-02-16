#pragma once

#include <luisa/runtime/rhi/command.h>
#include <luisa/runtime/raster/raster_state.h>
#include <luisa/backends/ext/registry.h>

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
public:
    struct Modification {
        luisa::fixed_vector<VertexBufferView, 2> vertex_buffers;
        luisa::variant<IndexBufferView, uint> index_buffer;
        RasterState state;
        uint64_t shader_handle;
        uint instance;
        static constexpr auto flag_instance = 1u << 0u;
        static constexpr auto flag_shader = 1u << 1u;
        static constexpr auto flag_index_buffer = 1u << 2u;
        static constexpr auto flag_vertex_buffer = 1u << 3u;
        static constexpr auto flag_state = 1u << 4u;
    };

private:
    uint64_t _handle;
    uint32_t _element_count;
    luisa::vector<Modification> _modifications;

public:
    [[nodiscard]] auto handle() const noexcept { return _handle; }
    [[nodiscard]] auto element_count() const noexcept { return _element_count; }
    [[nodiscard]] auto modifications() const noexcept { return luisa::span{_modifications}; }
    BuildRasterSceneCommand(
        uint64_t handle, uint32_t element_count,
        luisa::vector<Modification> modifications) noexcept
        : _handle(handle),
          _element_count(element_count),
          _modifications(std::move(modifications)) {}
    [[nodiscard]] uint64_t uuid() const noexcept override { return to_underlying(CustomCommandUUID::RASTER_BUILD_SCENE); }
    LUISA_MAKE_COMMAND_COMMON(StreamTag::GRAPHICS)
};

class LC_RUNTIME_API DrawRasterSceneCommand final : public CustomCommand, public ShaderDispatchCommandBase {
    //     friend lc::validation::Stream;

    // private:
    //     std::array<Argument::Texture, 8u> _rtv_texs;
    //     size_t _rtv_count;
    //     Argument::Texture _dsv_tex;
    //     luisa::vector<RasterMesh> _scene;
    //     Viewport _viewport;
    //     RasterState _raster_state;

    // public:
    //     DrawRasterSceneCommand(uint64_t shader_handle,
    //                            luisa::vector<std::byte> &&argument_buffer,
    //                            size_t argument_count,
    //                            std::array<Argument::Texture, 8u> rtv_textures,
    //                            size_t rtv_count,
    //                            Argument::Texture dsv_texture,
    //                            luisa::vector<RasterMesh> &&scene,
    //                            Viewport viewport,
    //                            const RasterState &raster_state) noexcept
    //         : ShaderDispatchCommandBase{
    //               shader_handle, std::move(argument_buffer), argument_count},
    //           _rtv_texs{rtv_textures}, _rtv_count{rtv_count}, _dsv_tex{dsv_texture}, _scene{std::move(scene)}, _viewport{viewport}, _raster_state{raster_state} {
    //     }

    // public:
    //     DrawRasterSceneCommand(DrawRasterSceneCommand const &) noexcept = delete;
    //     DrawRasterSceneCommand(DrawRasterSceneCommand &&) noexcept = default;
    //     ~DrawRasterSceneCommand() noexcept override = default;
    //     [[nodiscard]] auto rtv_texs() const noexcept { return luisa::span{_rtv_texs.data(), _rtv_count}; }
    //     [[nodiscard]] auto const &dsv_tex() const noexcept { return _dsv_tex; }
    //     [[nodiscard]] auto const &raster_state() const noexcept { return _raster_state; }
    //     [[nodiscard]] auto scene() const noexcept {
    //         return luisa::span{_scene};
    //     }
    //     [[nodiscard]] auto viewport() const noexcept { return _viewport; }
    //     [[nodiscard]] uint64_t uuid() const noexcept override { return to_underlying(CustomCommandUUID::RASTER_DRAW_SCENE); }
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
