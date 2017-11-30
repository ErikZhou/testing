// g_camera_driver_dalsa.h : header file
//

#pragma once


#include "g_camera_interface.h"

#include <atlstr.h>
//#include <afxstr.h>
#include <cstring>

#define NODE_NAME_WIDTH         (int8_t*)"Width"
#define NODE_NAME_HEIGHT        (int8_t*)"Height"
#define NODE_NAME_PIXELFORMAT   (int8_t*)"PixelFormat"
#define NODE_NAME_GAIN          (int8_t*)"GainRaw"
#define NODE_NAME_ACQSTART      (int8_t*)"AcquisitionStart"
#define NODE_NAME_ACQSTOP       (int8_t*)"AcquisitionStop"

#define	WIN_WIDTH	480
#define	WIN_HEIGHT	360

// CCameraJai dialog
class CDalsa;
class __declspec(dllexport) CCameraDriverDalsa : public ICamera
{
    // Construction
public:
    CCameraDriverDalsa();// standard constructor
    ~CCameraDriverDalsa();

    int OpenCamera();
    int CloseCamera();
    void UninitialCamera();
	void PauseCamera();
    CAMERA_STATUS GetCameraStatus() const;
    void Free() { delete this; }
    bool CallbackOutputStars(void *Param);

    void SetCallback(cCallback* pCall);

    static ICamera * __stdcall Create() 
    {
        printf("CCameraDriverDalsa created");
        return new CCameraDriverDalsa();
    }

    // Implementation
public:
    //BOOL OpenFactoryAndCamera();
    //void CloseFactoryAndCamera();
    //void StreamCBFunc(void * pAqImageInfo);
    //void ImageThread(void * pParam);
private:
    CAMERA_STATUS               m_iStatus;
    CDalsa*                     m_pCamera;

};
