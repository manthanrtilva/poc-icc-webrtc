/*
 *  Copyright 2012 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "peerconnection7/client/linux/main_wnd.h"

#include <stdio.h>

#include <iostream>
#include <sstream>
#include <string>

#include "api/media_stream_interface.h"
#include "api/video/video_frame.h"
#include "peerconnection7/client/main_wnd.h"
#include "rtc_base/logging.h"

GtkMainWnd::GtkMainWnd(const char* server,
                       int port,
                       bool autoconnect,
                       bool autocall)
    : server_(server), autoconnect_(autoconnect), autocall_(autocall) {
  char buffer[10];
  snprintf(buffer, sizeof(buffer), "%i", port);
  port_ = buffer;
}

GtkMainWnd::~GtkMainWnd() {
  if (input_thread_.joinable())
    input_thread_.detach();
}

void GtkMainWnd::RegisterObserver(MainWndCallback* callback) {
  callback_ = callback;
}

bool GtkMainWnd::IsWindow() {
  return running_.load();
}

void GtkMainWnd::MessageBox(const char* caption,
                            const char* text,
                            bool is_error) {
  printf("[%s] %s\n", caption, text);
}

MainWindow::UI GtkMainWnd::current_ui() {
  return ui_;
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
  std::lock_guard<std::mutex> lock(cb_mutex_);
  cb_queue_.push({msg_id, data});
}

bool GtkMainWnd::Create() {
  running_ = true;
  PrintHelp();
  if (autoconnect_) {
    Command cmd;
    cmd.type = Command::CONNECT_SERVER;
    cmd.server = server_;
    cmd.port = port_.length() ? atoi(port_.c_str()) : 8888;
    std::lock_guard<std::mutex> lock(cmd_mutex_);
    cmd_queue_.push(cmd);
  }
  input_thread_ = std::thread(&GtkMainWnd::InputThreadFunc, this);
  return true;
}

bool GtkMainWnd::Destroy() {
  running_ = false;
  return true;
}

void GtkMainWnd::SwitchToConnectUI() {
  ui_ = CONNECT_TO_SERVER;
  printf("Status: disconnected. Use 'sc <ip> <port>' to connect to server.\n");
}

void GtkMainWnd::SwitchToPeerList(const Peers& peers) {
  ui_ = LIST_PEERS;
  peers_ = peers;
  printf("Connected to server. Online peers:\n");
  for (const auto& p : peers)
    printf("  [%d] %s\n", p.first, p.second.c_str());
  if (peers.empty())
    printf("  (no peers online)\n");
  if (autocall_ && !peers.empty()) {
    Command cmd;
    cmd.type = Command::CONNECT_PEER;
    cmd.peer_id = peers.begin()->first;
    std::lock_guard<std::mutex> lock(cmd_mutex_);
    cmd_queue_.push(cmd);
  }
}

void GtkMainWnd::SwitchToStreamingUI() {
  ui_ = STREAMING;
  printf("Streaming. Use 'pd' to disconnect from peer.\n");
}

void GtkMainWnd::DisplayChatMessage(const std::string& message) {
  printf("Chat: %s\n", message.c_str());
}

void GtkMainWnd::ProcessPending() {
  // Drain UI-thread callbacks (posted from WebRTC engine threads).
  while (true) {
    UICallback cb;
    {
      std::lock_guard<std::mutex> lock(cb_mutex_);
      if (cb_queue_.empty())
        break;
      cb = cb_queue_.front();
      cb_queue_.pop();
    }
    if (callback_)
      callback_->UIThreadCallback(cb.msg_id, cb.data);
  }

  // Drain CLI commands (posted from the input thread).
  while (true) {
    Command cmd;
    {
      std::lock_guard<std::mutex> lock(cmd_mutex_);
      if (cmd_queue_.empty())
        break;
      cmd = cmd_queue_.front();
      cmd_queue_.pop();
    }
    if (!callback_)
      continue;
    switch (cmd.type) {
      case Command::CONNECT_SERVER:
        callback_->StartLogin(cmd.server, cmd.port);
        break;
      case Command::CONNECT_PEER:
        callback_->ConnectToPeer(cmd.peer_id);
        break;
      case Command::DISCONNECT_SERVER:
        callback_->DisconnectFromServer();
        break;
      case Command::DISCONNECT_PEER:
        callback_->DisconnectFromCurrentPeer();
        break;
      case Command::LIST_PEERS:
        if (peers_.empty()) {
          printf("  (no peers online)\n");
        } else {
          for (const auto& p : peers_)
            printf("  [%d] %s\n", p.first, p.second.c_str());
        }
        break;
      case Command::SEND_MESSAGE:
        callback_->SendChatMessage(cmd.message);
        break;
      case Command::EXIT:
        callback_->Close();
        running_ = false;
        break;
    }
  }
}

void GtkMainWnd::PrintHelp() {
  printf("Commands:\n");
  printf("  sc <ip> <port>  - connect to server\n");
  printf("  pc <peer_id>    - connect to peer\n");
  printf("  sd              - disconnect from server\n");
  printf("  pd              - disconnect from peer\n");
  printf("  list            - list peers\n");
  printf("  msg <text>      - send message to connected peer\n");
  printf("  exit            - exit\n");
}

void GtkMainWnd::InputThreadFunc() {
  std::string line;
  while (running_ && std::getline(std::cin, line)) {
    if (line.empty())
      continue;
    std::istringstream iss(line);
    std::string token;
    iss >> token;

    Command cmd;
    if (token == "sc") {
      std::string ip;
      int port = 8888;
      if (!(iss >> ip)) {
        printf("Usage: sc <ip> <port>\n");
        continue;
      }
      iss >> port;
      cmd.type = Command::CONNECT_SERVER;
      cmd.server = ip;
      cmd.port = port;
    } else if (token == "pc") {
      int peer_id;
      if (!(iss >> peer_id)) {
        printf("Usage: pc <peer_id>\n");
        continue;
      }
      cmd.type = Command::CONNECT_PEER;
      cmd.peer_id = peer_id;
    } else if (token == "sd") {
      cmd.type = Command::DISCONNECT_SERVER;
    } else if (token == "pd") {
      cmd.type = Command::DISCONNECT_PEER;
    } else if (token == "list") {
      cmd.type = Command::LIST_PEERS;
    } else if (token == "msg") {
      std::string rest;
      if (!std::getline(iss, rest) ||
          rest.find_first_not_of(' ') == std::string::npos) {
        printf("Usage: msg <text>\n");
        continue;
      }
      // strip leading space
      rest = rest.substr(rest.find_first_not_of(' '));
      cmd.type = Command::SEND_MESSAGE;
      cmd.message = rest;
    } else if (token == "exit") {
      cmd.type = Command::EXIT;
    } else {
      printf("Unknown command: %s\n", token.c_str());
      PrintHelp();
      continue;
    }

    {
      std::lock_guard<std::mutex> lock(cmd_mutex_);
      cmd_queue_.push(cmd);
    }
  }
  running_ = false;
}

//
// GtkMainWnd::VideoRenderer
//

GtkMainWnd::VideoRenderer::VideoRenderer(
    GtkMainWnd* /*main_wnd*/,
    webrtc::VideoTrackInterface* track_to_render)
    : rendered_track_(track_to_render) {
  rendered_track_->AddOrUpdateSink(this, webrtc::VideoSinkWants());
}

GtkMainWnd::VideoRenderer::~VideoRenderer() {
  rendered_track_->RemoveSink(this);
}
