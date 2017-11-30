// g_camera_driver_jai.h : header file
//

#pragma once


#include "g_camera_interface.h"
#include <Jai_Factory.h>
//#include <afx.h>


//#include "ImageWnd.h"
//#include "afxwin.h"
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
class cCallback;

// CCameraJai dialog
class __declspec(dllexport) CCameraJai : public ICamera
{
    // Construction
public:
    CCameraJai();	// standard constructor
    ~CCameraJai();

    int OpenCamera();
    int CloseCamera();
    void UninitialCamera();
    void PauseCamera();
    CAMERA_STATUS GetCameraStatus() const;
    void Free() { delete this; }
    bool CallbackOutputStars(void *Param){ return true; }
    void SetCallback(cCallback* pCall);

    static ICamera * __stdcall Create() 
    {
        printf("CCameraJai created\n");
        return new CCameraJai(); 
    }

    // Implementation
public:
    FACTORY_HANDLE  m_hFactory;     // Factory Handle
    CAM_HANDLE      m_hCam;         // Camera Handle
    THRD_HANDLE     m_hThread;
    int8_t          m_sCameraId[J_CAMERA_ID_SIZE];    // Camera ID
	int64_t			m_iWidthInc;
	int64_t			m_iHeightInc;
	bool			m_bEnableStreaming;


    BOOL OpenFactoryAndCamera();
    void CloseFactoryAndCamera();
    void StreamCBFunc(J_tIMAGE_INFO * pAqImageInfo);
    void InitializeControls();
    void ShowErrorMsg(CString message, J_STATUS_TYPE error);


    virtual BOOL OnInitDialog();
protected:

    // Generated message map functions
    /*afx_msg*/ HCURSOR OnQueryDragIcon();
    //DECLARE_MESSAGE_MAP()
public:
    /*afx_msg*/ void On_Destroy();
    /*afx_msg*/ void OnBnClickedStart();
    /*afx_msg*/ void OnBnClickedStop();
    /*afx_msg*/ void OnHScroll(UINT nSBCode, UINT nPos);
    void ImageThread(void * pParam);
   // void CheckCreateBmp(J_tIMAGE_INFO * pImageInfo);
    J_tIMAGE_INFO m_tBuffer;
    J_tIMAGE_INFO m_tDibBuffer;
    J_tIMAGE_INFO *m_pOriginalImage;
//    CImageWnd *m_pImageWnd;
    HBITMAP m_hBitmap;
    BITMAPINFO	m_tBmp;
    void BitInvert(J_tIMAGE_INFO * pImageInfo);
    void ImageProcessing(J_tIMAGE_INFO * pImageInfo);

   // CButton m_oAfterProcess;
    /*afx_msg*/ void OnBnClickedAfterProcess();
    int m_iAfterProcess;
private:
    CAMERA_STATUS               m_iStatus;
    HANDLE                      m_hThreadImageProcessing;
    cCallback*                  m_pCallback;
};
