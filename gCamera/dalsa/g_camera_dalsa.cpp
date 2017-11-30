// -----------------------------------------------------------------------------------------
// Sapera++ console grab example
// 
//    This program shows how to grab images from a camera into a buffer in the host
//    computer's memory, using Sapera++ Acquisition and Buffer objects, and a Transfer 
//    object to link them.  Also, a View object is used to display the buffer.
//
// -----------------------------------------------------------------------------------------

// Disable deprecated function warnings with Visual Studio 2005
#if defined(_MSC_VER) && _MSC_VER >= 1400
#pragma warning(disable: 4995)
#endif

#include "g_camera_dalsa.h"

#include "stdio.h"
#include "conio.h"
#include "math.h"
#include "sapclassbasic.h"
#include "g_camera_dalsa_utils.h"
#include "g_camera_interface.h"
#include "g_camera_callback.h"
#include "logger.h"

#include <process.h>
#include <thread>

#include <vld.h> 


// Restore deprecated function warnings with Visual Studio 2005
#if defined(_MSC_VER) && _MSC_VER >= 1400
#pragma warning(default: 4995)
#endif

// Static Functions
static void XferCallback(SapXferCallbackInfo *pInfo);
static BOOL GetOptions(int argc, char *argv[], char *acqServerName, UINT32 *pAcqDeviceIndex, char *configFileName);
static BOOL GetOptionsFromCommandLine(int argc, char *argv[], char *acqServerName, 
    UINT32 *pAcqDeviceIndex, char *configFileName);

static cCallback*  m_pCallback = nullptr;
static 	ImageBuffer* g_pImageBuffer = nullptr;
static SapAcqDevice* m_pAcqDevice = nullptr;

HANDLE	g_hEventImageReadyDalsa;
HANDLE	g_hEventBusyDalsa;
HANDLE	g_hEventExitDalsa;
HANDLE	g_hEventThreadDoneDalsa;
HANDLE hMutex;
static unsigned long g_lImageCount = 0;
static bool				g_IsPause = false;

static bool g_bExist = false;
static SapBuffer* g_pBuffer = nullptr;

unsigned int __stdcall ImageProcessThreadCallerDalsa(void * pParam);
BOOL SaveGrayScale(const BYTE* image, int width, int height, LPCTSTR fileName);


int CamWidth = 4096;
int CamHeight = 1000;
UINT8* g_pImage = NULL;

// Bitmap information + palette information
struct CUSTOM_BITMAPINFO
{
	// bitmap information
	BITMAPINFOHEADER bmiHeader;
	/// A 256 colors palette
	RGBQUAD bmiColors[256];
};

void Test();


// Function 
unsigned int __stdcall ImageProcessThreadCallerDalsa(void * pParam)
{
	// The ImageThread function of the CCameraJai class is called.
	// The worker thread loops in the ImageThread function.
	((CDalsa *)pParam)->ImageThread(pParam);

	return(0);
}

CDalsa::CDalsa() :m_iStatus(CAMERA_STOPPED), m_pBuffers(nullptr), m_pXfer(nullptr)
, m_hThreadImageProcessing(nullptr)
, m_pAcq(nullptr)
, m_pBuffersWith(nullptr)
//, m_pAcqDevice(nullptr)
, m_Argv(nullptr)
{
    g_hEventImageReadyDalsa = CreateEvent(NULL, TRUE, FALSE, NULL);
    g_hEventBusyDalsa = CreateEvent(NULL, TRUE, FALSE, NULL);
    g_hEventExitDalsa = CreateEvent(NULL, TRUE, FALSE, NULL);
    g_hEventThreadDoneDalsa = CreateEvent(NULL, TRUE, FALSE, NULL);

	hMutex = CreateMutex(NULL, FALSE, "screen");

    m_pAcq = new SapAcquisition();
    m_pBuffersWith = new SapBufferWithTrash();
    m_pAcqDevice = new SapAcqDevice();
	g_pImageBuffer = new ImageBuffer();
	const unsigned int iSize = CamWidth * CamHeight;
	g_pImageBuffer->iHeight = CamHeight;
	g_pImageBuffer->iWidth = CamWidth;
	g_pImageBuffer->pBuffer = new unsigned char[iSize];
	memset(g_pImageBuffer->pBuffer, 128, iSize);

	
	g_pImage = new UINT8[iSize];
	memset(g_pImage, 0, iSize);
}

