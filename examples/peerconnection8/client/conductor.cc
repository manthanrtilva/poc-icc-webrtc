/*
 *  Copyright 2012 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "peerconnection8/client/conductor.h"

#include <stddef.h>

#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "absl/memory/memory.h"
#include "api/create_peerconnection_factory.h"
#include "api/jsep.h"
#include "api/make_ref_counted.h"
#include "api/peer_connection_interface.h"
#include "api/scoped_refptr.h"
#include "api/task_queue/default_task_queue_factory.h"
#include "api/task_queue/task_queue_factory.h"
#include "json/reader.h"
#include "json/value.h"
#include "json/writer.h"
#include "peerconnection8/client/defaults.h"
#include "peerconnection8/client/main_wnd.h"
#include "rtc_base/checks.h"
#include "rtc_base/logging.h"
#include "rtc_base/strings/json.h"

namespace {
// Names used for a IceCandidate JSON object.
const char kCandidateSdpMidName[] = "sdpMid";
const char kCandidateSdpMlineIndexName[] = "sdpMLineIndex";
const char kCandidateSdpName[] = "candidate";

// Names used for a SessionDescription JSON object.
const char kSessionDescriptionTypeName[] = "type";
const char kSessionDescriptionSdpName[] = "sdp";

class DummySetSessionDescriptionObserver
    : public webrtc::SetSessionDescriptionObserver {
 public:
  static webrtc::scoped_refptr<DummySetSessionDescriptionObserver> Create() {
    return webrtc::make_ref_counted<DummySetSessionDescriptionObserver>();
  }
  virtual void OnSuccess() { RTC_LOG(LS_INFO) << __FUNCTION__; }
  virtual void OnFailure(webrtc::RTCError error) {
    RTC_LOG(LS_INFO) << __FUNCTION__ << " " << ToString(error.type()) << ": "
                     << error.message();
  }
};

}  // namespace

Conductor::Conductor(MainWindow* main_wnd) : peer_id_(-1), main_wnd_(main_wnd) {
  main_wnd->RegisterObserver(this);
}

Conductor::~Conductor() {
  RTC_DCHECK(!peer_connection_);
}

bool Conductor::connection_active() const {
  return peer_connection_ != nullptr;
}

void Conductor::Close() {
  DeletePeerConnection();
}

bool Conductor::InitializePeerConnection() {
  RTC_DCHECK(!peer_connection_factory_);
  RTC_DCHECK(!peer_connection_);

  if (!signaling_thread_.get()) {
    signaling_thread_ = webrtc::Thread::CreateWithSocketServer();
    signaling_thread_->Start();
  }

  webrtc::PeerConnectionFactoryDependencies deps;
  deps.signaling_thread = signaling_thread_.get();
  deps.task_queue_factory = webrtc::CreateDefaultTaskQueueFactory();
  task_queue_factory_ = deps.task_queue_factory.get();
  peer_connection_factory_ =
      webrtc::CreateModularPeerConnectionFactory(std::move(deps));

  if (!peer_connection_factory_) {
    main_wnd_->MessageBox("Error", "Failed to initialize PeerConnectionFactory",
                          true);
    DeletePeerConnection();
    return false;
  }

  if (!CreatePeerConnection()) {
    main_wnd_->MessageBox("Error", "CreatePeerConnection failed", true);
    DeletePeerConnection();
  }

  AddTracks();

  return peer_connection_ != nullptr;
}

bool Conductor::CreatePeerConnection() {
  RTC_DCHECK(peer_connection_factory_);
  RTC_DCHECK(!peer_connection_);

  webrtc::PeerConnectionInterface::RTCConfiguration config;
  config.sdp_semantics = webrtc::SdpSemantics::kUnifiedPlan;
  webrtc::PeerConnectionInterface::IceServer server;
  server.uri = GetPeerConnectionString();
  config.servers.push_back(server);

  webrtc::PeerConnectionDependencies pc_dependencies(this);
  auto error_or_peer_connection =
      peer_connection_factory_->CreatePeerConnectionOrError(
          config, std::move(pc_dependencies));
  if (error_or_peer_connection.ok()) {
    peer_connection_ = std::move(error_or_peer_connection.value());
  }
  return peer_connection_ != nullptr;
}

void Conductor::DeletePeerConnection() {
  if (data_channel_) {
    data_channel_->UnregisterObserver();
    data_channel_ = nullptr;
  }
  peer_connection_ = nullptr;
  peer_connection_factory_ = nullptr;
  peer_id_ = -1;
}

//
// PeerConnectionObserver implementation.
//

void Conductor::OnIceCandidate(const webrtc::IceCandidateInterface* candidate) {
  RTC_LOG(LS_INFO) << __FUNCTION__ << " " << candidate->sdp_mline_index();

  Json::Value jmessage;
  jmessage[kCandidateSdpMidName] = candidate->sdp_mid();
  jmessage[kCandidateSdpMlineIndexName] = candidate->sdp_mline_index();
  std::string sdp;
  if (!candidate->ToString(&sdp)) {
    RTC_LOG(LS_ERROR) << "Failed to serialize candidate";
    return;
  }
  jmessage[kCandidateSdpName] = sdp;

  Json::StreamWriterBuilder factory;
  factory["indentation"] = "";
  std::string json = Json::writeString(factory, jmessage);
  printf("*****\naddice %s\n*****\n", json.c_str());
  fflush(stdout);
}

void Conductor::AddTracks() {
  if (!peer_connection_->GetSenders().empty()) {
    return;  // Already added tracks.
  }
}

void Conductor::DisconnectFromCurrentPeer() {
  RTC_LOG(LS_INFO) << __FUNCTION__;
  DeletePeerConnection();
}

void Conductor::UIThreadCallback(int msg_id, void* data) {
  switch (msg_id) {
    case PEER_CONNECTION_CLOSED:
      RTC_LOG(LS_INFO) << "PEER_CONNECTION_CLOSED";
      DeletePeerConnection();
      break;

    case CHAT_MESSAGE_RECEIVED: {
      std::string* msg = reinterpret_cast<std::string*>(data);
      main_wnd_->DisplayChatMessage(*msg);
      delete msg;
      break;
    }

    default:
      RTC_DCHECK_NOTREACHED();
      break;
  }
}

void Conductor::OnSuccess(webrtc::SessionDescriptionInterface* desc) {
  peer_connection_->SetLocalDescription(
      DummySetSessionDescriptionObserver::Create().get(), desc);

  std::string sdp;
  desc->ToString(&sdp);
  RTC_LOG(LS_INFO) << " Created local session description :" << sdp;

  Json::Value jmessage;
  jmessage[kSessionDescriptionTypeName] =
      webrtc::SdpTypeToString(desc->GetType());
  jmessage[kSessionDescriptionSdpName] = sdp;

  Json::StreamWriterBuilder factory;
  factory["indentation"] = "";
  std::string json = Json::writeString(factory, jmessage);
  printf("\n******\nsetsdp %s\n******\n", json.c_str());
  fflush(stdout);
}

void Conductor::OnFailure(webrtc::RTCError error) {
  RTC_LOG(LS_ERROR) << ToString(error.type()) << ": " << error.message();
}

void Conductor::OnDataChannel(
    webrtc::scoped_refptr<webrtc::DataChannelInterface> channel) {
  RTC_LOG(LS_INFO) << "Data channel received: " << channel->label();
  if (!data_channel_) {
    data_channel_ = channel;
    data_channel_->RegisterObserver(this);
  } else {
    RTC_LOG(LS_WARNING) << "Data channel already exists, ignoring.";
  }
}

void Conductor::OnStateChange() {
  RTC_LOG(LS_INFO) << "Data channel state: "
                   << webrtc::DataChannelInterface::DataStateString(
                          data_channel_->state());
}

void Conductor::OnMessage(const webrtc::DataBuffer& buffer) {
  std::string text(buffer.data.data<char>(), buffer.data.size());
  RTC_LOG(LS_INFO) << "Data channel message: " << text;
  std::string* msg = new std::string("Peer: " + text);
  main_wnd_->QueueUIThreadCallback(CHAT_MESSAGE_RECEIVED, msg);
}

void Conductor::SendChatMessage(const std::string& message) {
  if (data_channel_ &&
      data_channel_->state() == webrtc::DataChannelInterface::kOpen) {
    webrtc::DataBuffer buffer(message);
    data_channel_->Send(buffer);
    std::string* echo = new std::string("Me: " + message);
    main_wnd_->QueueUIThreadCallback(CHAT_MESSAGE_RECEIVED, echo);
  } else {
    RTC_LOG(LS_WARNING) << "Data channel not open, cannot send message";
  }
}

void Conductor::CreateOffer() {
  if (peer_connection_.get()) {
    main_wnd_->MessageBox("Error", "Already in a call", true);
    return;
  }
  if (InitializePeerConnection()) {
    peer_id_ = 1;  // dummy id — no signaling server
    webrtc::DataChannelInit dc_config;
    auto dc_or_error =
        peer_connection_->CreateDataChannelOrError("chat", &dc_config);
    if (dc_or_error.ok()) {
      data_channel_ = dc_or_error.MoveValue();
      data_channel_->RegisterObserver(this);
    }
    peer_connection_->CreateOffer(
        this, webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());
  } else {
    main_wnd_->MessageBox("Error", "Failed to initialize PeerConnection", true);
  }
}

void Conductor::SetRemoteSdp(const std::string& json) {
  if (!peer_connection_.get()) {
    // We are the callee receiving an offer.
    if (!InitializePeerConnection()) {
      printf("Error: Failed to initialize PeerConnection\n");
      return;
    }
    peer_id_ = 1;  // dummy id — no signaling server
  }

  Json::CharReaderBuilder factory;
  std::unique_ptr<Json::CharReader> reader =
      absl::WrapUnique(factory.newCharReader());
  Json::Value jmessage;
  if (!reader->parse(json.data(), json.data() + json.length(), &jmessage,
                     nullptr)) {
    printf("Error: Failed to parse SDP JSON\n");
    return;
  }

  std::string type_str;
  webrtc::GetStringFromJsonObject(jmessage, kSessionDescriptionTypeName,
                                  &type_str);
  std::optional<webrtc::SdpType> type_maybe =
      webrtc::SdpTypeFromString(type_str);
  if (!type_maybe) {
    printf("Error: Unknown SDP type: %s\n", type_str.c_str());
    return;
  }
  webrtc::SdpType type = *type_maybe;

  std::string sdp;
  if (!webrtc::GetStringFromJsonObject(jmessage, kSessionDescriptionSdpName,
                                       &sdp)) {
    printf("Error: Can't parse SDP field\n");
    return;
  }

  webrtc::SdpParseError error;
  std::unique_ptr<webrtc::SessionDescriptionInterface> session_description =
      webrtc::CreateSessionDescription(type, sdp, &error);
  if (!session_description) {
    printf("Error: SDP parse error: %s\n", error.description.c_str());
    return;
  }

  peer_connection_->SetRemoteDescription(
      DummySetSessionDescriptionObserver::Create().get(),
      session_description.release());

  if (type == webrtc::SdpType::kOffer) {
    // We are the callee — generate and print an answer.
    peer_connection_->CreateAnswer(
        this, webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());
  }
}

void Conductor::AddRemoteIceCandidate(const std::string& json) {
  if (!peer_connection_.get()) {
    printf("Error: No active peer connection\n");
    return;
  }

  Json::CharReaderBuilder factory;
  std::unique_ptr<Json::CharReader> reader =
      absl::WrapUnique(factory.newCharReader());
  Json::Value jmessage;
  if (!reader->parse(json.data(), json.data() + json.length(), &jmessage,
                     nullptr)) {
    printf("Error: Failed to parse ICE candidate JSON\n");
    return;
  }

  std::string sdp_mid;
  int sdp_mlineindex = 0;
  std::string sdp;
  if (!webrtc::GetStringFromJsonObject(jmessage, kCandidateSdpMidName,
                                       &sdp_mid) ||
      !webrtc::GetIntFromJsonObject(jmessage, kCandidateSdpMlineIndexName,
                                    &sdp_mlineindex) ||
      !webrtc::GetStringFromJsonObject(jmessage, kCandidateSdpName, &sdp)) {
    printf("Error: Can't parse ICE candidate fields\n");
    return;
  }

  webrtc::SdpParseError error;
  std::unique_ptr<webrtc::IceCandidateInterface> candidate(
      webrtc::CreateIceCandidate(sdp_mid, sdp_mlineindex, sdp, &error));
  if (!candidate.get()) {
    printf("Error: ICE parse error: %s\n", error.description.c_str());
    return;
  }

  if (!peer_connection_->AddIceCandidate(candidate.get())) {
    printf("Error: Failed to add ICE candidate\n");
  }
}
