#pragma once

#include <luisa/ast/function.h>
#include <luisa/runtime/rhi/command.h>
#include <luisa/runtime/raster/viewport.h>

namespace luisa::compute {

class LC_RUNTIME_API ShaderDispatchCmdEncoder {

public:
    using Argument = luisa::compute::Argument;

protected:
    uint64_t _handle{~0ull};
    size_t _argument_count{0};
    size_t _argument_idx{0};
    luisa::vector<std::byte> _argument_buffer;
    [[nodiscard]] Argument &_create_argument() noexcept;

public:
    ShaderDispatchCmdEncoder() noexcept = default;
    explicit ShaderDispatchCmdEncoder(uint64_t handle,
                                      size_t arg_count,
                                      size_t uniform_size) noexcept;
    ShaderDispatchCmdEncoder(ShaderDispatchCmdEncoder const &) = delete;
    ShaderDispatchCmdEncoder(ShaderDispatchCmdEncoder &&rhs) noexcept;
    ShaderDispatchCmdEncoder &operator=(ShaderDispatchCmdEncoder const &) = delete;
    ShaderDispatchCmdEncoder &operator=(ShaderDispatchCmdEncoder &&rhs) noexcept {
        if (this == &rhs) [[unlikely]]
            return *this;
        this->~ShaderDispatchCmdEncoder();
        new (std::launder(this)) ShaderDispatchCmdEncoder{std::move(rhs)};
        return *this;
    }
    [[nodiscard]] static size_t compute_uniform_size(luisa::span<const Variable> arguments) noexcept;
    [[nodiscard]] static size_t compute_uniform_size(luisa::span<const Type *const> arg_types) noexcept;

    [[nodiscard]] auto handle() const noexcept { return _handle; }
    [[nodiscard]] auto argument_count() const noexcept { return _argument_count; }
    [[nodiscard]] auto argument_idx() const noexcept { return _argument_idx; }
    [[nodiscard]] luisa::span<std::byte const> argument_buffer() const noexcept { return _argument_buffer; }
    virtual ~ShaderDispatchCmdEncoder() noexcept = default;
    void encode_buffer(uint64_t handle, size_t offset, size_t size) noexcept;
    void encode_texture(uint64_t handle, uint32_t level) noexcept;
    void encode_uniform(const void *data, size_t size) noexcept;
    void encode_bindless_array(uint64_t handle) noexcept;
    void encode_accel(uint64_t handle) noexcept;
};

class LC_RUNTIME_API ComputeDispatchCmdEncoder final : public ShaderDispatchCmdEncoder {

private:
    luisa::variant<uint3, IndirectDispatchArg, luisa::vector<uint3>> _dispatch_size;

public:
    ComputeDispatchCmdEncoder(ComputeDispatchCmdEncoder const &) = delete;
    ComputeDispatchCmdEncoder(ComputeDispatchCmdEncoder &&rhs) noexcept = default;
    ComputeDispatchCmdEncoder &operator=(ComputeDispatchCmdEncoder const &) = delete;
    ComputeDispatchCmdEncoder &operator=(ComputeDispatchCmdEncoder &&rhs) noexcept = default;
    explicit ComputeDispatchCmdEncoder(uint64_t handle, size_t arg_count, size_t uniform_size) noexcept;
    ~ComputeDispatchCmdEncoder() noexcept = default;
    void set_dispatch_size(uint3 launch_size) noexcept;
    void set_dispatch_size(IndirectDispatchArg indirect_arg) noexcept;
    void set_dispatch_sizes(luisa::span<const uint3> sizes) noexcept;

    luisa::unique_ptr<ShaderDispatchCommand> build() && noexcept;
};

}// namespace luisa::compute
