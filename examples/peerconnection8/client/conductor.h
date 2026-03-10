/*
 *  Copyright 2012 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef EXAMPLES_PEERCONNECTION_CLIENT_CONDUCTOR_H_
#define EXAMPLES_PEERCONNECTION_CLIENT_CONDUCTOR_H_

#include <memory>
#include <string>

#include "api/data_channel_interface.h"
#include "api/jsep.h"
#include "api/peer_connection_interface.h"
#include "api/scoped_refptr.h"
#include "api/task_queue/task_queue_factory.h"
#include "peerconnection8/client/main_wnd.h"
#include "rtc_base/thread.h"

class Conductor : public webrtc::PeerConnectionObserver,
                  public webrtc::CreateSessionDescriptionObserver,
                  public webrtc::DataChannelObserver,
                  public MainWndCallback {
 public:
  enum CallbackID {
    PEER_CONNECTION_CLOSED = 2,
    CHAT_MESSAGE_RECEIVED,
  };

  Conductor(MainWindow* main_wnd);

  bool connection_active() const;

  void Close() override;

 protected:
  ~Conductor();
  bool InitializePeerConnection();
  bool CreatePeerConnection();
  void DeletePeerConnection();
  void AddTracks();

  // DataChannelObserver implementation.
  void OnStateChange() override;
  void OnMessage(const webrtc::DataBuffer& buffer) override;
  void OnBufferedAmountChange(uint64_t previous_amount) override {}

  //
  // PeerConnectionObserver implementation.
  //

  void OnSignalingChange(
      webrtc::PeerConnectionInterface::SignalingState new_state) override {}
  void OnDataChannel(
      webrtc::scoped_refptr<webrtc::DataChannelInterface> channel) override;
  void OnRenegotiationNeeded() override {}
  void OnIceConnectionChange(
      webrtc::PeerConnectionInterface::IceConnectionState new_state) override {}
  void OnIceGatheringChange(
      webrtc::PeerConnectionInterface::IceGatheringState new_state) override {}
  void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override;
  void OnIceConnectionReceivingChange(bool receiving) override {}

  void DisconnectFromCurrentPeer() override;

  void UIThreadCallback(int msg_id, void* data) override;

  void SendChatMessage(const std::string& message) override;

  void CreateOffer() override;
  void SetRemoteSdp(const std::string& json) override;
  void AddRemoteIceCandidate(const std::string& json) override;

  // CreateSessionDescriptionObserver implementation.
  void OnSuccess(webrtc::SessionDescriptionInterface* desc) override;
  void OnFailure(webrtc::RTCError error) override;

 protected:
  int peer_id_;
  std::unique_ptr<webrtc::Thread> signaling_thread_;
  webrtc::TaskQueueFactory* task_queue_factory_ = nullptr;
  webrtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_;
  webrtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>
      peer_connection_factory_;
  MainWindow* main_wnd_;
  webrtc::scoped_refptr<webrtc::DataChannelInterface> data_channel_;
};

#endif  // EXAMPLES_PEERCONNECTION_CLIENT_CONDUCTOR_H_
