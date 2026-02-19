#!/bin/bash

# PATH="$PATH:/depot_tools"
# export ARCH=x64
# cd /workspaces/src/ && gn gen out/ --args="is_debug=true target_os=\"linux\" target_cpu=\"x64\" is_debug=true rtc_include_tests=false rtc_use_h264=true ffmpeg_branding=\"Chrome\" is_component_build=false use_rtti=true use_custom_libcxx=false rtc_enable_protobuf=false rtc_build_examples=true treat_warnings_as_errors=false"

ninja -C /workspaces/src/out/ \
    && cp /workspaces/src/out/peerconnection_client_gui /workspaces/src/libwebrtc/ \
    && sshpass -p "$PASSWORD"  scp /workspaces/src/out/peerconnection_client_gui zti@10.40.0.200:/tmp/
