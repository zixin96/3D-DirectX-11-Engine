#pragma once

// HRESULT hr should exist in the local scope for these macros to work

// graphics exception checking/throwing macros (some with dxgi infos)
#define GFX_EXCEPT_NOINFO(hr) Graphics::HrException( __LINE__,__FILE__,(hr) )
#define GFX_THROW_NOINFO(hrcall) if( FAILED( hr = (hrcall) ) ) throw Graphics::HrException( __LINE__,__FILE__,hr )

#ifdef DX_DEBUG
#define GFX_EXCEPT(hr) Graphics::HrException( __LINE__,__FILE__,(hr),infoManager_.GetMessages() )
#define GFX_THROW_INFO(hrcall) infoManager_.Set(); if( FAILED( hr = (hrcall) ) ) throw GFX_EXCEPT(hr)
#define GFX_DEVICE_REMOVED_EXCEPT(hr) Graphics::DeviceRemovedException( __LINE__,__FILE__,(hr),infoManager_.GetMessages() )
#define GFX_THROW_INFO_ONLY(call) infoManager_.Set(); (call); {auto v = infoManager_.GetMessages(); if(!v.empty()) {throw Graphics::InfoException( __LINE__,__FILE__,v );}}
#else
#define GFX_EXCEPT(hr) Graphics::HrException( __LINE__,__FILE__,(hr) )
#define GFX_THROW_INFO(hrcall) GFX_THROW_NOINFO(hrcall)
#define GFX_DEVICE_REMOVED_EXCEPT(hr) Graphics::DeviceRemovedException( __LINE__,__FILE__,(hr) )
#define GFX_THROW_INFO_ONLY(call) (call)
#endif

// macro for importing infomanager into local scope
#ifndef DX_DEBUG
#define INFOMAN(gfx) HRESULT hr
#else
// INFOMAN is used outside graphics class, when we want a local copy of DxgiInfoManager
#define INFOMAN(gfx) HRESULT hr; DxgiInfoManager& infoManager_ = GetInfoManager((gfx))
#endif