CDalsa::~CDalsa()
{


	if (CAMERA_STOPPED != m_iStatus)
	{
		CloseCamera();
		UninitialCamera();
	}

	//CloseHandle(m_hThreadImageProcessing);
	// Thread Done event is set.
	//SetEvent(g_hEventThreadDoneDalsa);

    CloseHandle(g_hEventImageReadyDalsa);
    CloseHandle(g_hEventBusyDalsa);
    CloseHandle(g_hEventExitDalsa);
    CloseHandle(g_hEventThreadDoneDalsa);
	CloseHandle(hMutex);

    delete  m_pAcq;
    delete m_pBuffersWith;
    delete m_pAcqDevice;
	m_pAcqDevice = nullptr;

	delete[] g_pImageBuffer->pBuffer;
	g_pImageBuffer->pBuffer = nullptr;
	delete g_pImageBuffer;
	g_pImageBuffer = nullptr;

	delete[] g_pImage;
	g_pImage = nullptr;
}

std::string ExePath()
{
	char buffer[MAX_PATH];
	GetModuleFileName(NULL, buffer, MAX_PATH);
	std::string::size_type pos = std::string(buffer).find_last_of("\\/");
	return std::string(buffer).substr(0, pos);
}

int CDalsa::OpenCamera_i()
{
    UINT32   acqDeviceNumber;
    char*    acqServerName = new char[CORSERVER_MAX_STRLEN];
    char*    configFilename = new char[MAX_PATH];

    //char *a = "gCamera";
    //char *b = "Linea_M4096-7um_1";
    //char *c = "0";
    ////char *d = "C:/Dropbox/1Work/main/trunk/GSMV/bin_debug/dalsa0.ccf";
    //char *d = "dalsa0.ccf";
    //char* cs[] = { a, b, c, d }; // initialized
    m_Argv[0] = "gCamera";
    m_Argv[1] = "Linea_M4096-7um_1";
    m_Argv[2] = "0";
//    m_Argv[3] = "dalsa0.ccf";
	std::string sExePath = ExePath() + "\\dalsa0.ccf";
	
	int nSize = sExePath.size();
	char *cptr = new char[nSize + 1]; // +1 to account for \0 byte
	strcpy_s(cptr, nSize + 1, sExePath.c_str());
	m_Argv[3] = cptr;

   // LOG(INFO) << "CDalsa::OpenCamera " << m_Argv[3];

    //printf("Sapera Console Grab Example (C++ version)\n");

	LOG(INFO) << std::string(cptr);

    // Call GetOptions to determine which acquisition device to use and which config
    // file (CCF) should be loaded to configure it.
    // Note: if this were an MFC-enabled application, we could have replaced the lengthy GetOptions 
    // function with the CAcqConfigDlg dialog of the Sapera++ GUI Classes (see GrabMFC example)
    if (!GetOptions(m_Argc, m_Argv, acqServerName, &acqDeviceNumber, configFilename))
    {
		delete[] cptr;
        printf("\nPress any key to terminate\n");
        CorGetch();
        return 0;
    }
	delete[] cptr;

	//CORSTATUS status; // Declare status code = CorManOpen();
	// Initialize Sapera API
//	status = CorManOpen();

    //SapAcquisition Acq;
    //SapAcqDevice AcqDevice;
    //SapBufferWithTrash Buffers;
    SapTransfer AcqToBuf = SapAcqToBuf(m_pAcq, m_pBuffersWith);
    SapTransfer AcqDeviceToBuf = SapAcqDeviceToBuf(m_pAcqDevice, m_pBuffersWith);

    //SapView View;

    SapLocation loc(acqServerName, acqDeviceNumber);

    if (SapManager::GetResourceCount(acqServerName, SapManager::ResourceAcq) > 0)
    {
        *m_pAcq = SapAcquisition(loc, configFilename);
        *m_pBuffersWith = SapBufferWithTrash(2, m_pAcq);
        //View = SapView(&Buffers, SapHwndAutomatic);
        //AcqToBuf = SapAcqToBuf(&Acq, &Buffers, XferCallback, &View);

        m_pXfer = &AcqToBuf;

        // Create acquisition object
        if (!m_pAcq->Create())
            return -1;//goto FreeHandles;

    }

    else if (SapManager::GetResourceCount(acqServerName, SapManager::ResourceAcqDevice) > 0)
    {
		if (strcmp(configFilename, "NoFile") == 0)
		{
			*m_pAcqDevice = SapAcqDevice(loc, FALSE);
			LOG(INFO) << "NoFile.";
		}
		else
		{
			*m_pAcqDevice = SapAcqDevice(loc, configFilename);
			LOG(INFO) << "configFilename " << configFilename;
		}

        *m_pBuffersWith = SapBufferWithTrash(2, m_pAcqDevice);
        // View = SapView(&Buffers, SapHwndAutomatic);
        //  AcqDeviceToBuf = SapAcqDeviceToBuf(&AcqDevice, &Buffers, XferCallback, &View);

        AcqDeviceToBuf = SapAcqDeviceToBuf(m_pAcqDevice, m_pBuffersWith, CDalsa::XferCallback, m_pBuffersWith);
        // AcqDeviceToBuf = SapAcqDeviceToBuf(&AcqDevice, &Buffers, XferCallback, this);
        m_pXfer = &AcqDeviceToBuf;

        // Create acquisition object
		LOG(INFO) << "Before m_pAcqDevice->Create";
        if (!m_pAcqDevice->Create())
		{
			LOG(INFO) << "m_pAcqDevice->Create failed";
			//UninitialCamera();
			return -1;//goto FreeHandles;
		}
		LOG(INFO) << "After m_pAcqDevice->Create";

    }

    // Create buffer object
    if (!m_pBuffersWith->Create())
        return -1;//goto FreeHandles;

    // Create transfer object
    if (m_pXfer && !m_pXfer->Create())
        return -1;//goto FreeHandles;

    // Create view object
    //if (!View.Create())
    //  goto FreeHandles;

    // Start continous grab
       m_pXfer->Grab();

    //m_pXfer->Wait(20000);

	   BOOL	m_bExit = FALSE;
	   // Event: Exit event, Image Ready event
	   HANDLE	hEvents[] = { g_hEventExitDalsa, g_hEventImageReadyDalsa };
					
	   while (m_iStatus != CAMERA_STOPPED && !m_bExit)
    {
        if (WAIT_OBJECT_0 == WaitForSingleObject(g_hEventExitDalsa, 0))
        {
            break;
        }

		WaitForSingleObject(hMutex, INFINITE);
		//cout << "Fun display!" << endl;
		Sleep(100);
		ReleaseMutex(hMutex);

       // WaitForSingleObject(g_hEventExitDalsa, INFINITE);
        std::cout << "main display!" << std::endl;
        //Sleep(2000);
		//Sleep(100);
       // ReleaseMutex(g_hEventExitDalsa);

		//////////////////////////////////////////////////////////////////////////
		DWORD dwRet = MsgWaitForMultipleObjects(2, hEvents, FALSE, INFINITE, QS_ALLINPUT);
		switch (dwRet)
		{
		case	WAIT_OBJECT_0:		// Exit
			// If it is Exit event, the Exit flag is set.
			m_bExit = TRUE;
			m_iStatus = CAMERA_STOPPED;
			LOG(INFO) << "CAMERA_STOPPED";

// 			CloseHandle(m_hThreadImageProcessing);
// 			// Thread Done event is set.
// 			SetEvent(g_hEventThreadDoneDalsa);

			break;
		case	WAIT_OBJECT_0 + 1:	// Image Ready
			// If it is image Ready event, it sets it the Busy event. 
			SetEvent(g_hEventBusyDalsa);
			// Image Ready event is reset.
			ResetEvent(g_hEventImageReadyDalsa);	
			// if (m_iAfterProcess == 1)
			{
				//BitInvert(&m_tBuffer);
				// Making of Bitmap for display. And, displays.
				// CheckCreateBmp(&m_tBuffer);
				PUINT8 pData;
				int numPix = CamHeight * CamWidth;
				int compIndex = 0;
		
				g_pImageBuffer->iCount = g_lImageCount;
// 				g_pBuffer->GetAddress(compIndex, (void**)&pData);
// 				memcpy(g_pImageBuffer->pBuffer, pData, sizeof(char) * numPix);

				std::string sFilePath = "c:/images/test/";
				sFilePath += std::to_string(g_lImageCount) + ".bmp";

				SaveGrayScale(g_pImage, CamWidth, CamHeight, sFilePath.c_str());

				//LOG(INFO) << "execute begin height:" << g_pImageBuffer->iHeight;
				if (nullptr != m_pCallback)
				{
					m_pCallback->Execute((void*)g_pImageBuffer);
				}
				
				//LOG(INFO) << "execute end height" << g_pImageBuffer->iHeight;


			}

			//   ImageProcessing(&m_tBuffer);

			// The Busy event is reset.
			ResetEvent(g_hEventBusyDalsa);
			break;
		default:
			// Time-out etc.Time-out etc.
			break;
		}

		//////////////////////////////////////////////////////////////////////////
		//for pause camera
		switch (m_iStatus)
		{				  
		case CAMERA_PAUSE:
			{
				LOG(INFO) << "CAMERA_PAUSE";
				if (m_pXfer->IsConnected())
				{
					if (m_pXfer->IsGrabbing())
					{
						m_pXfer->Freeze();
						if (!m_pXfer->Wait(2000))
						{
							printf("Grab could not stop properly.\n");
						}
					}
				}

			}
			break;
		case CAMERA_STOPPED:
		{
			LOG(INFO) << "CAMERA_STOPPED";
			//if (m_pXfer->IsConnected())
			//{
			//}
			//m_pXfer->Freeze();
			//if (!m_pXfer->Wait(2000))
			//{
			//	printf("Grab could not stop properly.\n");
			//}
		//	CloseHandle(m_hThreadImageProcessing);
			// Thread Done event is set.
			SetEvent(g_hEventThreadDoneDalsa);
		}
		
		default:
			break;
		}

// 		if (g_IsPause)
// 		{
// 			LOG(INFO) << "CAMERA_PAUSE";
// 			m_pXfer->Freeze();
// 			if (!m_pXfer->Wait(2000))
// 			{
// 				printf("Grab could not stop properly.\n");
// 			}
// 		}
		
    }
 
	CloseHandle(m_hThreadImageProcessing);
	LOG(INFO) << "CAMERA exit";
    return 0;
}

