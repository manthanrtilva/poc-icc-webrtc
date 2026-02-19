/*
 *  Copyright 2012 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "libwebrtc/examples/peerconnection-cli/client/linux/main_wnd.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <map>

#include "api/media_stream_interface.h"
#include "api/scoped_refptr.h"
#include "api/video/i420_buffer.h"
#include "api/video/video_frame.h"
#include "api/video/video_frame_buffer.h"
#include "api/video/video_rotation.h"
#include "api/video/video_source_interface.h"
#include "libwebrtc/examples/peerconnection-cli/client/linux/main_wnd.h"
#include "libwebrtc/examples/peerconnection-cli/client/peer_connection_client.h"
#include "rtc_base/checks.h"
#include "rtc_base/logging.h"
#include "third_party/libyuv/include/libyuv/convert_from.h"

namespace {

//
// Simple static functions that simply forward the callback to the
// GtkMainWnd instance.
//

bool OnDestroyedCallback(GtkWidget* widget, GdkEvent* event, void* data) {
  RTC_LOG(LS_INFO) << __FUNCTION__;
  return false;
}

void OnClickedCallback(GtkWidget* widget, void* data) {
  // reinterpret_cast<GtkMainWnd*>(data)->OnClicked(widget);
  RTC_LOG(LS_INFO) << __FUNCTION__;
}

bool SimulateButtonClick(void* button) {
  // g_signal_emit_by_name(button, "clicked");
  RTC_LOG(LS_INFO) << __FUNCTION__;
  return false;
}

bool OnKeyPressCallback(GtkWidget* widget, GdkEventKey* key, void* data) {
  // reinterpret_cast<GtkMainWnd*>(data)->OnKeyPress(widget, key);
  RTC_LOG(LS_INFO) << __FUNCTION__;
  return false;
}

void OnRowActivatedCallback(GtkTreeView* tree_view,
                            GtkTreePath* path,
                            GtkTreeViewColumn* column,
                            void* data) {
  // reinterpret_cast<GtkMainWnd*>(data)->OnRowActivated(tree_view, path,
  // column);
  RTC_LOG(LS_INFO) << __FUNCTION__;
}

bool SimulateLastRowActivated(void* data) {
  RTC_LOG(LS_INFO) << __FUNCTION__;
  return false;
}

// Creates a tree view, that we use to display the list of peers.
void InitializeList(GtkWidget* list) {
  RTC_LOG(LS_INFO) << __FUNCTION__;
}

// Adds an entry to a tree view.
void AddToList(GtkWidget* list, const char* str, int value) {
  RTC_LOG(LS_INFO) << __FUNCTION__;
}

struct UIThreadCallbackData {
  explicit UIThreadCallbackData(MainWndCallback* cb, int id, void* d)
      : callback(cb), msg_id(id), data(d) {}
  MainWndCallback* callback;
  int msg_id;
  void* data;
};

bool HandleUIThreadCallback(void* data) {
  UIThreadCallbackData* cb_data = reinterpret_cast<UIThreadCallbackData*>(data);
  cb_data->callback->UIThreadCallback(cb_data->msg_id, cb_data->data);
  delete cb_data;
  return false;
}

bool Redraw(void* data) {
  GtkMainWnd* wnd = reinterpret_cast<GtkMainWnd*>(data);
  wnd->OnRedraw();
  return false;
}

bool Draw(GtkWidget* widget, cairo_t* cr, void* data) {
  GtkMainWnd* wnd = reinterpret_cast<GtkMainWnd*>(data);
  wnd->Draw(widget, cr);
  return false;
}

}  // namespace

//
// GtkMainWnd implementation.
//

GtkMainWnd::GtkMainWnd(const char* server,
                       int port,
                       bool autoconnect,
                       bool autocall)
    : window_(NULL),
      draw_area_(NULL),
      vbox_(NULL),
      server_edit_(NULL),
      port_edit_(NULL),
      peer_list_(NULL),
      callback_(NULL),
      server_(server),
      autoconnect_(autoconnect),
      autocall_(autocall) {
  char buffer[10];
  snprintf(buffer, sizeof(buffer), "%i", port);
  port_ = buffer;
}

GtkMainWnd::~GtkMainWnd() {
  RTC_DCHECK(!IsWindow());
}

void GtkMainWnd::RegisterObserver(MainWndCallback* callback) {
  callback_ = callback;
}

bool GtkMainWnd::IsWindow() {
  return is_window_;
}

void GtkMainWnd::MessageBox(const char* caption,
                            const char* text,
                            bool is_error) {
  RTC_LOG(LS_INFO) << __FUNCTION__;
}

MainWindow::UI GtkMainWnd::current_ui() {
  return STREAMING;
}

void GtkMainWnd::StartLocalRenderer(webrtc::VideoTrackInterface* local_video) {
  local_renderer_.reset(new VideoRenderer(this, local_video));
}

void GtkMainWnd::StopLocalRenderer() {
  local_renderer_.reset();
}

void GtkMainWnd::StartRemoteRenderer(
    webrtc::VideoTrackInterface* remote_video) {
  remote_renderer_.reset(new VideoRenderer(this, remote_video));
}

void GtkMainWnd::StopRemoteRenderer() {
  remote_renderer_.reset();
}

void GtkMainWnd::QueueUIThreadCallback(int msg_id, void* data) {
  RTC_LOG(LS_INFO) << __FUNCTION__;
}

bool GtkMainWnd::Create() {
  th = std::thread([this]() {
    RTC_LOG(LS_INFO) << __FUNCTION__;
    // while () {
    std::string line;
    while (IsWindow() && std::getline(std::cin, line)) {
      if (line == "exit") {
        is_window_ = false;
        if (queue_base_) {
          queue_base_->PostTask([this] { callback_->DisconnectFromServer(); });
        }

        break;
      } else if (line.rfind("connect", 0) == 0) {
        std::string server;
        int port = 8888;  // default port
        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd >> server >> port;

        if (!server.empty()) {
          if (queue_base_) {
            queue_base_->PostTask(
                [this, server, port] { callback_->StartLogin(server, port); });
          }
        } else {
          RTC_LOG(LS_WARNING) << "Usage: connect <server> [port]";
        }
      } else if (line == "list") {
        if (queue_base_) {
          queue_base_->PostTask([this] { callback_->PrintPeers(); });
        }
      } else if (line.rfind("pconnect", 0) == 0) {
        int port = 8888;  // default port
        std::istringstream iss(line);
        std::string cmd;
        int id = -1;
        iss >> cmd >> id;
        if (id != -1) {
          if (queue_base_) {
            queue_base_->PostTask([this, id] { callback_->ConnectToPeer(id); });
          }
        } else {
          RTC_LOG(LS_WARNING) << "Usage: pconnect <peer_id>";
        }
      } else if (line.rfind("psend", 0) == 0) {
        std::string message;
        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd >> message;
        if (!message.empty()) {
          if (queue_base_) {
            queue_base_->PostTask(
                [this, message] { callback_->SendDataMessage(message); });
          }
        } else {
          RTC_LOG(LS_WARNING) << "Usage: psend <message>";
        }
      }
      std::cout << "You entered: " << line << std::endl;
    }
    // }
  });
  return window_ != NULL;
}
#if 0
int id = -1;
    gtk_tree_model_get(model, &iter, 0, &text, 1, &id, -1);
    if (id != -1)
      callback_->ConnectToPeer(id);
#endif
bool GtkMainWnd::Destroy() {
  if (th.joinable()) {
    th.join();
  }
  if (!IsWindow())
    return false;
  return true;
}

void GtkMainWnd::SwitchToConnectUI() {
  RTC_LOG(LS_INFO) << __FUNCTION__;

  RTC_DCHECK(IsWindow());
  RTC_DCHECK(vbox_ == NULL);

  // if (autoconnect_)
  //   g_idle_add(SimulateButtonClick, button);
}

void GtkMainWnd::SwitchToPeerList(const Peers& peers) {
  RTC_LOG(LS_INFO) << __FUNCTION__;

  // AddToList(peer_list_, "List of currently connected peers:", -1);
  // for (Peers::const_iterator i = peers.begin(); i != peers.end(); ++i)
  //   AddToList(peer_list_, i->second.c_str(), i->first);

  // if (autocall_ && peers.begin() != peers.end())
  //   g_idle_add(SimulateLastRowActivated, peer_list_);
}

void GtkMainWnd::SwitchToStreamingUI() {
  RTC_LOG(LS_INFO) << __FUNCTION__;

  RTC_DCHECK(draw_area_ == NULL);
}

void GtkMainWnd::OnDestroyed(GtkWidget* widget, GdkEvent* event) {
  callback_->Close();

  window_ = NULL;
  draw_area_ = NULL;
  vbox_ = NULL;
  server_edit_ = NULL;
  port_edit_ = NULL;
  peer_list_ = NULL;
  is_window_ = false;
}

void GtkMainWnd::OnClicked(GtkWidget* widget) {
  // Make the connect button insensitive, so that it cannot be clicked more than
  // once.  Now that the connection includes auto-retry, it should not be
  // necessary to click it more than once.
  int port = 8888;
  callback_->StartLogin(server_, port);
}

void GtkMainWnd::OnKeyPress(GtkWidget* widget, GdkEventKey* key) {
  RTC_LOG(LS_INFO) << __FUNCTION__;
}

void GtkMainWnd::OnRowActivated(GtkTreeView* tree_view,
                                GtkTreePath* path,
                                GtkTreeViewColumn* column) {
  RTC_LOG(LS_INFO) << __FUNCTION__;
}

void GtkMainWnd::OnRedraw() {}

void GtkMainWnd::Draw(GtkWidget* widget, cairo_t* cr) {}

GtkMainWnd::VideoRenderer::VideoRenderer(
    GtkMainWnd* main_wnd,
    webrtc::VideoTrackInterface* track_to_render)
    : width_(0),
      height_(0),
      main_wnd_(main_wnd),
      rendered_track_(track_to_render) {
  rendered_track_->AddOrUpdateSink(this, webrtc::VideoSinkWants());
}

GtkMainWnd::VideoRenderer::~VideoRenderer() {
  rendered_track_->RemoveSink(this);
}

void GtkMainWnd::VideoRenderer::SetSize(int width, int height) {}

void GtkMainWnd::VideoRenderer::OnFrame(const webrtc::VideoFrame& video_frame) {
}
