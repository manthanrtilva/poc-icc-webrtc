/*
 *  Copyright 2012 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "peerconnection8/client/linux/main_wnd.h"

#include <stdio.h>

#include <iostream>
#include <sstream>
#include <string>

#include "peerconnection8/client/main_wnd.h"

GtkMainWnd::GtkMainWnd() {}

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

void GtkMainWnd::QueueUIThreadCallback(int msg_id, void* data) {
  std::lock_guard<std::mutex> lock(cb_mutex_);
  cb_queue_.push({msg_id, data});
}

bool GtkMainWnd::Create() {
  running_ = true;
  PrintHelp();
  input_thread_ = std::thread(&GtkMainWnd::InputThreadFunc, this);
  return true;
}

bool GtkMainWnd::Destroy() {
  running_ = false;
  return true;
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
      case Command::CREATE_OFFER:
        callback_->CreateOffer();
        break;
      case Command::SET_SDP:
        callback_->SetRemoteSdp(cmd.message);
        break;
      case Command::ADD_ICE:
        callback_->AddRemoteIceCandidate(cmd.message);
        break;
      case Command::DISCONNECT_PEER:
        callback_->DisconnectFromCurrentPeer();
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
  printf("  offer             - create SDP offer (you are the caller)\n");
  printf("  setsdp <json>     - set remote SDP offer or answer\n");
  printf("  addice <json>     - add remote ICE candidate\n");
  printf("  pd                - disconnect from peer\n");
  printf("  msg <text>        - send message to connected peer\n");
  printf("  exit              - exit\n");
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
    if (token == "offer") {
      cmd.type = Command::CREATE_OFFER;
    } else if (token == "setsdp") {
      std::string rest;
      if (!std::getline(iss, rest) ||
          rest.find_first_not_of(' ') == std::string::npos) {
        printf("Usage: setsdp <json>\n");
        continue;
      }
      rest = rest.substr(rest.find_first_not_of(' '));
      cmd.type = Command::SET_SDP;
      cmd.message = rest;
    } else if (token == "addice") {
      std::string rest;
      if (!std::getline(iss, rest) ||
          rest.find_first_not_of(' ') == std::string::npos) {
        printf("Usage: addice <json>\n");
        continue;
      }
      rest = rest.substr(rest.find_first_not_of(' '));
      cmd.type = Command::ADD_ICE;
      cmd.message = rest;
    } else if (token == "pd") {
      cmd.type = Command::DISCONNECT_PEER;
    } else if (token == "msg") {
      std::string rest;
      if (!std::getline(iss, rest) ||
          rest.find_first_not_of(' ') == std::string::npos) {
        printf("Usage: msg <text>\n");
        continue;
      }
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