int CDalsa::OpenCamera(int argc, char* argv[])
{

	//Test();
    m_Argc = argc;
    m_Argv = argv;

	m_iStatus = CAMERA_STARTED;
	LOG(INFO) << "CAMERA_STARTED";
	g_IsPause = false;

	if (nullptr == m_hThreadImageProcessing)
	{
		m_hThreadImageProcessing = (HANDLE)_beginthreadex(
			NULL, 0, ImageProcessThreadCallerDalsa,
			reinterpret_cast<void *>(this), 0, NULL);
	}

    Sleep(2000);

   return 0;
}

void CDalsa::UninitialCamera()
{

    m_iStatus = CAMERA_STOPPED;
	LOG(INFO) << "CAMERA_STOPPED";
    //unregister the acquisition callback
    m_pAcq->UnregisterCallback();

    // Destroy view object
    // if (!View.Destroy()) return FALSE;

    // Destroy transfer object
    if (m_pXfer && *m_pXfer && !m_pXfer->Destroy()) return;

    // Destroy buffer object
    if (!m_pBuffersWith->Destroy()) return;

    // Destroy acquisition object
    if (!m_pAcq->Destroy()) return;

    // Destroy acquisition object
    if (!m_pAcqDevice->Destroy()) return;
}


void CDalsa::CloseCamera()
{
    // Stop grab
   // if (m_pXfer->IsGrabbing())
	WaitForSingleObject(hMutex, INFINITE);
	//cout << "Fun display!" << endl;
	Sleep(1000);
	ReleaseMutex(hMutex);
    {
		m_iStatus = CAMERA_STOPPED;
		LOG(INFO) << "CAMERA_STOPPED";
    }

	SetEvent(g_hEventExitDalsa);
	SetEvent(g_hEventThreadDoneDalsa);
}

