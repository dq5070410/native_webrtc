#include <iostream>
#include <X11/Xlib.h>  
#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  

#include "webrtc/video_engine/include/vie_base.h"
#include "webrtc/video_engine/include/vie_network.h"
#include "webrtc/video_engine/include/vie_codec.h"
#include "webrtc/video_engine/include/vie_network.h"
#include "webrtc/video_engine/include/vie_rtp_rtcp.h"
#include "webrtc/video_engine/include/vie_image_process.h"
#include "webrtc/video_engine/include/vie_capture.h"
#include "webrtc/modules/video_capture/include/video_capture_factory.h"
#include "webrtc/video_engine/include/vie_render.h"
#include "webrtc/video_engine/include/vie_external_codec.h"
#include "webrtc/common_video/libyuv/include/webrtc_libyuv.h"
#include "webrtc/modules/video_render/include/video_render_defines.h"
#include "webrtc/system_wrappers/interface/clock.h"
#include "webrtc/system_wrappers/interface/scoped_ptr.h"
#include "webrtc/video/encoded_frame_callback_adapter.h"
#include "receive_statistics_proxy.h"
#include "transport_adapter.h"
#include "webrtc/test/channel_transport/udp_transport.h"
#include "webrtc/video_receive_stream.h"

#include "main.h"

#if USEOPENH264 || USEX264
#include "h264.h"
#endif

using namespace webrtc;
using namespace internal;
using namespace newapi;

static const int kNackHistoryMs = 1000;
static const int kTOFExtensionId = 4;
static const int kASTExtensionId = 5;
static const uint8_t kSendRtxPayloadType = 96;
static const uint32_t kSendRtxSsrcs = 0x654322;
static const uint32_t kSendSsrcs = 0x654321;
static const uint32_t kReceiverLocalSsrc = 0x123456;
const char* kTOffset = "urn:ietf:params:rtp-hdrext:toffset";
const char* kAbsSendTime ="http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time";

class VideoChannelTransport : public webrtc::test::UdpTransportData {
public:
	VideoChannelTransport(ViENetwork* vie_network, int channel);

	virtual  ~VideoChannelTransport();

	// Start implementation of UdpTransportData.
	virtual void IncomingRTPPacket(const int8_t* incoming_rtp_packet,
		const int32_t packet_length,
		const char* /*from_ip*/,
		const uint16_t /*from_port*/) OVERRIDE;

	virtual void IncomingRTCPPacket(const int8_t* incoming_rtcp_packet,
		const int32_t packet_length,
		const char* /*from_ip*/,
		const uint16_t /*from_port*/) OVERRIDE;
	// End implementation of UdpTransportData.

	// Specifies the ports to receive RTP packets on.
	int SetLocalReceiver(uint16_t rtp_port);

	// Specifies the destination port and IP address for a specified channel.
	int SetSendDestination(const char* ip_address, uint16_t rtp_port);

private:
	int channel_;
	ViENetwork* vie_network_;
	webrtc::test::UdpTransport* socket_transport_;
};

VideoChannelTransport::VideoChannelTransport(ViENetwork* vie_network,
	int channel)
	: channel_(channel),
	vie_network_(vie_network) {
	uint8_t socket_threads = 1;
	socket_transport_ = webrtc::test::UdpTransport::Create(channel, socket_threads);
	int registered = vie_network_->RegisterSendTransport(channel,
		*socket_transport_);
}

VideoChannelTransport::~VideoChannelTransport() {
	vie_network_->DeregisterSendTransport(channel_);
	webrtc::test::UdpTransport::Destroy(socket_transport_);
}

void VideoChannelTransport::IncomingRTPPacket(
	const int8_t* incoming_rtp_packet,
	const int32_t packet_length,
	const char* /*from_ip*/,
	const uint16_t /*from_port*/) {
	std::cout << " rtp packet is coming,packet_length = "<< packet_length << std::endl;
	/*static int discard_num = 0;
	discard_num++;
	if(discard_num == 20)
	{
		discard_num = 0;
		return;
	}*/
	vie_network_->ReceivedRTPPacket(
		channel_, incoming_rtp_packet, packet_length, PacketTime());
}

void VideoChannelTransport::IncomingRTCPPacket(
	const int8_t* incoming_rtcp_packet,
	const int32_t packet_length,
	const char* /*from_ip*/,
	const uint16_t /*from_port*/) {
	std::cout << " rtcp packet is coming,packet_length = "<< packet_length << std::endl;
	vie_network_->ReceivedRTCPPacket(channel_, incoming_rtcp_packet,
		packet_length);
}

int VideoChannelTransport::SetLocalReceiver(uint16_t rtp_port) {
	int return_value = socket_transport_->InitializeReceiveSockets(this,
		rtp_port);
	if (return_value == 0) {
		return socket_transport_->StartReceiving(500);
	}
	return return_value;
}

