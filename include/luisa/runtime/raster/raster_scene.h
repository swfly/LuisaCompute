#pragma once
#include <luisa/backends/ext/raster_cmd.h>
#include <luisa/backends/ext/raster_ext_interface.h>
#include <luisa/runtime/buffer.h>
#include <luisa/core/spin_mutex.h>
#include <luisa/runtime/raster/raster_state.h>
namespace lc::validation {
class Stream;
}// namespace lc::validation
namespace luisa::compute {
namespace detail {
template<typename T>
    requires(is_buffer_view_v<T>)
VertexBufferView make_vbv(T const &buffer_view) noexcept {
    return VertexBufferView{
        .handle = buffer_view.handle(),
        .offset = buffer_view.offset_bytes(),
        .size = buffer_view.size_bytes(),
        .stride = buffer_view.stride(),
    };
}

template<typename T>
    requires(is_buffer_v<T>)
VertexBufferView make_vbv(T const &buffer) noexcept {
    return VertexBufferView{
        .handle = buffer.handle(),
        .offset = 0,
        .size = buffer.size_bytes(),
        .stride = buffer.stride(),
    };
}
struct RasterMesh {
    friend class lc::validation::Stream;
    luisa::fixed_vector<VertexBufferView, 2> _vertex_buffers{};
    luisa::variant<BufferView<uint>, uint> _index_buffer;
    RasterState _state;

    template<typename T>
    RasterMesh(
        luisa::span<BufferView<T>> vertex_buffers,
        BufferView<uint> index_buffer) noexcept
        : _index_buffer(index_buffer) {
        _vertex_buffers.reserve(vertex_buffers.size());
        for (auto &i : vertex_buffers) {
            _vertex_buffers.emplace_back(detail::make_vbv(i));
        }
    }
    RasterMesh() noexcept = default;
    RasterMesh(RasterMesh &&) noexcept = default;
    RasterMesh(RasterMesh const &) noexcept = delete;
    RasterMesh &operator=(RasterMesh &&) noexcept = default;
    RasterMesh &operator=(RasterMesh const &) noexcept = delete;
    template<typename T>
    RasterMesh(
        luisa::span<BufferView<T>> vertex_buffers,
        uint vertex_count) noexcept
        : _index_buffer(vertex_count) {
        _vertex_buffers.reserve(vertex_buffers.size());
        for (auto &i : vertex_buffers) {
            _vertex_buffers.emplace_back(detail::make_vbv(i));
        }
    }
};
}// namespace detail
class LC_RUNTIME_API RasterScene : public Resource {
public:
    using Modification = BuildRasterSceneCommand::Modification;
private:
    luisa::fixed_vector<PixelFormat, 8> _render_formats;
    DepthFormat _depth_format;
    luisa::unordered_map<size_t, Modification> _modifications;
    mutable luisa::spin_mutex _mtx;
    size_t _instance_count{};
    bool _dirty{true};

public:
    using Resource::operator bool;
    RasterScene(
        DeviceInterface *device,
        luisa::span<PixelFormat> render_formats,
        DepthFormat depth_format) noexcept;
    RasterScene(RasterScene &&) noexcept;
    RasterScene(RasterScene const &) noexcept = delete;
    RasterScene &operator=(RasterScene &&rhs) noexcept {
        _move_from(std::move(rhs));
        return *this;
    }
    RasterScene &operator=(RasterScene const &) noexcept = delete;
    ~RasterScene() noexcept;
};
}// namespace luisa::compute