void CDalsa::PauseCamera()
{
	g_IsPause = true;
	m_iStatus = CAMERA_PAUSE;
	LOG(INFO) << "CAMERA_STOPPED";
}

 void CDalsa::XferCallback(SapXferCallbackInfo *pInfo)
{

    //LOG(INFO) << "::XferCallback start.";
    printf("XferCallback:%d\n",g_lImageCount);
    if (WAIT_OBJECT_0 == WaitForSingleObject(g_hEventBusyDalsa, 0))
    {
        return;
    }
 //           return;
  //SapView *pView = (SapView *)pInfo->GetContext();
   SapBuffer *pBuffer = (SapBuffer *)pInfo->GetContext();
   if (nullptr == pBuffer)
   {
       std::cout << "pBuffer is null\n";
       return;
   }



   bool bCallback = true;
   // SapBuffer* pBuffer = pView->GetBuffer();
   {
	   PUINT8 pData;
       int numPix = pBuffer->GetWidth() * pBuffer->GetHeight();
	   //g_pBuffer = new SapBuffer((const SapBuffer) *pBuffer);
       //for (int compIndex = 0; compIndex < 1; ++compIndex)

       {
           // Retrieve the data address of the current component
           pBuffer->GetAddress(0, (void**)&pData);
           // Process the component
		   for (int pixIndex = 0; pixIndex < numPix; pixIndex++)
		   {		 
			   g_pImage[pixIndex] = pData[pixIndex];
			   //pData++;
		   }
       }
   }




   //////////////////////////////////////////////////////////////////////////
   // Save the image to disk in TIFF format
   //std::string sFilePath = "c:/images/";
   //sFilePath += std::to_string(g_lImageCount) + ".tiff";
   //char* options = "-format tiff";
   //if (g_lImageCount < 1000)
   //{
   //    pBuffer->Save(sFilePath.c_str(), options);
   //  
   //}

   g_lImageCount++;
   if (g_lImageCount > 999)
   {
       g_lImageCount = 0;
   }
   //std::cout << "image count:" << g_lImageCount << std::endl;
   //LOG(INFO) << "image count " << g_lImageCount;
                                                     

   //////////////////////////////////////////////////////////////////////////
   // refresh framerate
   /*
   static float lastframerate = 0.0f;

   SapTransfer* pXfer = pInfo->GetTransfer();
	if(pXfer->IsParameterValid()  )

       {
           if (pXfer->UpdateFrameRateStatistics())
           {
               SapXferFrameRateInfo* pFrameRateInfo = pXfer->GetFrameRateStatistics();
               float framerate = 0.0f;

               if (pFrameRateInfo->IsLiveFrameRateAvailable())
                   framerate = pFrameRateInfo->GetLiveFrameRate();

               // check if frame rate is stalled
               //This can occur if no new frame is received for 2 seconds.
               if (pFrameRateInfo->IsLiveFrameRateStalled())
               {
                   printf("Live frame rate is stalled.\n");
               }
               // update FPS only if the value changed by +/- 0.1
               else if ((framerate > 0.0f) && (abs(lastframerate - framerate) > 0.1f))
               {
                   printf("Grabbing at %.1f frames/sec\n", framerate);
                   lastframerate = framerate;
               }                                         
           }  
   } */

   SetEvent(g_hEventImageReadyDalsa);
}

