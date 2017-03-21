#include "webrtc/modules/video_render/external/video_sdl_channel.h"

#include "webrtc/common_video/libyuv/include/webrtc_libyuv.h"
#include "webrtc/system_wrappers/interface/critical_section_wrapper.h"
#include "webrtc/system_wrappers/interface/trace.h"
#define LENGTH 640*480*3/2

namespace webrtc{

VideoSDLChannel::VideoSDLChannel(int32_t id):_crit(*CriticalSectionWrapper::CreateCriticalSection())
{

}
VideoSDLChannel::~VideoSDLChannel()
{
    SDL_Quit();
	delete  &_crit;
}
int32_t VideoSDLChannel::Init(float left, float top, float right,float bottom)
{
	CriticalSectionScoped cs(&_crit);
	_left = left;
    _right = right;
    _top = top;
    _bottom = bottom;
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {    
        printf( "Could not initialize SDL - %s\n", SDL_GetError());   
        return -1;  
    }
    screen_ = SDL_CreateWindow("Simplest ffmpeg player's Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,  
        640, 480,SDL_WINDOW_OPENGL);  
  
    if(!screen_) {    
        printf("SDL: could not create window - exiting:%s\n",SDL_GetError());    
        return -1;  
    }
    sdlRenderer_ = SDL_CreateRenderer(screen_, -1, 0);    
    //IYUV: Y + U + V  (3 planes)  
    //YV12: Y + V + U  (3 planes)  
    sdlTexture_ = SDL_CreateTexture(sdlRenderer_, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING,640,480);    
  
    sdlRect_.x=0;  
    sdlRect_.y=0;  
    sdlRect_.w=640;  
    sdlRect_.h=480;
}
int32_t VideoSDLChannel::RenderFrame(const uint32_t streamId,I420VideoFrame& videoFrame)
{
    CriticalSectionScoped cs(&_crit);
    /*int r = ConvertFromI420(videoFrame, kARGB, 0, _buffer);
    if(r != 0)
    {
        printf("ConvertFromI420ToARGB error,result = %d\n",r);
        return -1;
    }*/
    //printf("Y stride %d\n",videoFrame.stride(kYPlane));
    //printf("U stride %d\n",videoFrame.stride(kUPlane));
    //printf("v stride %d\n",videoFrame.stride(kVPlane));
    unsigned char* _buffer = new unsigned char[videoFrame.stride(kYPlane) * videoFrame.height() * 3 /2];
    memcpy(_buffer,videoFrame.buffer(kYPlane),videoFrame.stride(kYPlane) * videoFrame.height());
    memcpy(_buffer+videoFrame.stride(kYPlane) * videoFrame.height(),videoFrame.buffer(kUPlane),videoFrame.stride(kYPlane) * videoFrame.height()/4);
	memcpy(_buffer+videoFrame.stride(kYPlane) * videoFrame.height()+videoFrame.stride(kYPlane) * videoFrame.height()/4,videoFrame.buffer(kVPlane),videoFrame.stride(kYPlane) * videoFrame.height()/4);
    SDL_UpdateTexture( sdlTexture_, NULL, /*videoFrame.buffer(kYPlane)*/_buffer, videoFrame.stride(kYPlane) );    
    SDL_RenderClear( sdlRenderer_ );    
    //SDL_RenderCopy( sdlRenderer, sdlTexture, &sdlRect, &sdlRect );    
    SDL_RenderCopy( sdlRenderer_, sdlTexture_, NULL, NULL);    
    SDL_RenderPresent( sdlRenderer_ );  
    delete []_buffer;
    _buffer = NULL;
}


}