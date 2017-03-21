#ifndef WEBRTC_MODULES_VIDEO_RENDER_MAIN_SOURCE_LINUX_VIDEO_SDL_RENDER_H_
#define WEBRTC_MODULES_VIDEO_RENDER_MAIN_SOURCE_LINUX_VIDEO_SDL_RENDER_H_

#include "webrtc/modules/video_render/include/video_render_defines.h"

#include <SDL2/SDL.h>
namespace webrtc{
class CriticalSectionWrapper;
class VideoSDLChannel;

class VideoSDLRender
{
public:
	VideoSDLRender();
	~VideoSDLRender();

	int32_t Init();

	VideoSDLChannel* CreateSDLRenderChannel(int32_t streamId,
                                            int32_t zOrder,
                                            const float left,
                                            const float top,
                                            const float right,
                                            const float bottom);
	int32_t DeleteSDLRenderChannel(int32_t streamId);

    int32_t GetIncomingStreamProperties(int32_t streamId,
                                        uint32_t& zOrder,
                                        float& left, float& top,
                                        float& right, float& bottom);
private:
	CriticalSectionWrapper& _critSect;

};

}

#endif