static BOOL GetOptions(int argc, char *argv[], char *acqServerName, UINT32 *pAcqDeviceIndex, char *configFileName)
{
   // Check if arguments were passed
   if (argc > 1)
      return GetOptionsFromCommandLine(argc, argv, acqServerName, pAcqDeviceIndex, configFileName);
   else
      return GetOptionsFromQuestions(acqServerName, pAcqDeviceIndex, configFileName);
}

static BOOL GetOptionsFromCommandLine(int argc, char *argv[], char *acqServerName, UINT32 *pAcqDeviceIndex, char *configFileName)
{
   // Check the command line for user commands
   if ((strcmp(argv[1], "/?") == 0) || (strcmp(argv[1], "-?") == 0))
   {
      // print help
      printf("Usage:\n");
      printf("GrabCPP [<acquisition server name> <acquisition device index> <config filename>]\n");
      //Linea_M4096-7um_1 0 T_Linea_M4096-7um_20171113.ccf
      return FALSE;
   }

   // Check if enough arguments were passed
   if (argc < 4)
   {
      printf("Invalid command line!\n");
      return FALSE;
   }

   // Validate server name
   if (SapManager::GetServerIndex(argv[1]) < 0)
   {
      printf("Invalid acquisition server name!\n");
      return FALSE;
   }

   // Does the server support acquisition?
   int deviceCount = SapManager::GetResourceCount(argv[1], SapManager::ResourceAcq);
   int cameraCount = SapManager::GetResourceCount(argv[1], SapManager::ResourceAcqDevice);

   if (deviceCount + cameraCount == 0)
   {
      printf("This server does not support acquisition!\n");
      return FALSE;
   }

   // Validate device index
   if (atoi(argv[2]) < 0 || atoi(argv[2]) >= deviceCount + cameraCount)
   {
      printf("Invalid acquisition device index!\n");
      return FALSE;
   }

   if (cameraCount == 0)
   {
      // Verify that the specified config file exist
      OFSTRUCT of = { 0 };
      if (OpenFile(argv[3], &of, OF_EXIST) == HFILE_ERROR)
      {
         printf("The specified config file (%s) is invalid!\n", argv[3]);
         return FALSE;
      }
   }

   // Fill-in output variables
   CorStrncpy(acqServerName, argv[1], CORSERVER_MAX_STRLEN);
   *pAcqDeviceIndex = atoi(argv[2]);
  // if (cameraCount == 0)
      CorStrncpy(configFileName, argv[3], MAX_PATH);

   return TRUE;
}

