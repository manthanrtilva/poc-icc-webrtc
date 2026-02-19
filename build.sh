#!/bin/bash

cd /workspaces/src/
PATH="$PATH:/depot_tools"
export ARCH=x64
gn gen out/ --args="is_debug=true target_os=\"linux\" target_cpu=\"$ARCH\" is_debug=true rtc_include_tests=false rtc_use_h264=true ffmpeg_branding=\"Chrome\" is_component_build=false use_rtti=true use_custom_libcxx=false rtc_enable_protobuf=false rtc_build_examples=true treat_warnings_as_errors=false"
ninja -C out/

#  --autoconnect --gui
# cmake -S /workspaces/libwebrtc/peerclient2/ -B /tmp/build2 -DCMAKE_CXX_COMPILER=/workspaces/src/third_party/llvm-build/Release+Asserts/bin/clang++
# cmake --build /tmp/build2/

# Build with gcc 
# gn gen out/gcc --args='is_clang=false use_sysroot=false is_debug=true target_os="linux" target_cpu="x64" rtc_include_tests=false  rtc_use_h264=true ffmpeg_branding="Chrome" is_component_build=false use_rtti=true use_custom_libcxx=false rtc_enable_protobuf=false  rtc_build_examples=true treat_warnings_as_errors=false extra_cflags_cc=["-fpermissive"] extra_ldflags=["-latomic"] use_llvm_libatomic=false'

# non gui build
# gn gen out/nongui --args="is_debug=true target_os=\"linux\" target_cpu=\"$ARCH\" is_debug=true rtc_include_tests=false rtc_use_h264=true ffmpeg_branding=\"Chrome\" is_component_build=false use_rtti=true use_custom_libcxx=false rtc_enable_protobuf=false rtc_build_examples=true rtc_use_x11=false use_gio=false use_glib=false"

# 