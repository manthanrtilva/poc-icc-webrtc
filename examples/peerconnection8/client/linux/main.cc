/*
 *  Copyright 2012 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <stdio.h>

#include <fstream>

#include "api/scoped_refptr.h"
#include "peerconnection8/client/conductor.h"
#include "peerconnection8/client/linux/main_wnd.h"
#include "rtc_base/logging.h"
#include "rtc_base/physical_socket_server.h"
#include "rtc_base/ssl_adapter.h"
#include "rtc_base/thread.h"

class FileLogSink : public webrtc::LogSink {
 public:
  explicit FileLogSink(const std::string& path) : file_(path, std::ios::app) {}

  void OnLogMessage(const std::string& message) override {
    file_ << message;
    file_.flush();
  }

 private:
  std::ofstream file_;
};

class CustomSocketServer : public webrtc::PhysicalSocketServer {
 public:
  explicit CustomSocketServer(GtkMainWnd* wnd)
      : wnd_(wnd), conductor_(nullptr) {}
  virtual ~CustomSocketServer() {}

  void SetMessageQueue(webrtc::Thread* queue) override {
    message_queue_ = queue;
  }

  void set_conductor(Conductor* conductor) { conductor_ = conductor; }

  // Pump CLI commands and WebRTC callbacks on every Wait() iteration.
  bool Wait(webrtc::TimeDelta max_wait_duration, bool process_io) override {
    wnd_->ProcessPending();

    if (!wnd_->IsWindow() && !conductor_->connection_active()) {
      message_queue_->Quit();
    }
    return webrtc::PhysicalSocketServer::Wait(webrtc::TimeDelta::Zero(),
                                              process_io);
  }

 protected:
  webrtc::Thread* message_queue_;
  GtkMainWnd* wnd_;
  Conductor* conductor_;
};

int main(int argc, char* argv[]) {
  FileLogSink sink("/tmp/webrtc.log");
  webrtc::LogMessage::AddLogToStream(&sink,
                                     webrtc::LS_VERBOSE);  // capture all levels
  webrtc::LogMessage::SetLogToStderr(false);

  GtkMainWnd wnd;
  wnd.Create();

  CustomSocketServer socket_server(&wnd);
  webrtc::AutoSocketServerThread thread(&socket_server);

  webrtc::InitializeSSL();
  // Must be constructed after we set the socketserver.
  auto conductor = webrtc::make_ref_counted<Conductor>(&wnd);
  socket_server.set_conductor(conductor.get());

  thread.Run();

  wnd.Destroy();
  webrtc::CleanupSSL();
  webrtc::LogMessage::RemoveLogToStream(
      &sink);  // cleanup before sink is destroyed
  return 0;
}