void CDalsa::SetCallback(cCallback* pCall)
{
    m_pCallback = pCall;
}




void CDalsa::ImageThread(void * pParam)
{
    OpenCamera_i();
}




//saves an 8 bits-per-pixel image with a gray-scaled palette
BOOL SaveGrayScale(const BYTE* image, int width, int height, LPCTSTR fileName)
{
	//fullwidth must be multiple of 4
	int fullWidth = (width + 3) & 0xfffffffc;
	//copy each scan line in a temporary buffer
	BYTE* bytes = new BYTE[fullWidth * height];
	for (int i = 0; i < height; i++)
	{
		//CopyMemory(bytes + i * fullWidth, image + i * width, width);
		memcpy(bytes + i * fullWidth, image + i * width, width);
	}
	//BMP header
	BITMAPFILEHEADER header;
	ZeroMemory(&header, sizeof(BITMAPFILEHEADER));
	header.bfType = ((WORD)('M' << 8) | 'B');
	header.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(CUSTOM_BITMAPINFO) + fullWidth * height;
	header.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(CUSTOM_BITMAPINFO);
	//Bitmap information
	CUSTOM_BITMAPINFO bitmapInfo;
	ZeroMemory(&bitmapInfo, sizeof(CUSTOM_BITMAPINFO));
	bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitmapInfo.bmiHeader.biPlanes = 1;
	bitmapInfo.bmiHeader.biBitCount = 8;
	bitmapInfo.bmiHeader.biCompression = BI_RGB;
	bitmapInfo.bmiHeader.biSizeImage = 0;
	bitmapInfo.bmiHeader.biWidth = width;
	//negative height for a top-down bitmap
	bitmapInfo.bmiHeader.biHeight = -height;
	//create the gray scale palette
	for (int i = 0; i < 256; i++)
	{
		bitmapInfo.bmiColors[i].rgbRed = i;
		bitmapInfo.bmiColors[i].rgbGreen = i;
		bitmapInfo.bmiColors[i].rgbBlue = i;
		bitmapInfo.bmiColors[i].rgbReserved = 0;
	}
	//save the image
	HANDLE hFile = CreateFile(fileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		delete[] bytes;
		return FALSE;
	}
	DWORD writtenBytes = 0;
	if (!WriteFile(hFile, &header, sizeof(BITMAPFILEHEADER), &writtenBytes, NULL) || writtenBytes != sizeof(BITMAPFILEHEADER)
		|| !WriteFile(hFile, &bitmapInfo, sizeof(CUSTOM_BITMAPINFO), &writtenBytes, NULL) || writtenBytes != sizeof(CUSTOM_BITMAPINFO)
		|| !WriteFile(hFile, bytes, fullWidth * height, &writtenBytes, NULL) || writtenBytes != fullWidth * height)
	{
		delete[] bytes;
		CloseHandle(hFile);
		return FALSE;
	}
	delete[] bytes;
	CloseHandle(hFile);
	return TRUE;
}
//simple test function
void Test()
{
	int width = 199;
	int height = 200;
	BYTE* image = new BYTE[width * height];
	for (int y = 0; y < height; y++)
		for (int x = 0; x < width; x++)
			image[y * width + x] = x;
	SaveGrayScale(image, width, height, "c:\\test.bmp");
	delete[] image;
}