

#include <iostream>

#include "api/peer_connection_interface.h"
#include "api/rtc_error.h"
#include "api/rtp_receiver_interface.h"
#include "api/scoped_refptr.h"
#include "api/task_queue/default_task_queue_factory.h"
#include "rtc_base/ssl_adapter.h"
// #include "api/task_queue/task_queue_factory.h"

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

class PeerConnection : public webrtc::PeerConnectionObserver,
                       public webrtc::CreateSessionDescriptionObserver {
 public:
  PeerConnection() {
    RTC_LOG(LS_INFO) << __FUNCTION__;
    CreatePCFactory();
    RTC_LOG(LS_INFO) << __FUNCTION__;
    CreatePeerConnection();
    RTC_LOG(LS_INFO) << __FUNCTION__;
  }
  // CreateSessionDescriptionObserver implementation.
  virtual void OnSuccess(webrtc::SessionDescriptionInterface* desc) override {
    RTC_LOG(LS_INFO) << __FUNCTION__;
  }
  virtual void OnFailure(webrtc::RTCError error) override {
    RTC_LOG(LS_INFO) << __FUNCTION__ << " " << ToString(error.type()) << ": "
                     << error.message();
  }
  // PeerConnectionObserver implementation.
  virtual void OnSignalingChange(
      webrtc::PeerConnectionInterface::SignalingState new_state) override {
    RTC_LOG(LS_INFO) << __FUNCTION__;
  }
  virtual void OnAddStream(
      webrtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override {
    RTC_LOG(LS_INFO) << __FUNCTION__;
  }
  virtual void OnRemoveStream(
      webrtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override {
    RTC_LOG(LS_INFO) << __FUNCTION__;
  }
  virtual void OnDataChannel(webrtc::scoped_refptr<webrtc::DataChannelInterface>
                                 data_channel) override {
    RTC_LOG(LS_INFO) << __FUNCTION__;
  }
  virtual void OnRenegotiationNeeded() override {
    RTC_LOG(LS_INFO) << __FUNCTION__;
  }
  virtual void OnNegotiationNeededEvent(uint32_t event_id) override {
    RTC_LOG(LS_INFO) << __FUNCTION__;
  }
  virtual void OnIceConnectionChange(
      webrtc::PeerConnectionInterface::IceConnectionState new_state) override {
    RTC_LOG(LS_INFO) << __FUNCTION__;
  }
  virtual void OnStandardizedIceConnectionChange(
      webrtc::PeerConnectionInterface::IceConnectionState new_state) override {
    RTC_LOG(LS_INFO) << __FUNCTION__;
  }
  virtual void OnConnectionChange(
      webrtc::PeerConnectionInterface::PeerConnectionState new_state) override {
    RTC_LOG(LS_INFO) << __FUNCTION__;
  }
  virtual void OnIceGatheringChange(
      webrtc::PeerConnectionInterface::IceGatheringState new_state) override {
    RTC_LOG(LS_INFO) << __FUNCTION__;
  }
  virtual void OnIceCandidate(
      const webrtc::IceCandidateInterface* candidate) override {
    RTC_LOG(LS_INFO) << __FUNCTION__;
  }
  virtual void OnIceCandidateError(const std::string& address,
                                   int port,
                                   const std::string& url,
                                   int error_code,
                                   const std::string& error_text) override {
    RTC_LOG(LS_INFO) << __FUNCTION__;
  }
  virtual void OnIceCandidatesRemoved(
      const std::vector<webrtc::Candidate>& candidates) override {
    RTC_LOG(LS_INFO) << __FUNCTION__;
  }
  virtual void OnIceConnectionReceivingChange(bool receiving) override {
    RTC_LOG(LS_INFO) << __FUNCTION__;
  }
  virtual void OnIceSelectedCandidatePairChanged(
      const webrtc::CandidatePairChangeEvent& event) override {
    RTC_LOG(LS_INFO) << __FUNCTION__;
  }
  virtual void OnAddTrack(
      webrtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver,
      const std::vector<webrtc::scoped_refptr<webrtc::MediaStreamInterface>>&
          streams) override {
    RTC_LOG(LS_INFO) << __FUNCTION__;
  }
  virtual void OnTrack(webrtc::scoped_refptr<webrtc::RtpTransceiverInterface>
                           transceiver) override {
    RTC_LOG(LS_INFO) << __FUNCTION__;
  }
  virtual void OnRemoveTrack(
      webrtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) override {
    RTC_LOG(LS_INFO) << __FUNCTION__;
  }
  virtual void OnInterestingUsage(int usage_pattern) override {
    RTC_LOG(LS_INFO) << __FUNCTION__;
  }

 private:
  void CreatePCFactory() {
    webrtc::PeerConnectionFactoryDependencies deps;
    deps.task_queue_factory = webrtc::CreateDefaultTaskQueueFactory();
    peer_connection_factory_ =
        webrtc::CreateModularPeerConnectionFactory(std::move(deps));
  }
  void CreatePeerConnection() {
    webrtc::PeerConnectionInterface::RTCConfiguration config;
    RTC_LOG(LS_INFO) << __FUNCTION__;
    config.sdp_semantics = webrtc::SdpSemantics::kUnifiedPlan;
    webrtc::PeerConnectionInterface::IceServer server;
    server.uri = "stun:stun.l.google.com:19302";
    config.servers.push_back(server);
    auto res = peer_connection_factory_->CreatePeerConnectionOrError(
        config, webrtc::PeerConnectionDependencies{this});
    RTC_LOG(LS_INFO) << __FUNCTION__;
    if (res.ok()) {
      RTC_LOG(LS_INFO) << __FUNCTION__;
      peer_connection_ = std::move(res.value());
      RTC_LOG(LS_INFO) << __FUNCTION__;
      peer_connection_->CreateOffer(
          this, webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());
      RTC_LOG(LS_INFO) << __FUNCTION__;
    }
  }

 private:
  webrtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_ =
      nullptr;
  webrtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>
      peer_connection_factory_ = nullptr;
};
class WebRTCChatClient {
 public:
  WebRTCChatClient() { webrtc::InitializeSSL(); }
  ~WebRTCChatClient() { webrtc::CleanupSSL(); }
  void CreateDataChannel() {
    peer_connection_ = webrtc::make_ref_counted<PeerConnection>();
    // peer_connection_->CreatePCFactory();
    // peer_connection_->CreatePeerConnection();
  }
  void CreateOffer() {}
  void SetRemoteOffer(const std::string& sdp) {}
  void SetRemoteAnswer(const std::string& sdp) {}
  bool IsDataChannelReady() const { return false; }
  void SendMessage(const std::string& message) {}

 private:
  webrtc::scoped_refptr<PeerConnection> peer_connection_;
};

void printUsage() {
  std::cout << "Available commands:" << std::endl;
  std::cout << "  offer   - Create an SDP offer" << std::endl;
  std::cout << "  answer  - Set a remote SDP offer and create an answer"
            << std::endl;
  std::cout << "  remote  - Set a remote SDP answer" << std::endl;
  std::cout << "  chat    - Enter chat mode to send messages" << std::endl;
  std::cout << "  help    - Show this usage information" << std::endl;
  std::cout << "  quit/exit - Exit the application" << std::endl;
}
int main() {
  std::cout << "WebRTC Data Channel CLI Chat Application" << std::endl;
  std::cout << "=======================================" << std::endl;

  WebRTCChatClient client;
  client.CreateDataChannel();
  client.CreateOffer();
  std::string command;
  while (true) {
    std::cout << "> ";
    std::getline(std::cin, command);

    if (command == "quit" || command == "exit") {
      break;
    } else if (command == "help") {
      printUsage();
    } else if (command == "offer") {
      client.CreateDataChannel();
      client.CreateOffer();
    } else if (command == "answer") {
      std::cout << "Paste the offer SDP (end with empty line):" << std::endl;
      std::string offer_sdp, line;
      while (std::getline(std::cin, line) && !line.empty()) {
        offer_sdp += line + "\n";
      }
      client.SetRemoteOffer(offer_sdp);
    } else if (command == "remote") {
      std::cout << "Paste the answer SDP (end with empty line):" << std::endl;
      std::string answer_sdp, line;
      while (std::getline(std::cin, line) && !line.empty()) {
        answer_sdp += line + "\n";
      }
      client.SetRemoteAnswer(answer_sdp);
    } else if (command == "chat") {
      if (!client.IsDataChannelReady()) {
        std::cout << "Data channel not ready. Complete the offer/answer "
                     "exchange first."
                  << std::endl;
        continue;
      }

      std::cout << "=== CHAT MODE (type 'exit' to leave) ===" << std::endl;
      std::string message;
      while (true) {
        std::cout << "[You]: ";
        std::getline(std::cin, message);
        if (message == "exit") {
          std::cout << "=== EXITED CHAT MODE ===" << std::endl;
          break;
        }
        client.SendMessage(message);
      }
    } else if (!command.empty()) {
      std::cout << "Unknown command: " << command << std::endl;
      std::cout << "Type 'help' for available commands." << std::endl;
    }
  }

  std::cout << "Goodbye!" << std::endl;
  return 0;
}
