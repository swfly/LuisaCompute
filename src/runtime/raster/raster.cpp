#include <luisa/runtime/raster/raster_shader.h>
#include <luisa/runtime/depth_format.h>
#include <luisa/runtime/raster/depth_buffer.h>
#include <luisa/runtime/raster/raster_scene.h>
#include <luisa/runtime/rtx/accel.h>
#include <luisa/runtime/bindless_array.h>
#include <luisa/core/logging.h>

namespace luisa::compute {

// see definition in rtx/accel.cpp

#ifndef NDEBUG
namespace detail {
bool rastershader_rettype_is_float4(Type const *t) noexcept {
    return (t->is_vector() && t->dimension() == 4 && t->element()->tag() == Type::Tag::FLOAT32);
};
void rastershader_check_vertex_func(Function func) noexcept {
    // rastershader_check_func(func);
    auto ret_type = func.return_type();

    if (rastershader_rettype_is_float4(ret_type))
        return;
    if (ret_type->is_structure() && ret_type->members().size() >= 1 && rastershader_rettype_is_float4(ret_type->members()[0])) {
        if (ret_type->members().size() > 16) {
            LUISA_ERROR("Vertex shader return type's structure element count need less than 16!");
        }
        for (auto &&i : ret_type->members()) {
            if (!(i->is_vector() || i->is_scalar()))
                LUISA_ERROR("Vertex shader return type can only contain scalar and vector type!");
        }
        return;
    }
    LUISA_ERROR("First element of vertex shader's return type must be float4!");
}
void rastershader_check_pixel_func(Function func) noexcept {
    // rastershader_check_func(func);
    auto ret_type = func.return_type();
    if (rastershader_rettype_is_float4(ret_type)) {
        return;
    }
    if (ret_type->is_structure() && ret_type->members().size() >= 1) {
        if (ret_type->members().size() > 8) {
            LUISA_ERROR("Pixel shader return type's structure element count need less than 8!");
        }
        for (auto &&i : ret_type->members()) {
            if (!(i->is_vector() || i->is_scalar()))
                LUISA_ERROR("Pixel shader return type can only contain scalar and vector type!");
        }
        return;
    }

    LUISA_ERROR("Illegal pixel shader return type!");
}
}// namespace detail
#endif

}// namespace luisa::compute
