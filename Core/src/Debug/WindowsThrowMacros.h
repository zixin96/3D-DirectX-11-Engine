#pragma once

// error exception helper macro
// usage: throw XXX_EXCEPT(XXX)

#define WND_EXCEPT( hr ) DXWindow::HrException( __LINE__,__FILE__,(hr) )
// some errors require you to GetLastError() to retrieve the error
#define WND_LAST_EXCEPT() DXWindow::HrException( __LINE__,__FILE__,GetLastError() )
#define WND_NOGFX_EXCEPT() DXWindow::NoGfxException( __LINE__,__FILE__ )
