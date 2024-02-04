set(LUISA_COMPUTE_XIR_SOURCE
        node.cpp)

add_library(luisa-compute-xir SHARED
        ${LUISA_COMPUTE_XIR_SOURCE})
target_link_libraries(luisa-compute-xir
        PRIVATE luisa-compute-ast)
target_compile_definitions(luisa-compute-xir
        PRIVATE LC_XIR_EXPORT_DLL=1)

set_target_properties(luisa-compute-xir PROPERTIES
        OUTPUT_NAME "lc-xir"
        UNITY_BUILD ${LUISA_COMPUTE_ENABLE_UNITY_BUILD})

luisa_compute_install(xir SOURCES ${LUISA_COMPUTE_XIR_SOURCE})
