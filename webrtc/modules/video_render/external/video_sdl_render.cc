#include "webrtc/modules/video_render/external/video_sdl_channel.h"
#include "webrtc/modules/video_render/external/video_sdl_render.h"

#include "webrtc/system_wrappers/interface/critical_section_wrapper.h"

namespace webrtc{

VideoSDLRender::VideoSDLRender(): _critSect(*CriticalSectionWrapper::CreateCriticalSection())
{

}
VideoSDLRender::~VideoSDLRender()
{
	delete &_critSect;
}
int32_t VideoSDLRender::Init()
{
	CriticalSectionScoped cs(&_critSect);
	return 0;
}
VideoSDLChannel* VideoSDLRender::CreateSDLRenderChannel(int32_t streamId,
                                            int32_t zOrder,
                                            const float left,
                                            const float top,
                                            const float right,
                                            const float bottom)
{
	CriticalSectionScoped cs(&_critSect);
	VideoSDLChannel* renderChannel = NULL;
	renderChannel = new VideoSDLChannel(streamId);
	renderChannel->Init(left, top, right, bottom);
	return renderChannel;
}
int32_t VideoSDLRender::DeleteSDLRenderChannel(int32_t streamId)
{
	return 0;
}
int32_t VideoSDLRender::GetIncomingStreamProperties(int32_t streamId,
                                        uint32_t& zOrder,
                                        float& left, float& top,
                                        float& right, float& bottom)
{
	return 0;
}

}