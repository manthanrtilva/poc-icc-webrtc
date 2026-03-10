/*
 *  Copyright 2012 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef EXAMPLES_PEERCONNECTION_CLIENT_MAIN_WND_H_
#define EXAMPLES_PEERCONNECTION_CLIENT_MAIN_WND_H_

#include <string>

class MainWndCallback {
 public:
  virtual void DisconnectFromCurrentPeer() = 0;
  virtual void UIThreadCallback(int msg_id, void* data) = 0;
  virtual void Close() = 0;
  virtual void SendChatMessage(const std::string& message) = 0;

  // Serverless (out-of-band) SDP signaling.
  virtual void CreateOffer() = 0;
  virtual void SetRemoteSdp(const std::string& json) = 0;
  virtual void AddRemoteIceCandidate(const std::string& json) = 0;

 protected:
  virtual ~MainWndCallback() {}
};

// Pure virtual interface for the main window.
class MainWindow {
 public:
  virtual ~MainWindow() {}

  enum UI {
    CONNECT_TO_SERVER,
    STREAMING,
  };

  virtual void RegisterObserver(MainWndCallback* callback) = 0;

  virtual bool IsWindow() = 0;
  virtual void MessageBox(const char* caption,
                          const char* text,
                          bool is_error) = 0;

  virtual UI current_ui() = 0;

  virtual void QueueUIThreadCallback(int msg_id, void* data) = 0;
  virtual void DisplayChatMessage(const std::string& message) = 0;
};

#endif  // EXAMPLES_PEERCONNECTION_CLIENT_MAIN_WND_H_
