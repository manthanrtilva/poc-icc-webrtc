/*
 *  Copyright 2012 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef EXAMPLES_PEERCONNECTION_CLIENT_LINUX_MAIN_WND_H_
#define EXAMPLES_PEERCONNECTION_CLIENT_LINUX_MAIN_WND_H_

#include <atomic>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

#include "peerconnection8/client/main_wnd.h"

// CLI implementation of the peer connection client UI.
class GtkMainWnd : public MainWindow {
 public:
  GtkMainWnd();
  ~GtkMainWnd();

  void RegisterObserver(MainWndCallback* callback) override;
  bool IsWindow() override;
  void MessageBox(const char* caption,
                  const char* text,
                  bool is_error) override;
  MainWindow::UI current_ui() override;
  void QueueUIThreadCallback(int msg_id, void* data) override;
  void DisplayChatMessage(const std::string& message) override;

  bool Create();
  bool Destroy();

  // Called from the socket server loop to drain pending callbacks and commands.
  void ProcessPending();

 protected:
 private:
  void InputThreadFunc();
  void PrintHelp();

  struct UICallback {
    int msg_id;
    void* data;
  };

  struct Command {
    enum Type {
      CREATE_OFFER,
      SET_SDP,
      ADD_ICE,
      DISCONNECT_PEER,
      SEND_MESSAGE,
      EXIT,
    };
    Type type;
    std::string message;
  };

  std::atomic<bool> running_{false};
  MainWndCallback* callback_{nullptr};
  UI ui_{CONNECT_TO_SERVER};
  std::mutex cb_mutex_;
  std::queue<UICallback> cb_queue_;
  std::mutex cmd_mutex_;
  std::queue<Command> cmd_queue_;
  std::thread input_thread_;
};

#endif  // EXAMPLES_PEERCONNECTION_CLIENT_LINUX_MAIN_WND_H_
