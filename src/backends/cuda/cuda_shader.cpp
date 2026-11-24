#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <nvtx3/nvToolsExtCuda.h>

#include <luisa/core/logging.h>

#include "cuda_ptx_version.h"
#include "cuda_shader_printer.h"
#include "cuda_shader.h"
#include "cuda_command_encoder.h"
#include "cuda_error.h"

namespace luisa::compute::cuda {

namespace {

struct ProfiledLaunch {
    CUstream stream;
    CUevent begin;
    CUevent end;
    luisa::string name;
    long long host_ns;
};

struct LaunchTrace {
    std::mutex mutex;
    luisa::vector<ProfiledLaunch> launches;
    FILE *output{nullptr};
};

LaunchTrace &launch_trace() noexcept {
    static LaunchTrace trace;
    return trace;
}

[[nodiscard]] long long trace_host_ns() noexcept {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}

} // namespace

void flush_diagnostic_launch_trace(CUstream stream) noexcept {
    if (std::getenv("LUISA_CUDA_EVENT_TRACE") == nullptr) { return; }
    auto &trace = launch_trace();
    std::scoped_lock lock{trace.mutex};
    auto keep = [&](const ProfiledLaunch &launch) noexcept {
        if (launch.stream != stream || cuEventQuery(launch.end) == CUDA_ERROR_NOT_READY) {
            return true;
        }
        float gpu_ms = 0.f;
        LUISA_CHECK_CUDA(cuEventElapsedTime(&gpu_ms, launch.begin, launch.end));
        std::fprintf(trace.output, "%lld,%f,%s\n", launch.host_ns, gpu_ms, launch.name.c_str());
        LUISA_CHECK_CUDA(cuEventDestroy(launch.begin));
        LUISA_CHECK_CUDA(cuEventDestroy(launch.end));
        return false;
    };
    auto end = std::stable_partition(trace.launches.begin(), trace.launches.end(), keep);
    if (end != trace.launches.end()) {
        trace.launches.erase(end, trace.launches.end());
        std::fflush(trace.output);
    }
}


void CUDAShader::_patch_ptx_version(luisa::vector<std::byte> &ptx) noexcept {
    patch_cuda_ptx_version_bytes(ptx);
}

CUDAShader::CUDAShader(luisa::unique_ptr<CUDAShaderPrinter> printer,
                       luisa::vector<Usage> arg_usages) noexcept
    : _printer{std::move(printer)},
      _argument_usages{std::move(arg_usages)} {}

CUDAShader::~CUDAShader() noexcept = default;

Usage CUDAShader::argument_usage(size_t i) const noexcept {
    LUISA_ASSERT(i < _argument_usages.size(),
                 "Invalid argument index {} for shader with {} argument(s).",
                 i, _argument_usages.size());
    return _argument_usages[i];
}

void CUDAShader::set_name(luisa::string &&name) noexcept {
    std::scoped_lock lock{_name_mutex};
    _name = std::move(name);
}

void CUDAShader::launch(CUDACommandEncoder &encoder,
                        ShaderDispatchCommand *command) const noexcept {

    // check if this is an empty launch
    auto report_empty_launch = [&]() noexcept {
#ifndef NDEBUG
        LUISA_WARNING_WITH_LOCATION(
            "Empty launch detected. "
            "This might be caused by a shader dispatch command with all dispatch sizes set to zero. "
            "The command will be ignored.");
#endif
    };
    if (command->is_indirect()) {
        auto indirect = command->indirect_dispatch();
        if (indirect.max_dispatch_size == 0u) {
            report_empty_launch();
            return;
        }
    } else if (command->is_multiple_dispatch()) {
        auto dispatch_sizes = command->dispatch_sizes();
        if (std::all_of(dispatch_sizes.begin(), dispatch_sizes.end(),
                        [](auto size) noexcept { return any(size == make_uint3(0u)); })) {
            report_empty_launch();
            return;
        }
    } else {
        auto dispatch_size = command->dispatch_size();
        if (any(dispatch_size == make_uint3(0u))) {
            report_empty_launch();
            return;
        }
    }

    auto name = [this] {
        std::scoped_lock lock{_name_mutex};
        return _name;
    }();
    if (!name.empty()) { nvtxRangePushA(name.c_str()); }
    if (const char *path = std::getenv("LUISA_CUDA_EVENT_TRACE")) {
        auto stream = encoder.stream()->handle();
        CUevent begin, end;
        LUISA_CHECK_CUDA(cuEventCreate(&begin, CU_EVENT_DEFAULT));
        LUISA_CHECK_CUDA(cuEventCreate(&end, CU_EVENT_DEFAULT));
        auto host_ns = trace_host_ns();
        LUISA_CHECK_CUDA(cuEventRecord(begin, stream));
        _launch(encoder, command);
        LUISA_CHECK_CUDA(cuEventRecord(end, stream));
        auto &trace = launch_trace();
        std::scoped_lock lock{trace.mutex};
        if (trace.output == nullptr) {
            trace.output = std::fopen(path, "w");
            LUISA_ASSERT(trace.output != nullptr, "Could not open CUDA diagnostic trace file.");
        }
        trace.launches.emplace_back(ProfiledLaunch{
            stream, begin, end, name.empty() ? luisa::string{"<unnamed>"} : name, host_ns});
    } else {
        _launch(encoder, command);
    }
    if (!name.empty()) { nvtxRangePop(); }
}

}// namespace luisa::compute::cuda
