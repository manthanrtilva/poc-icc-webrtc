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

#include "api/media_stream_interface.h"
#include "api/scoped_refptr.h"
#include "api/video/video_frame.h"
#include "api/video/video_sink_interface.h"
#include "peerconnection7/client/main_wnd.h"
#include "peerconnection7/client/peer_connection_client.h"

// CLI implementation of the peer connection client UI.
class GtkMainWnd : public MainWindow {
 public:
  GtkMainWnd(const char* server, int port, bool autoconnect, bool autocall);
  ~GtkMainWnd();

  virtual void RegisterObserver(MainWndCallback* callback);
  virtual bool IsWindow();
  virtual void SwitchToConnectUI();
  virtual void SwitchToPeerList(const Peers& peers);
  virtual void SwitchToStreamingUI();
  virtual void MessageBox(const char* caption, const char* text, bool is_error);
  virtual MainWindow::UI current_ui();
  virtual void StartLocalRenderer(webrtc::VideoTrackInterface* local_video);
  virtual void StopLocalRenderer();
  virtual void StartRemoteRenderer(webrtc::VideoTrackInterface* remote_video);
  virtual void StopRemoteRenderer();
  virtual void QueueUIThreadCallback(int msg_id, void* data);
  void DisplayChatMessage(const std::string& message) override;

  bool Create();
  bool Destroy();

  // Called from the socket server loop to drain pending callbacks and commands.
  void ProcessPending();

 protected:
  class VideoRenderer : public webrtc::VideoSinkInterface<webrtc::VideoFrame> {
   public:
    VideoRenderer(GtkMainWnd* main_wnd,
                  webrtc::VideoTrackInterface* track_to_render);
    virtual ~VideoRenderer();
    void OnFrame(const webrtc::VideoFrame& frame) override {}

   protected:
    webrtc::scoped_refptr<webrtc::VideoTrackInterface> rendered_track_;
  };

 private:
  void InputThreadFunc();
  void PrintHelp();

  struct UICallback {
    int msg_id;
    void* data;
  };

  struct Command {
    enum Type {
      CONNECT_SERVER,
      CONNECT_PEER,
      DISCONNECT_SERVER,
      DISCONNECT_PEER,
      LIST_PEERS,
      SEND_MESSAGE,
      EXIT
    };
    Type type;
    std::string server;
    int port{0};
    int peer_id{0};
    std::string message;
  };

  std::atomic<bool> running_{false};
  MainWndCallback* callback_{nullptr};
  UI ui_{CONNECT_TO_SERVER};
  Peers peers_;
  std::string server_;
  std::string port_;
  bool autoconnect_;
  bool autocall_;

  std::mutex cb_mutex_;
  std::queue<UICallback> cb_queue_;
  std::mutex cmd_mutex_;
  std::queue<Command> cmd_queue_;
  std::thread input_thread_;

  std::unique_ptr<VideoRenderer> local_renderer_;
  std::unique_ptr<VideoRenderer> remote_renderer_;
};

#endif  // EXAMPLES_PEERCONNECTION_CLIENT_LINUX_MAIN_WND_H_
