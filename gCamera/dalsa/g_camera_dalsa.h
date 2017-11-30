

#pragma once

#include <string>
class SapBuffer;
class SapTransfer;
class SapXferCallbackInfo;
class cCallback;
class SapAcquisition;
class SapBufferWithTrash;
class SapAcqDevice;
struct ImageBuffer;

typedef void * HANDLE;

class CDalsa
{
    // Construction
public:
    CDalsa();	// standard constructor
    ~CDalsa();

    int OpenCamera(int argc, char* argv[]);


    void CloseCamera();
	void PauseCamera();

    void UninitialCamera();

   // void SetCameraStatus(int iStatus){ m_iStatus = iStatus; }

    SapBuffer *GetBuffer()        const { return m_pBuffers; }
    SapTransfer *GetCamera()        const { return m_pXfer; }

    int GetStatus() const { return m_iStatus; }

    static void XferCallback(SapXferCallbackInfo *pInfo);

    void SetCallback(cCallback* pCall);

    void ImageThread(void * pParam);

private:
    int OpenCamera_i();
private:
    int     m_iStatus;
    SapBuffer *m_pBuffers;
    SapTransfer* m_pXfer;
    SapBufferWithTrash* m_pBuffersWith;
    //SapAcqDevice* m_pAcqDevice;
    HANDLE                      m_hThreadImageProcessing;

    SapAcquisition* m_pAcq;
    int m_Argc;
    char** m_Argv;

public:


};
