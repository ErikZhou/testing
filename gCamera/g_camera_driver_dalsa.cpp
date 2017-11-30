// ImageProcessingThreadSampleDlg.cpp : implementation file
//


#include "g_camera_driver_dalsa.h"
//#include "ImageWnd.h"
#include <process.h>
#include <VLib.h>
#include <vector>

#include "g_camera_dalsa.h"
#include "logger.h"

//#ifdef _DEBUG
//#define new DEBUG_NEW
//#endif



CCameraDriverDalsa::CCameraDriverDalsa() :
m_pCamera(nullptr)
{
    m_pCamera = new CDalsa();




}
CCameraDriverDalsa::~CCameraDriverDalsa()
{
    if (nullptr != m_pCamera)
    {
        delete m_pCamera;
        m_pCamera = nullptr;
    }
}




std::string GetExePath()
{
	char buff[MAX_PATH];
	GetModuleFileNameA(NULL, buff, MAX_PATH);
	std::string::size_type pos = std::string(buff).find_last_of("\\/");
	return std::string(buff).substr(0, pos);
}

int CCameraDriverDalsa::OpenCamera()
{
	std::string sExePath = GetExePath();
    const std::string sWarning = sExePath + "\\warning.txt";
	const std::string sError = sExePath + "\\error.txt";
	const std::string sInfo = sExePath + "\\info.txt";
    initLogger(sInfo, sWarning, sError);

    LOG(INFO) << "CCameraDriverDalsa::OpenCamera start.";
    
    if (nullptr == m_pCamera)
    {
//log
        return -1;
    }
    


    //const std::string sCamera ="";
    //0 dalsa0.ccf
    char *a = "gCamera";
    char *b = "Linea_M4096-7um_1";
    char *c = "0";
    //char *d = "C:/Dropbox/1Work/main/trunk/GSMV/bin_debug/dalsa0.ccf";
    char *d = "dalsa0.ccf";
    char* cs[] = { a, b, c, d}; // initialized
    int iRet = m_pCamera->OpenCamera(4, cs);
    return iRet;
}



int CCameraDriverDalsa::CloseCamera()
{
    m_iStatus = CAMERA_STOPPED;
    m_pCamera->CloseCamera();
    return 0;
}
CAMERA_STATUS CCameraDriverDalsa::GetCameraStatus() const
{
    return m_iStatus;
}

bool CCameraDriverDalsa::CallbackOutputStars(void *Param)
{
    char s8_Out[200];
    sprintf_s(s8_Out, "  ***** Some stars *****  %s\n", (char*)Param);

    printf(s8_Out);
    return true;
}

void CCameraDriverDalsa::SetCallback(cCallback* pCall)
{
    m_pCamera->SetCallback(pCall);
}

void CCameraDriverDalsa::UninitialCamera()
{
    m_pCamera->UninitialCamera();
}

void CCameraDriverDalsa::PauseCamera()
{
	m_pCamera->PauseCamera();

}


  