int VideoChannelTransport::SetSendDestination(const char* ip_address,
	uint16_t rtp_port) {
	return socket_transport_->InitializeSendSockets(ip_address, rtp_port);
}

int VideoEngineSample(void* window)
{

	int error = 0;
	VideoReceiveStream::Config config_;
	scoped_ptr<ReceiveStatisticsProxy> stats_proxy_;
	config_.rtp.remote_ssrc = kSendSsrcs;
	config_.rtp.local_ssrc = kReceiverLocalSsrc;
	//config_.rtp.rtx[kSendRtxPayloadType].ssrc = kSendRtxSsrcs;
  	//config_.rtp.rtx[kSendRtxPayloadType].payload_type = kSendRtxPayloadType;
	config_.rtp.fec.red_payload_type = 118;
	config_.rtp.fec.ulpfec_payload_type = 119;
	config_.rtp.nack.rtp_history_ms = 0;
	config_.rtp.extensions.push_back(RtpExtension(kTOffset, kTOFExtensionId));
	config_.rtp.extensions.push_back(RtpExtension(kAbsSendTime, kASTExtensionId));

	//
	// Create a VideoEngine instance
	//
	webrtc::VideoEngine* ptrViE = NULL;
	ptrViE = webrtc::VideoEngine::Create();
	if (ptrViE == NULL)
	{
		printf("ERROR in VideoEngine::Create\n");
		return -1;
	}

	//
	// Init VideoEngine and create a channel
	//
	webrtc::ViEBase* ptrViEBase = webrtc::ViEBase::GetInterface(ptrViE);
	if (ptrViEBase == NULL)
	{
		printf("ERROR in ViEBase::GetInterface\n");
		return -1;
	}

	error = ptrViEBase->Init();
	if (error == -1)
	{
		printf("ERROR in ViEBase::Init\n");
		return -1;
	}

	webrtc::ViERTP_RTCP* ptrViERtpRtcp =
		webrtc::ViERTP_RTCP::GetInterface(ptrViE);
	if (ptrViERtpRtcp == NULL)
	{
		printf("ERROR in ViERTP_RTCP::GetInterface\n");
		return -1;
	}

	int videoChannel = -1;
	error = ptrViEBase->CreateChannel(videoChannel);
	if (error == -1)
	{
		printf("ERROR in ViEBase::CreateChannel\n");
		return -1;
	}
	//
	// RTP/RTCP settings
	//

	error = ptrViERtpRtcp->SetRTCPStatus(videoChannel,
		webrtc::kRtcpCompound_RFC4585);
	if (error == -1)
	{
		printf("ERROR in ViERTP_RTCP::SetRTCPStatus\n");
		return -1;
	}
	//error = ptrViERtpRtcp->SetHybridNACKFECStatus(videoChannel,true,3,-1);

	error = ptrViERtpRtcp->SetKeyFrameRequestMethod(
		videoChannel, webrtc::kViEKeyFrameRequestPliRtcp);
	if (error == -1)
	{
		printf("ERROR in ViERTP_RTCP::SetKeyFrameRequestMethod\n");
		return -1;
	}

	error = ptrViERtpRtcp->SetRembStatus(videoChannel, true, true);
	if (error == -1)
	{
		printf("ERROR in ViERTP_RTCP::SetTMMBRStatus\n");
		return -1;
	}
	ptrViERtpRtcp->SetNACKStatus(videoChannel, config_.rtp.nack.rtp_history_ms > 0);
  	ptrViERtpRtcp->SetKeyFrameRequestMethod(videoChannel, kViEKeyFrameRequestPliRtcp);
  	switch (config_.rtp.rtcp_mode)
  	{
    	case newapi::kRtcpCompound:
      		ptrViERtpRtcp->SetRTCPStatus(videoChannel, kRtcpCompound_RFC4585);
      		break;
    	case newapi::kRtcpReducedSize:
      		ptrViERtpRtcp->SetRTCPStatus(videoChannel, kRtcpNonCompound_RFC5506);
      		break;
  	}
	assert(config_.rtp.remote_ssrc != 0);
	// TODO(pbos): What's an appropriate local_ssrc for receive-only streams?
	assert(config_.rtp.local_ssrc != 0);
	assert(config_.rtp.remote_ssrc != config_.rtp.local_ssrc);

	ptrViERtpRtcp->SetLocalSSRC(videoChannel, config_.rtp.local_ssrc);
	// TODO(pbos): Support multiple RTX, per video payload.
	VideoReceiveStream::Config::Rtp::RtxMap::const_iterator it = config_.rtp.rtx.begin();
	if (it != config_.rtp.rtx.end())
	{
		assert(it->second.ssrc != 0);
		assert(it->second.payload_type != 0);

		ptrViERtpRtcp->SetRemoteSSRCType(videoChannel, kViEStreamTypeRtx, it->second.ssrc);
		ptrViERtpRtcp->SetRtxReceivePayloadType(videoChannel, it->second.payload_type);
	}

	ptrViERtpRtcp->SetRembStatus(videoChannel, false, config_.rtp.remb);

	for (size_t i = 0; i < config_.rtp.extensions.size(); ++i)
	{
		const std::string& extension = config_.rtp.extensions[i].name;
		int id = config_.rtp.extensions[i].id;
		if (extension == kTOffset)
		{
			if (ptrViERtpRtcp->SetReceiveTimestampOffsetStatus(videoChannel, true, id) != 0)
			abort();
		} 
		else if (extension == kAbsSendTime)
		{
		if (ptrViERtpRtcp->SetReceiveAbsoluteSendTimeStatus(videoChannel, true, id) != 0)
			abort();
		} 
		else 
		{
			abort();  // Unsupported extension.
		}
	}


	//
	// Set up rendering
	//
	webrtc::ViERender* ptrViERender = webrtc::ViERender::GetInterface(ptrViE);
	if (ptrViERender == NULL) {
		printf("ERROR in ViERender::GetInterface\n");
		return -1;
	}
	//add linux render

	error = ptrViERender->AddRenderer(videoChannel, window, 1, 0.0, 0.0, 1.0,1.0);
	if (error == -1)
	{
		printf("ERROR in ViERender::AddRenderer\n");
		return -1;
	}

	error = ptrViERender->StartRender(videoChannel);
	if (error == -1)
	{
		printf("ERROR in ViERender::StartRender\n");
		return -1;
	}

	//
	// Setup codecs
	//
	webrtc::ViECodec* ptrViECodec = webrtc::ViECodec::GetInterface(ptrViE);
	if (ptrViECodec == NULL)
	{
		printf("ERROR in ViECodec::GetInterface\n");
		return -1;
	}

#if USEOPENH264 || USEX264
	webrtc::H264Encoder *h264encoder = webrtc::H264Encoder::Create();
	webrtc::H264Decoder *h264decoder = webrtc::H264Decoder::Create();

	webrtc::ViEExternalCodec* external_codec = webrtc::ViEExternalCodec
		::GetInterface(ptrViE);
	external_codec->RegisterExternalSendCodec(videoChannel, 124,
		h264encoder, false);
	external_codec->RegisterExternalReceiveCodec(videoChannel,
		124, h264decoder, false);
#endif

	VideoCodec videoCodec;

	int numOfVeCodecs = ptrViECodec->NumberOfCodecs();
	for (int i = 0; i<numOfVeCodecs; ++i)
	{
		if (ptrViECodec->GetCodec(i, videoCodec) != -1)
		{
#if USEOPENH264 || USEX264
			if (videoCodec.codecType == kVideoCodecH264)
				break;
#endif
#if USEVP8
			if (videoCodec.codecType == kVideoCodecVP8)
				break;
#endif
		}
	}

	videoCodec.targetBitrate = 256;
	videoCodec.minBitrate = 200;
	videoCodec.maxBitrate = 300;
	videoCodec.width = 640;
	videoCodec.height = 480;
#if USEOPENH264 || USEX264
	videoCodec.plType = 124;

#endif
	videoCodec.maxFramerate = 25;
	error = ptrViECodec->SetReceiveCodec(videoChannel, videoCodec);
	assert(error != -1);


	if (config_.rtp.fec.ulpfec_payload_type != -1)
	{
		// ULPFEC without RED doesn't make sense.
		assert(config_.rtp.fec.red_payload_type != -1);
		VideoCodec codec;
		memset(&codec, 0, sizeof(codec));
		codec.codecType = kVideoCodecULPFEC;
		strcpy(codec.plName, "ulpfec");
		codec.plType = config_.rtp.fec.ulpfec_payload_type;
		if (ptrViECodec->SetReceiveCodec(videoChannel, codec) != 0)
		{
	  		printf("Could not set ULPFEC codec. This shouldn't happen.\n");
	  		abort();
		}
	}
	if (config_.rtp.fec.red_payload_type != -1)
	{
		VideoCodec codec;
		memset(&codec, 0, sizeof(codec));
		codec.codecType = kVideoCodecRED;
		strcpy(codec.plName, "red");
		codec.plType = config_.rtp.fec.red_payload_type;
		if (ptrViECodec->SetReceiveCodec(videoChannel, codec) != 0)
		{
	  		printf("Could not set RED codec. This shouldn't happen.\n");
	  		abort();
		}
	}

	for (size_t i = 0; i < config_.codecs.size(); ++i)
	{
		if (ptrViECodec->SetReceiveCodec(videoChannel, config_.codecs[i]) != 0)
		{
	  	// TODO(pbos): Abort gracefully, this can be a runtime error.
	  	//             Factor out to an Init() method.
	  		abort();
		}
	}

	stats_proxy_.reset(new ReceiveStatisticsProxy(config_.rtp.local_ssrc, Clock::GetRealTimeClock(), ptrViERtpRtcp, ptrViECodec, videoChannel));

	if (ptrViERtpRtcp->RegisterReceiveChannelRtcpStatisticsCallback(
	      videoChannel, stats_proxy_.get()) != 0)
		abort();

	if (ptrViERtpRtcp->RegisterReceiveChannelRtpStatisticsCallback(
	      videoChannel, stats_proxy_.get()) != 0)
		abort();

	if (config_.rtp.rtcp_xr.receiver_reference_time_report)
	{
    	ptrViERtpRtcp->SetRtcpXrRrtrStatus(videoChannel, true);
  	}


	//
	// Address settings
	//
	webrtc::ViENetwork* ptrViENetwork =
		webrtc::ViENetwork::GetInterface(ptrViE);
	if (ptrViENetwork == NULL)
	{
		printf("ERROR in ViENetwork::GetInterface\n");
		return -1;
	}

	VideoChannelTransport* video_channel_transport = NULL;
	video_channel_transport = new VideoChannelTransport(
		ptrViENetwork, videoChannel);


	const char* ipAddress = "192.168.1.1";
	const unsigned short rtpPort = 6666;
	std::cout << std::endl;
	std::cout << "Using rtp port: " << rtpPort << std::endl;
	std::cout << std::endl;

	error = video_channel_transport->SetLocalReceiver(rtpPort);
	if (error == -1)
	{
		printf("ERROR in SetLocalReceiver\n");
		return -1;
	}
	/*error = video_channel_transport->SetSendDestination(ipAddress, rtpPort);
	if (error == -1)
	{
		printf("ERROR in SetSendDestination\n");
		return -1;
	}*/

	error = ptrViEBase->StartReceive(videoChannel);
	if (error == -1)
	{
		printf("ERROR in ViENetwork::StartReceive\n");
		return -1;
	}

	/*error = ptrViEBase->StartSend(videoChannel);
	if (error == -1)
	{
		printf("ERROR in ViENetwork::StartSend\n");
		return -1;
	}*/

	printf("\n call started\n\n");
	printf("Press enter to stop...\n");
	while ((getchar()) != '\n')
		;

	error = ptrViEBase->StopReceive(videoChannel);
	if (error == -1)
	{
		printf("ERROR in ViEBase::StopReceive\n");
		return -1;
	}


	error = ptrViEBase->DeleteChannel(videoChannel);
	if (error == -1)
	{
		printf("ERROR in ViEBase::DeleteChannel\n");
		return -1;
	}

	delete video_channel_transport;
	int remainingInterfaces = 0;
	remainingInterfaces = ptrViECodec->Release();
	remainingInterfaces += ptrViERtpRtcp->Release();
	remainingInterfaces += ptrViENetwork->Release();
	remainingInterfaces += ptrViEBase->Release();
#if USEOPENH264 || USEX264
	remainingInterfaces += external_codec->Release();
#endif
	if (remainingInterfaces > 0)
	{
		printf("ERROR: Could not release all interfaces\n");
		return -1;
	}

	bool deleted = webrtc::VideoEngine::Delete(ptrViE);
	if (deleted == false)
	{
		printf("ERROR in VideoEngine::Delete\n");
		return -1;
	}

	return 0;

}

int main(int argc, char* argvc[])
{
	// Create the windows
	/*Display *display;  
    Window window;  
    XEvent event;   
    char *msg = "hello , this is test window";  
    int s;  
   
    // 与Xserver建立连接 
    display = XOpenDisplay(NULL);  
    if (display == NULL)  
    {  
        fprintf(stderr, "Cannot open display\n");  
        exit(1);  
    }  
   
          
    s = DefaultScreen(display);  
   
    // 创建一个窗口  
    window = XCreateSimpleWindow(display, RootWindow(display, s), 10, 10, 200, 200, 1,  
                           BlackPixel(display, s), WhitePixel(display, s));  
   
    //选择一种感兴趣的事件进行监听
    XSelectInput(display, window, ExposureMask | KeyPressMask);  
   
    // 显示窗口
    XMapWindow(display, window); */
    
	VideoEngineSample(NULL);

	/* 关闭与Xserver服务器的连接 */  
    //XCloseDisplay(display); 
	return 0;
}

