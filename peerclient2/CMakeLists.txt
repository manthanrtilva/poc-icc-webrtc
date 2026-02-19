cmake_minimum_required(VERSION 3.0)
project(peerclient)

set(webrtc_sdk_source_dir /workspaces/src)
set(webrtc_sdk_build_dir /workspaces/src/out)


find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED IMPORTED_TARGET gtk+-3.0)
# find_package(jsoncpp REQUIRED)

add_executable(peerclient client/linux/main.cc client/linux/main_wnd.cc)

target_link_libraries(peerclient PRIVATE 
    PkgConfig::GTK3 
    ${webrtc_sdk_build_dir}/obj/libwebrtc.a
#    ${webrtc_sdk_build_dir}/obj/examples/libbigwrapper.a
    )
# ../third_party/llvm-build/Release+Asserts/lib/clang/21/lib/x86_64-unknown-linux-gnu/libclang_rt.builtins.a
target_include_directories(peerclient PRIVATE 
    ${webrtc_sdk_source_dir} 
    ${webrtc_sdk_source_dir}/third_party/abseil-cpp/
    ${CMAKE_CURRENT_LIST_DIR}/client/
    ${webrtc_sdk_source_dir}/third_party/libyuv/include
    )
target_compile_definitions(peerclient PRIVATE -DUSE_UDEV -DUSE_AURA=1 -DUSE_GLIB=1 -DUSE_OZONE=1 -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_GNU_SOURCE -DCR_CLANG_REVISION=\"llvmorg-21-init-9266-g09006611-1\" -D_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_NONE -D_GLIBCXX_ASSERTIONS=1 -DCR_SYSROOT_KEY=20250129T203412Z-1 -D_DEBUG -DDYNAMIC_ANNOTATIONS_ENABLED=1 -DWEBRTC_ENABLE_PROTOBUF=0 -DWEBRTC_STRICT_FIELD_TRIALS=0 -DWEBRTC_INCLUDE_INTERNAL_AUDIO_DEVICE -DRTC_ENABLE_VP9 -DRTC_ENABLE_H265 -DRTC_DAV1D_IN_INTERNAL_DECODER_FACTORY -DWEBRTC_HAVE_SCTP -DWEBRTC_USE_H264 -DWEBRTC_LIBRARY_IMPL -DPROTOBUF_ENABLE_DEBUG_LOGGING_MAY_LEAK_PII=0 -DWEBRTC_ENABLE_AVX2 -DWEBRTC_NON_STATIC_TRACE_EVENT_HANDLERS=0 -DWEBRTC_POSIX -DWEBRTC_LINUX -DWEBRTC_ALLOW_DEPRECATED_NAMESPACES -DABSL_ALLOCATOR_NOTHROW=1 -DCHROMIUM -DLIBYUV_DISABLE_NEON -DLIBYUV_DISABLE_SVE -DLIBYUV_DISABLE_SME -DLIBYUV_DISABLE_LSX -DLIBYUV_DISABLE_LASX -DWEBRTC_USE_PIPEWIRE -DWEBRTC_USE_GIO)
