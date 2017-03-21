/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "webrtc/modules/video_render/external/video_render_external_impl.h"

#include "webrtc/modules/video_render/external/video_sdl_render.h"
#include "webrtc/modules/video_render/external/video_sdl_channel.h"

namespace webrtc {

VideoRenderExternalImpl::VideoRenderExternalImpl(
                                                 const int32_t id,
                                                 const VideoRenderType videoRenderType,
                                                 void* window,
                                                 const bool fullscreen) :
    _id(id), _critSect(*CriticalSectionWrapper::CreateCriticalSection()),
            _fullscreen(fullscreen),_ptrSDLRender(NULL)
{
}

VideoRenderExternalImpl::~VideoRenderExternalImpl()
{
  if(_ptrSDLRender)
    delete _ptrSDLRender;
    delete &_critSect;
}

int32_t VideoRenderExternalImpl::Init()
{

    CriticalSectionScoped cs(&_critSect);
    
    _ptrSDLRender = new VideoSDLRender();
    if (!_ptrSDLRender)
    {
        return -1;
    }
    int retVal = _ptrSDLRender->Init();
    if (retVal == -1)
    {
        return -1;
    }

    return 0;
}

int32_t VideoRenderExternalImpl::ChangeUniqueId(const int32_t id)
{
    CriticalSectionScoped cs(&_critSect);
    _id = id;
    return 0;
}

int32_t VideoRenderExternalImpl::ChangeWindow(void* window)
{
    CriticalSectionScoped cs(&_critSect);
    return 0;
}

VideoRenderCallback*
VideoRenderExternalImpl::AddIncomingRenderStream(const uint32_t streamId,
                                                 const uint32_t zOrder,
                                                 const float left,
                                                 const float top,
                                                 const float right,
                                                 const float bottom)
{
    CriticalSectionScoped cs(&_critSect);
     VideoRenderCallback* renderCallback = NULL;
    if (_ptrSDLRender)
    {
        VideoSDLChannel* renderChannel =
                _ptrSDLRender->CreateSDLRenderChannel(streamId, zOrder, left,
                                                      top, right, bottom);
        if (!renderChannel)
        {
            return NULL;
        }
        renderCallback = (VideoRenderCallback *) renderChannel;
    }
    else
    {
        return NULL;
    }
    return renderCallback;
}

int32_t VideoRenderExternalImpl::DeleteIncomingRenderStream(
                                                                  const uint32_t streamId)
{
    CriticalSectionScoped cs(&_critSect);
    if (_ptrSDLRender)
    {
        return _ptrSDLRender->DeleteSDLRenderChannel(streamId);
    }
    return 0;
}

int32_t VideoRenderExternalImpl::GetIncomingRenderStreamProperties(
                                                                         const uint32_t streamId,
                                                                         uint32_t& zOrder,
                                                                         float& left,
                                                                         float& top,
                                                                         float& right,
                                                                         float& bottom) const
{
    CriticalSectionScoped cs(&_critSect);

    if (_ptrSDLRender)
    {
        return _ptrSDLRender->GetIncomingStreamProperties(streamId, zOrder,
                                                          left, top, right,
                                                          bottom);
    }

    return 0;
}

int32_t VideoRenderExternalImpl::StartRender()
{
    CriticalSectionScoped cs(&_critSect);
    return 0;
}

int32_t VideoRenderExternalImpl::StopRender()
{
    CriticalSectionScoped cs(&_critSect);
    return 0;
}

VideoRenderType VideoRenderExternalImpl::RenderType()
{
    return kRenderExternal;
}

RawVideoType VideoRenderExternalImpl::PerferedVideoType()
{
    return kVideoI420;
}

bool VideoRenderExternalImpl::FullScreen()
{
    CriticalSectionScoped cs(&_critSect);
    return _fullscreen;
}

int32_t VideoRenderExternalImpl::GetGraphicsMemory(
                                                         uint64_t& totalGraphicsMemory,
                                                         uint64_t& availableGraphicsMemory) const
{
    totalGraphicsMemory = 0;
    availableGraphicsMemory = 0;
    return -1;
}

uint32_t VideoRenderExternalImpl::RenderFrameRate(const uint32_t streamId)
{
    return 0;
}

int32_t VideoRenderExternalImpl::GetScreenResolution(
                                                           uint32_t& screenWidth,
                                                           uint32_t& screenHeight) const
{
    CriticalSectionScoped cs(&_critSect);
    screenWidth = 0;
    screenHeight = 0;
    return 0;
}


int32_t VideoRenderExternalImpl::SetStreamCropping(
                                                         const uint32_t streamId,
                                                         const float left,
                                                         const float top,
                                                         const float right,
                                                         const float bottom)
{
    CriticalSectionScoped cs(&_critSect);
    return 0;
}

int32_t VideoRenderExternalImpl::ConfigureRenderer(
                                                         const uint32_t streamId,
                                                         const unsigned int zOrder,
                                                         const float left,
                                                         const float top,
                                                         const float right,
                                                         const float bottom)
{
    CriticalSectionScoped cs(&_critSect);
    return 0;
}

int32_t VideoRenderExternalImpl::SetTransparentBackground(
                                                                const bool enable)
{
    CriticalSectionScoped cs(&_critSect);
    return 0;
}

int32_t VideoRenderExternalImpl::SetText(
                                               const uint8_t textId,
                                               const uint8_t* text,
                                               const int32_t textLength,
                                               const uint32_t textColorRef,
                                               const uint32_t backgroundColorRef,
                                               const float left,
                                               const float top,
                                               const float right,
                                               const float bottom)
{
    CriticalSectionScoped cs(&_critSect);
    return 0;
}

int32_t VideoRenderExternalImpl::SetBitmap(const void* bitMap,
                                           const uint8_t pictureId,
                                           const void* colorKey,
                                           const float left,
                                           const float top,
                                           const float right,
                                           const float bottom)
{
    CriticalSectionScoped cs(&_critSect);
    return 0;
}

}  // namespace webrtc
