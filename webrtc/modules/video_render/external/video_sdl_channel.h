#ifndef WEBRTC_MODULES_VIDEO_RENDER_MAIN_SOURCE_EXTERNAL_VIDEO_SDL_CHANNEL_H_
#define WEBRTC_MODULES_VIDEO_RENDER_MAIN_SOURCE_EXTERNAL_VIDEO_SDL_CHANNEL_H_

#include "webrtc/common_video/libyuv/include/webrtc_libyuv.h"
#include "webrtc/modules/video_render/include/video_render_defines.h"

#include <SDL2/SDL.h>

namespace webrtc{

class CriticalSectionWrapper;

#define DEFAULT_RENDER_FRAME_WIDTH 640
#define DEFAULT_RENDER_FRAME_HEIGHT 480

class VideoSDLChannel:public VideoRenderCallback
{
public:
	VideoSDLChannel(int32_t id);
	virtual ~VideoSDLChannel();
	int32_t Init(float left, float top, float right,float bottom);
	virtual int32_t RenderFrame(const uint32_t streamId,I420VideoFrame& videoFrame);
private:
	CriticalSectionWrapper& _crit;

	float _left;
    float _right;
    float _top;
    float _bottom;
	
	SDL_Window *screen_;   
    SDL_Renderer* sdlRenderer_;  
    SDL_Texture* sdlTexture_;  
    SDL_Rect sdlRect_;  
    SDL_Thread *video_tid_;  
    SDL_Event event_;

};


}

#endif