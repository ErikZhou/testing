// ImageProcessingThreadSampleDlg.cpp : implementation file
//

//#include "stdafx.h"
//#include "stdafx.h"    //Always include it first then other files.

#include "g_camera_driver_jai.h"
//#include "ImageWnd.h"
#include <process.h>
#include <VLib.h>
#include <vector>

//#ifdef _DEBUG
//#define new DEBUG_NEW
//#endif

#define TRACE printf

HANDLE	g_hEventImageReady;
HANDLE	g_hEventBusy;
HANDLE	g_hEventExit;
HANDLE	g_hEventThreadDone;

long long g_lImageCount = 0;

//Utility function to set the frame grabber's width/height (if one is present in the system).
void SetFramegrabberValue(CAM_HANDLE hCam, int8_t* szName, int64_t int64Val, int8_t* sCameraId)
{
	//Set frame grabber value, if applicable
	DEV_HANDLE hDev = NULL; //If a frame grabber exists, it is at the GenTL "local device layer".
	J_STATUS_TYPE retval = J_Camera_GetLocalDeviceHandle(hCam, &hDev);
	if(J_ST_SUCCESS != retval)
		return;

	if(NULL == hDev)
		return;

	NODE_HANDLE hNode;
	retval = J_Camera_GetNodeByName(hDev, szName, &hNode);
	if(J_ST_SUCCESS != retval)
		return;

	retval = J_Node_SetValueInt64(hNode, false, int64Val);
	if(J_ST_SUCCESS != retval)
		return;

	//Special handling for Active Silicon CXP boards, which also has nodes prefixed
	//with "Incoming":
	std::string strTransportName((char*)sCameraId);
	if(std::string::npos != strTransportName.find("TLActiveSilicon"))
	{
		std::string strName((char*)szName);
		if(std::string::npos != strName.find("Width") 
			|| std::string::npos != strName.find("Height"))
		{
			std::string strIncoming = "Incoming" + strName;
			NODE_HANDLE hNodeIncoming;
			J_STATUS_TYPE retval = J_Camera_GetNodeByName(hDev, (int8_t*)strIncoming.c_str(), &hNodeIncoming);
			if (retval == J_ST_SUCCESS)
			{
				retval = J_Node_SetValueInt64(hNodeIncoming, false, int64Val);
			}
		}

	}//if(std::string::npos != strTransportName.find("TLActiveSilicon"))
}

//Utility function to set the frame grabber's pixel format (if one is present in the system).
void SetFramegrabberPixelFormat(CAM_HANDLE hCam, int8_t* szName, int64_t jaiPixelFormat, int8_t* sCameraId)
{
	DEV_HANDLE hDev = NULL; //If a frame grabber exists, it is at the GenTL "local device layer".
	J_STATUS_TYPE retval = J_Camera_GetLocalDeviceHandle(hCam, &hDev);
	if(J_ST_SUCCESS != retval)
		return;

	if(NULL == hDev)
		return;

	int8_t szJaiPixelFormatName[512];
	uint32_t iSize = 512;
	retval = J_Image_Get_PixelFormatName(hCam, jaiPixelFormat, szJaiPixelFormatName, iSize);
	if(J_ST_SUCCESS != retval)
		return;

	NODE_HANDLE hLocalDeviceNode = 0;
	retval = J_Camera_GetNodeByName(hDev, (int8_t *)"PixelFormat", &hLocalDeviceNode);
	if(J_ST_SUCCESS != retval)
		return;

	if(0 == hLocalDeviceNode)
		return;

	//NOTE: this may fail if the camera and/or frame grabber does not use the SFNC naming convention for pixel formats!
	//Check the camera and frame grabber for details.
	retval = J_Node_SetValueString(hLocalDeviceNode, false, szJaiPixelFormatName);
	if(J_ST_SUCCESS != retval)
		return;

	//Special handling for Active Silicon CXP boards, which also has nodes prefixed
	//with "Incoming":
	std::string strTransportName((char*)sCameraId);
	if(std::string::npos != strTransportName.find("TLActiveSilicon"))
	{
		std::string strIncoming = std::string("Incoming") + std::string((char*)szName);
		NODE_HANDLE hNodeIncoming;
		J_STATUS_TYPE retval = J_Camera_GetNodeByName(hDev, (int8_t*)strIncoming.c_str(), &hNodeIncoming);
		if (retval == J_ST_SUCCESS)
		{
			//NOTE: this may fail if the camera and/or frame grabber does not use the SFNC naming convention for pixel formats!
			//Check the camera and frame grabber for details.
			retval = J_Node_SetValueString(hNodeIncoming, false, szJaiPixelFormatName);
		}
	}
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
// Image Processing Thread
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
// Prototype declaration
unsigned int __stdcall ImageProcessThreadCaller( void * pParam );

// Function 
unsigned int __stdcall ImageProcessThreadCaller(void * pParam)
{
    // The ImageThread function of the CCameraJai class is called.
    // The worker thread loops in the ImageThread function.
    ((CCameraJai *)pParam)->ImageThread(pParam);

    return(0);
}


// CCameraJai dialog

//--------------------------------------------------------------------------------------------------
// Constructor
//--------------------------------------------------------------------------------------------------
CCameraJai::CCameraJai()
:m_hBitmap(NULL)
, m_iAfterProcess(1),
m_hThreadImageProcessing(nullptr),
m_pCallback(nullptr)
{
 //   m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

    m_hFactory = NULL;
    m_hCam = NULL;
    m_hThread = NULL;

    g_hEventImageReady = CreateEvent(NULL, TRUE, FALSE, NULL);
    g_hEventBusy = CreateEvent(NULL, TRUE, FALSE, NULL);
    g_hEventExit = CreateEvent(NULL, TRUE, FALSE, NULL);
    g_hEventThreadDone = CreateEvent(NULL, TRUE, FALSE, NULL);

    m_tBuffer.pImageBuffer = NULL;
    m_tDibBuffer.pImageBuffer = NULL;

	m_iWidthInc = 1;
	m_iHeightInc = 1;
	m_bEnableStreaming = false;

    m_iStatus = CAMERA_STOPPED;
}

CCameraJai::~CCameraJai()
{
    CloseHandle(g_hEventImageReady);
    CloseHandle(g_hEventBusy);
    CloseHandle(g_hEventExit);
    CloseHandle(g_hEventThreadDone);
    CloseHandle(m_hThreadImageProcessing);

    printf("~CCameraJai\n");
}



//--------------------------------------------------------------------------------------------------
// OnInitDialog
//--------------------------------------------------------------------------------------------------
BOOL CCameraJai::OnInitDialog()
{


    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
 //   SetIcon(m_hIcon, TRUE);			// Set big icon
  //  SetIcon(m_hIcon, FALSE);		// Set small icon

    ////////////////////////////////////////////////////////////////////////////////////
    // Starts Image Processing Thread
    ////////////////////////////////////////////////////////////////////////////////////
    m_hThreadImageProcessing = (HANDLE)_beginthreadex(NULL, 0, ImageProcessThreadCaller, 
        reinterpret_cast<void *>(this), 0, NULL);
    ////////////////////////////////////////////////////////////////////////////////////

    RECT	tImageRect = {4, 240, WIN_WIDTH + 4, WIN_HEIGHT + 240};

    //m_pImageWnd = new CImageWnd(this);
    //m_pImageWnd->Create(NULL, _T("Image Window"), WS_CHILD | WS_VISIBLE, tImageRect, this, 3001);

    BOOL retval;

    // Open factory & camera
    retval = OpenFactoryAndCamera();  
    //if (retval)
    //{
    //    GetDlgItem(IDC_CAMERAID)->SetWindowText(CString((char*)m_sCameraId));    // Display camera ID
    //    EnableControls(TRUE, FALSE);   // Enable Controls
    //}
    //else
    //{
    //    GetDlgItem(IDC_CAMERAID)->SetWindowText(CString("error"));
    //    EnableControls(FALSE, FALSE);  // Enable Controls
    //}
    if (!retval)
    {
        printf("Failed to OpenFactoryAndCamera.\n");
        return false;
    }
    InitializeControls();   // Initialize Controls


    //////////////////////////////////////////////////////////////////////////
    //OnBnClickedStart
    Sleep(2000);
    OnBnClickedStart();
    return TRUE;  // return TRUE  unless you set the focus to a control
}
//--------------------------------------------------------------------------------------------------
// OnDestroy
//--------------------------------------------------------------------------------------------------
void CCameraJai::On_Destroy()
{
//    CDialog::OnDestroy();

    // TODO: Add your message handler code here

    // Stop acquisition
    OnBnClickedStop();

    // Close factory & camera
    CloseFactoryAndCamera();

    //if(m_hBitmap)
    //    ::DeleteObject(m_hBitmap);

    //if(m_pImageWnd)
    //    delete m_pImageWnd;
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.
int CCameraJai::OpenCamera()
{
    this->OnInitDialog();
    return 0;
}

CAMERA_STATUS CCameraJai::GetCameraStatus() const
{
    return m_iStatus;
}

int CCameraJai::CloseCamera()
{
    this->OnBnClickedStop();
    this->CloseFactoryAndCamera();

	m_iStatus = CAMERA_PAUSE;
    return 0;
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.

void CCameraJai::ShowErrorMsg(CString message, J_STATUS_TYPE error)
{
    CString errorMsg;
    errorMsg.Format(_T("%s: Error = %d: "), message, error);

    switch(error)
    {
    case J_ST_INVALID_BUFFER_SIZE:	errorMsg += "Invalid buffer size ";	                break;
    case J_ST_INVALID_HANDLE:		errorMsg += "Invalid handle ";		                break;
    case J_ST_INVALID_ID:			errorMsg += "Invalid ID ";			                break;
    case J_ST_ACCESS_DENIED:		errorMsg += "Access denied ";		                break;
    case J_ST_NO_DATA:				errorMsg += "No data ";				                break;
    case J_ST_ERROR:				errorMsg += "Generic error ";		                break;
    case J_ST_INVALID_PARAMETER:	errorMsg += "Invalid parameter ";	                break;
    case J_ST_TIMEOUT:				errorMsg += "Timeout ";				                break;
    case J_ST_INVALID_FILENAME:		errorMsg += "Invalid file name ";	                break;
    case J_ST_INVALID_ADDRESS:		errorMsg += "Invalid address ";		                break;
    case J_ST_FILE_IO:				errorMsg += "File IO error ";		                break;
    case J_ST_GC_ERROR:				errorMsg += "GenICam error ";		                break;
    case J_ST_VALIDATION_ERROR:		errorMsg += "Settings File Validation Error ";		break;
    case J_ST_VALIDATION_WARNING:	errorMsg += "Settings File Validation Warning ";    break;
    }

   // AfxMessageBox(errorMsg, MB_OKCANCEL | MB_ICONINFORMATION);
    printf("%s\n",errorMsg);

}


//--------------------------------------------------------------------------------------------------
// OpenFactoryAndCamera
//--------------------------------------------------------------------------------------------------
BOOL CCameraJai::OpenFactoryAndCamera()
{
	J_STATUS_TYPE   retval;
	uint32_t        iSize;
	uint32_t        iNumDev;
	bool8_t         bHasChange;

	m_bEnableStreaming = false;

	// Open factory
    retval = J_Factory_Open((int8_t*)"", &m_hFactory);
	if (retval != J_ST_SUCCESS)
	{
		ShowErrorMsg(CString("Could not open factory!"), retval);
		return FALSE;
	}
	TRACE("Opening factory succeeded\n");

	//Update camera list
	retval = J_Factory_UpdateCameraList(m_hFactory, &bHasChange);
	if (retval != J_ST_SUCCESS)
	{
		ShowErrorMsg(CString("Could not update camera list!"), retval);
		return FALSE;
	}
	TRACE("Updating camera list succeeded\n");

	// Get the number of Cameras
	retval = J_Factory_GetNumOfCameras(m_hFactory, &iNumDev);
	if (retval != J_ST_SUCCESS)
	{
		ShowErrorMsg(CString("Could not get the number of cameras!"), retval);
		return FALSE;
	}

	if (iNumDev == 0)
	{
		ShowErrorMsg(CString("Invalid number of cameras!"), retval);
		return FALSE;
	}
	TRACE("%d cameras were found\n", iNumDev);

	// Get camera ID
	iSize = (uint32_t)sizeof(m_sCameraId);
	retval = J_Factory_GetCameraIDByIndex(m_hFactory, 0, m_sCameraId, &iSize);
	if (retval != J_ST_SUCCESS)
	{
		ShowErrorMsg(CString("Could not get the camera ID!"), retval);
		return FALSE;
	}
	TRACE("Camera ID: %s\n", m_sCameraId);

	// Open camera
	retval = J_Camera_Open(m_hFactory, m_sCameraId, &m_hCam);
	if (retval != J_ST_SUCCESS)
	{
		ShowErrorMsg(CString("Could not open the camera!"), retval);
		return FALSE;
	}

	//Make sure streaming is supported!
	uint32_t numStreams = 0;
	retval = J_Camera_GetNumOfDataStreams(m_hCam, &numStreams);
	if (retval != J_ST_SUCCESS)
	{
		ShowErrorMsg(CString("Error with J_Camera_GetNumOfDataStreams."), retval);
		return FALSE;
	}

	if(0 == numStreams)
	{
		m_bEnableStreaming = false;
	}
	else
	{
		m_bEnableStreaming = true;
	}

	if(iNumDev > 0 && 0 != m_hCam)
	{
		TRACE("Opening camera succeeded\n");
	}
	else
	{
		ShowErrorMsg(CString("Invalid number of Devices!"), iNumDev);
		return FALSE;
	}



	return TRUE;
}

//--------------------------------------------------------------------------------------------------
// CloseFactoryAndCamera
//--------------------------------------------------------------------------------------------------
void CCameraJai::CloseFactoryAndCamera()
{
    J_STATUS_TYPE   retval;

    if (m_hCam)
    {
        // Close camera
        retval = J_Camera_Close(m_hCam);
        if (retval != J_ST_SUCCESS)
        {
            ShowErrorMsg(CString("Could not close the camera!"), retval);
        }
        m_hCam = NULL;
        TRACE("Closed camera\n");
    }

    if (m_hFactory)
    {
        // Close factory
        retval = J_Factory_Close(m_hFactory);
        if (retval != J_ST_SUCCESS)
        {
            ShowErrorMsg(CString("Could not close the factory!"), retval);
        }
        m_hFactory = NULL;
        TRACE("Closed factory\n");
    }

    //SetEvent(g_hEventExit);
}
//--------------------------------------------------------------------------------------------------
// OnBnClickedStart
//--------------------------------------------------------------------------------------------------
void CCameraJai::OnBnClickedStart()
{
    J_STATUS_TYPE   retval;
    int64_t int64Val;

    SIZE	ViewSize;
    POINT	TopLeft;

	if(!m_bEnableStreaming)
	{
		ShowErrorMsg(CString("Streaming not enabled on this device."), 0);
		return;
	}

    // Get Width from the camera
    retval = J_Camera_GetValueInt64(m_hCam, NODE_NAME_WIDTH, &int64Val);
    if (retval != J_ST_SUCCESS)
    {
        ShowErrorMsg(CString("Could not get Width!"), retval);
        return;
    }
    ViewSize.cx = (LONG)int64Val;     // Set window size cx

	//Set frame grabber dimension, if applicable
	SetFramegrabberValue(m_hCam, NODE_NAME_WIDTH, int64Val, &m_sCameraId[0]);

    // Get Height from the camera
    retval = J_Camera_GetValueInt64(m_hCam, NODE_NAME_HEIGHT, &int64Val);
    if (retval != J_ST_SUCCESS)
    {
        ShowErrorMsg(CString("Could not get Height!"), retval);
        return;
    }
    ViewSize.cy = (LONG)int64Val;     // Set window size cy

	//Set frame grabber dimension, if applicable
	SetFramegrabberValue(m_hCam, NODE_NAME_HEIGHT, int64Val, &m_sCameraId[0]);

    // Set window position
    TopLeft.x = 100;
    TopLeft.y = 50;

	// Get the pixelformat from the camera
	int64_t pixelFormat = 0;
	uint64_t jaiPixelFormat = 0;
	retval = J_Camera_GetValueInt64(m_hCam, NODE_NAME_PIXELFORMAT, &pixelFormat);
	J_Image_Get_PixelFormat(m_hCam, pixelFormat, &jaiPixelFormat);

	//Set frame grabber pixel format, if applicable
	SetFramegrabberPixelFormat(m_hCam, NODE_NAME_PIXELFORMAT, pixelFormat, &m_sCameraId[0]);

	// Calculate number of bits (not bytes) per pixel using macro
	int bpp = J_BitsPerPixel(jaiPixelFormat);

    // Open stream
    retval = J_Image_OpenStream(m_hCam, 0, 
        reinterpret_cast<J_IMG_CALLBACK_OBJECT>(this),
        reinterpret_cast<J_IMG_CALLBACK_FUNCTION>(&CCameraJai::StreamCBFunc),
        &m_hThread, (ViewSize.cx*ViewSize.cy*bpp)/8);
    if (retval != J_ST_SUCCESS) {
        ShowErrorMsg(CString("Could not open stream!"), retval);
        return;
    }
    TRACE("Opening stream succeeded\n");

    // Start Acquisition
    retval = J_Camera_ExecuteCommand(m_hCam, NODE_NAME_ACQSTART);
    if (retval != J_ST_SUCCESS)
    {
        ShowErrorMsg(CString("Could not Start Acquisition!"), retval);
        return;
    }

	m_iStatus = CAMERA_STARTED;
}

//--------------------------------------------------------------------------------------------------
// OnBnClickedStop
//--------------------------------------------------------------------------------------------------
void CCameraJai::OnBnClickedStop()
{
    J_STATUS_TYPE retval;

	if(!m_bEnableStreaming)
	{
		return;
	}

    // Stop Acquisition
    if (m_hCam) {
        retval = J_Camera_ExecuteCommand(m_hCam, NODE_NAME_ACQSTOP);
        if (retval != J_ST_SUCCESS)
        {
            ShowErrorMsg(CString("Could not Stop Acquisition!"), retval);
        }
    }

    if(m_hThread)
    {
        // Close stream
        retval = J_Image_CloseStream(m_hThread);
        if (retval != J_ST_SUCCESS)
        {
            ShowErrorMsg(CString("Could not close Stream!"), retval);
        }
        m_hThread = NULL;
        TRACE("Closed stream\n");
    }
}
//--------------------------------------------------------------------------------------------------
// InitializeControls
//--------------------------------------------------------------------------------------------------
void CCameraJai::InitializeControls()
{
    J_STATUS_TYPE   retval;
    NODE_HANDLE hNode;
    int64_t int64Val;

//    CSliderCtrl* pSCtrl;

	//- Width ------------------------------------------------

	// Get SliderCtrl for Width
//	pSCtrl = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_WIDTH);
	if(m_bEnableStreaming)
	{
		// Get Width Node
		retval = J_Camera_GetNodeByName(m_hCam, NODE_NAME_WIDTH, &hNode);
		if (retval == J_ST_SUCCESS)
		{
			// Get/Set Min
			retval = J_Node_GetMinInt64(hNode, &int64Val);
			//pSCtrl->SetRangeMin((int)int64Val, TRUE);

			// Get/Set Max
			retval = J_Node_GetMaxInt64(hNode, &int64Val);
			//pSCtrl->SetRangeMax((int)int64Val, TRUE);

			// Get/Set Value
			retval = J_Node_GetValueInt64(hNode, FALSE, &int64Val);
			//pSCtrl->SetPos((int)int64Val);

//			SetDlgItemInt(IDC_WIDTH, (int)int64Val);

			retval = J_Node_GetInc(hNode, &m_iWidthInc);
			m_iWidthInc = max(1, m_iWidthInc);

			//Set frame grabber dimension, if applicable
			SetFramegrabberValue(m_hCam, NODE_NAME_WIDTH, int64Val, &m_sCameraId[0]);
		}
		else
		{
			ShowErrorMsg(CString("Could not get Width node!"), retval);

			//pSCtrl->ShowWindow(SW_HIDE);
//			GetDlgItem(IDC_LBL_WIDTH)->ShowWindow(SW_HIDE);
//			GetDlgItem(IDC_WIDTH)->ShowWindow(SW_HIDE);
		}
	}
	else
	{
		//pSCtrl->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_LBL_WIDTH)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_WIDTH)->ShowWindow(SW_HIDE);
	}

	//- Height -----------------------------------------------
	// Get SliderCtrl for Height
	//pSCtrl = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_HEIGHT);

	if(m_bEnableStreaming)
	{
		// Get Height Node
		retval = J_Camera_GetNodeByName(m_hCam, NODE_NAME_HEIGHT, &hNode);
		if (retval == J_ST_SUCCESS && m_bEnableStreaming)
		{
			// Get/Set Min
			retval = J_Node_GetMinInt64(hNode, &int64Val);
			//pSCtrl->SetRangeMin((int)int64Val, TRUE);

			// Get/Set Max
			retval = J_Node_GetMaxInt64(hNode, &int64Val);
			//pSCtrl->SetRangeMax((int)int64Val, TRUE);

			// Get/Set Value
			retval = J_Node_GetValueInt64(hNode, FALSE, &int64Val);
		//	pSCtrl->SetPos((int)int64Val);

		//	SetDlgItemInt(IDC_HEIGHT, (int)int64Val);

			retval = J_Node_GetInc(hNode, &m_iHeightInc);
			m_iHeightInc = max(1, m_iHeightInc);

			//Set frame grabber dimension, if applicable
			SetFramegrabberValue(m_hCam, NODE_NAME_HEIGHT, int64Val, &m_sCameraId[0]);
		}
		else
		{
			ShowErrorMsg(CString("Could not get Height node!"), retval);

			//pSCtrl->ShowWindow(SW_HIDE);
			//GetDlgItem(IDC_LBL_HEIGHT)->ShowWindow(SW_HIDE);
			//GetDlgItem(IDC_HEIGHT)->ShowWindow(SW_HIDE);
		}
	}
	else
	{
		//pSCtrl->ShowWindow(SW_HIDE);
		//GetDlgItem(IDC_LBL_HEIGHT)->ShowWindow(SW_HIDE);
		//GetDlgItem(IDC_HEIGHT)->ShowWindow(SW_HIDE);
	}

    //- Gain -----------------------------------------------

    // Get SliderCtrl for Gain
   // pSCtrl = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_GAIN);

    // Get Gain Node
    retval = J_Camera_GetNodeByName(m_hCam, NODE_NAME_GAIN, &hNode);
    if (retval == J_ST_SUCCESS)
    {
        // Get/Set Min
        retval = J_Node_GetMinInt64(hNode, &int64Val);
       // pSCtrl->SetRangeMin((int)int64Val, TRUE);

        // Get/Set Max
        retval = J_Node_GetMaxInt64(hNode, &int64Val);
      //  pSCtrl->SetRangeMax((int)int64Val, TRUE);

        // Get/Set Value
        retval = J_Node_GetValueInt64(hNode, FALSE, &int64Val);
       // pSCtrl->SetPos((int)int64Val);

       // SetDlgItemInt(IDC_GAIN, (int)int64Val);
    }
    else
    {
        //ShowErrorMsg(CString("Could not get Gain node!"), retval);

     //   pSCtrl->ShowWindow(SW_HIDE);
        //GetDlgItem(IDC_LBL_GAIN)->ShowWindow(SW_HIDE);
        //GetDlgItem(IDC_GAIN)->ShowWindow(SW_HIDE);
    }
}

//--------------------------------------------------------------------------------------------------
// OnHScroll
//--------------------------------------------------------------------------------------------------
void CCameraJai::OnHScroll(UINT nSBCode, UINT nPos)
{
    /*
    // Get SliderCtrl for Width
//    CSliderCtrl* pSCtrl;
    int iPos;
    J_STATUS_TYPE   retval;
    int64_t int64Val;

	//- Width ------------------------------------------------

	// Get SliderCtrl for Width
//	pSCtrl = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_WIDTH);
	//if (pSCtrl == (CSliderCtrl*)pScrollBar) 
    {

		iPos = pSCtrl->GetPos();
		int64Val = (int64_t)iPos;

		//To avoid J_Camera_SetValueInt64 errors,
		//make the slider value be an even multiple of the increment + min value
		int64_t inc = m_iWidthInc;
		if(inc > 1)
		{
			int64_t iMin = pSCtrl->GetRangeMin();

			int64_t diff = int64Val - iMin;

			ldiv_t divResult = div((long)diff, ( long)inc);
			int64Val = iMin + (divResult.quot * inc);

			if(iPos != (int)int64Val)
			{
				pSCtrl->SetPos((int)int64Val);
			}
		}

		// Set Width value
		retval = J_Camera_SetValueInt64(m_hCam, NODE_NAME_WIDTH, int64Val);
		if (retval != J_ST_SUCCESS)
		{
			ShowErrorMsg(CString("Could not set Width!"), retval);
		}

		//Set frame grabber dimension, if applicable
		SetFramegrabberValue(m_hCam, NODE_NAME_WIDTH, int64Val, &m_sCameraId[0]);

//		SetDlgItemInt(IDC_WIDTH, iPos);
	}

	//- Height -----------------------------------------------

	// Get SliderCtrl for Height
//	pSCtrl = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_HEIGHT);
	//if (pSCtrl == (CSliderCtrl*)pScrollBar)
    {

		iPos = pSCtrl->GetPos();
		int64Val = (int64_t)iPos;

		//To avoid J_Camera_SetValueInt64 errors,
		//make the slider value be an even multiple of the increment + min value
		int64_t inc = m_iHeightInc;
		if(inc > 1)
		{
			int64_t iMin = pSCtrl->GetRangeMin();

			int64_t diff = int64Val - iMin;

			ldiv_t divResult = div((long)diff, ( long)inc);
			int64Val = iMin + (divResult.quot * inc);

			if(iPos != (int)int64Val)
			{
				pSCtrl->SetPos((int)int64Val);
			}
		}

		// Set Height Value
		retval = J_Camera_SetValueInt64(m_hCam, NODE_NAME_HEIGHT, int64Val);
		if (retval != J_ST_SUCCESS)
		{
			ShowErrorMsg(CString("Could not set Height!"), retval);
		}

		//Set frame grabber dimension, if applicable
		SetFramegrabberValue(m_hCam, NODE_NAME_HEIGHT, int64Val, &m_sCameraId[0]);

//		SetDlgItemInt(IDC_HEIGHT, iPos);
	}

    //- Gain -----------------------------------------------

    // Get SliderCtrl for Gain
//    pSCtrl = (CSliderCtrl*)GetDlgItem(IDC_SLIDER_GAIN);
    //if (pSCtrl == (CSliderCtrl*)pScrollBar)
    {

        iPos = pSCtrl->GetPos();
        int64Val = (int64_t)iPos;

        // Set Gain value
        retval = J_Camera_SetValueInt64(m_hCam, NODE_NAME_GAIN, int64Val);
        if (retval != J_ST_SUCCESS)
        {
            ShowErrorMsg(CString("Could not set Gain!"), retval);
        }

//        SetDlgItemInt(IDC_GAIN, iPos);
    }

//    CDialog::OnHScroll(nSBCode, nPos, pScrollBar);

    */
}

void CCameraJai::OnBnClickedAfterProcess()
{
    m_iAfterProcess = 1;// m_oAfterProcess.GetCheck();
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
// StreamCBFunc : This function comes off without processing anything if the back thread is Busy.
// Moreover, it doesn't display from this function, and the image for internal processing is made.
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
void CCameraJai::StreamCBFunc(J_tIMAGE_INFO * pAqImageInfo)
{
    // The state of the processing thread is checked. It returns without processing anything if it is Busy.
   //printf("StreamCBFunc:%d\n",g_lImageCount);
    if(WAIT_OBJECT_0 == WaitForSingleObject(g_hEventBusy, 0))
        return;

    // The state of the buffer is confirmed.
    if(m_tBuffer.pImageBuffer != NULL)
    {
        // Already when the buffer exists, and the size is different:
        if((m_tBuffer.iSizeX != pAqImageInfo->iSizeX) || (m_tBuffer.iSizeY != pAqImageInfo->iSizeY))
        {
            // Abandons the buffer.
            J_Image_Free(&m_tBuffer);
            m_tBuffer.pImageBuffer = NULL;
        }
    }

    m_pOriginalImage = pAqImageInfo;

    // Allocates it when there is no buffer.
    if(m_tBuffer.pImageBuffer == NULL)
        J_Image_Malloc(pAqImageInfo, &m_tBuffer);

    // The image making is done for the picture processing.
    //J_Image_FromRawToImage(pAqImageInfo, &m_tBuffer);
    //J_Image_FromRawToImageEx(pAqImageInfo, &m_tBuffer, BAYER_STANDARD_MULTI);
    J_Image_FromRawToImageEx(pAqImageInfo, &m_tBuffer, BAYER_STANDARD);

    // Preparation completion. Event setting.
    SetEvent(g_hEventImageReady);

    //BitInvert(&m_tBuffer);
    //if (g_lImageCount >= 9999 )
    //{
    //    SetEvent(g_hEventExit);
    //}
}

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
// Image Processing Thread
// The picture processing and the display are executed by this thread.
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
void CCameraJai::ImageThread(void * pParam)
{
    BOOL	m_bExit = FALSE;
    // Event: Exit event, Image Ready event
    HANDLE	hEvents[] = {g_hEventExit, g_hEventImageReady};

    
    // Event waiting loop
    do
    {
        // The event is waited for.
        DWORD dwRet = WaitForMultipleObjects(2, hEvents, FALSE, 2000);
        switch (dwRet)
        {
        case	WAIT_OBJECT_0:		// Exit
            // If it is Exit event, the Exit flag is set.
            m_bExit = TRUE;
            m_iStatus = CAMERA_STOPPED;
            break;
        case	WAIT_OBJECT_0+1:	// Image Ready
            // If it is image Ready event, it sets it the Busy event. 
            SetEvent(g_hEventBusy);
            // Image Ready event is reset.
            ResetEvent(g_hEventImageReady);

            if(m_iAfterProcess == 0)
            {
                // Making of Bitmap for display. And, displays.
                //CheckCreateBmp(&m_tBuffer);
            }

            // The image processing is written here.
            // There is information in the image for the image processing in m_tBuffer.
            // Processing example: Image brightness reversing
            if(m_iAfterProcess == 1)
            {
                //BitInvert(&m_tBuffer);
                // Making of Bitmap for display. And, displays.
               // CheckCreateBmp(&m_tBuffer);
            }

            ImageProcessing(&m_tBuffer);

            // The Busy event is reset.
            ResetEvent(g_hEventBusy);
            break;
        default:
            // Time-out etc.Time-out etc.
            break;
        }
        // It comes off the loop if the Exit flag stands.
    } while(m_bExit == FALSE);

    printf("CCameraJai::ImageThread exit\n");

   // _endthreadex((unsigned int)m_hThreadImageProcessing);
    //_endthreadex(0);
    CloseHandle(m_hThreadImageProcessing);
    // Thread Done event is set.
    SetEvent(g_hEventThreadDone);
}


//--------------------------------------------------------------------------------------------------
// Making of Bitmap for display. And, displays.
// Technique for displaying image without using display function of SDK.
//--------------------------------------------------------------------------------------------------
/*
void CCameraJai::CheckCreateBmp(J_tIMAGE_INFO * pImageInfo)
{
    // If DIB is defined
    if(m_hBitmap != NULL)
    {
        // If the size is different...
        if((pImageInfo->iSizeX != m_tBmp.bmiHeader.biWidth) || (pImageInfo->iSizeY != m_tBmp.bmiHeader.biHeight))
        {
            // Deletes it.
            DeleteObject(m_hBitmap);
            m_hBitmap = NULL;
        }
    }

    // If DIB is undefined
    if(m_hBitmap == NULL)
    {
        // DIB information setting
        m_tBmp.bmiHeader.biSize		= sizeof(BITMAPINFOHEADER);
        m_tBmp.bmiHeader.biBitCount	= 32;				// Alpha/R/G/B
        m_tBmp.bmiHeader.biPlanes	= 1;
        m_tBmp.bmiHeader.biWidth	= pImageInfo->iSizeX;
        m_tBmp.bmiHeader.biHeight	= 0 - pImageInfo->iSizeY;	// Upper and lower reversing
        m_tBmp.bmiHeader.biCompression = BI_RGB;

        // DIB is made and the address of DIB is obtained as image buffer substance of J_tIMAGE_INFO. 
        m_hBitmap = CreateDIBSection(NULL, &m_tBmp, DIB_RGB_COLORS, reinterpret_cast<void **>(&m_tDibBuffer.pImageBuffer), NULL, 0);

        // Other information on J_tIMAGE_INFO is set.
        m_tDibBuffer.iPixelType	= J_GVSP_PIX_BGRA8_PACKED;
        m_tDibBuffer.iSizeX		= m_tBmp.bmiHeader.biWidth;
        m_tDibBuffer.iSizeY		= 0 - m_tBmp.bmiHeader.biHeight;	// Upper and lower reversing again.
        m_tDibBuffer.iImageSize	= m_tBmp.bmiHeader.biWidth * m_tBmp.bmiHeader.biHeight * 4;
    }

    // The image for the image processing is converted into DIB.
    J_Image_FromRawToDIB(pImageInfo, &m_tDibBuffer);

    // The WM_PAINT message is issued for the image display.
    //m_pImageWnd->Invalidate();
}

*/

//--------------------------------------------------------------------------------------------------
// Sample image processing  Brightness reversing
//--------------------------------------------------------------------------------------------------
void CCameraJai::BitInvert(J_tIMAGE_INFO * pImageInfo)
{
    g_lImageCount++;
    printf("CCameraJai::BitInvert %d\n", g_lImageCount);
    int	iByteWord;
    int	iPixelStep;
    uint8_t		*pPixel8;
    uint16_t	*pPixel16;

    switch(pImageInfo->iPixelType & J_GVSP_PIX_EFFECTIVE_PIXEL_SIZE_MASK)
    {
    case	J_GVSP_PIX_OCCUPY8BIT:
        iByteWord = 0;
        iPixelStep = 1;
        break;
    case	J_GVSP_PIX_OCCUPY16BIT:
        iByteWord = 1;
        iPixelStep = 1;
        break;
    case	J_GVSP_PIX_OCCUPY24BIT:
        iByteWord = 0;
        iPixelStep = 3;
        break;
    case	J_GVSP_PIX_OCCUPY48BIT:
        iByteWord = 1;
        iPixelStep = 3;
        break;
    }

    unsigned int	iX;
    unsigned int	iY;

    if(iByteWord == 0)
    {	// 8bit
        for(iY = 0 ; iY < pImageInfo->iSizeY ; iY++)
        {
            pPixel8 = pImageInfo->pImageBuffer + pImageInfo->iSizeX * iY * iPixelStep;
            for(iX = 0 ; iX < (pImageInfo->iSizeX * iPixelStep) ; iX++)
            {
                *pPixel8 = ~*pPixel8;
                pPixel8++;
            }
        }
    }
    else
    {	// 16bit
        for(iY = 0 ; iY < pImageInfo->iSizeY ; iY++)
        {
            pPixel16 = reinterpret_cast<uint16_t *>(pImageInfo->pImageBuffer) + pImageInfo->iSizeX * iY * iPixelStep;
            for(iX = 0 ; iX < (pImageInfo->iSizeX * iPixelStep) ; iX++)
            {
                *pPixel16 = ~*pPixel16;
                pPixel16++;
            }
        }
    }

    // Save the image to disk in TIFF format
    CString sFilePath =_T("c:/images/");
    sFilePath += (CString)(std::to_string(g_lImageCount).c_str()) + _T(".tiff");
    //printf("image:%s\n", sFilePath);

    if (J_Image_SaveFile(pImageInfo, (LPCWSTR)sFilePath) != J_ST_SUCCESS)
    {
        printf("Failed to save file:%s",sFilePath);
    }
}

void CCameraJai::ImageProcessing(J_tIMAGE_INFO * pImageInfo)
{
    g_lImageCount++;
    //printf("CCameraJai::ImageProcessing %d\n", g_lImageCount);

    // Save the image to disk in TIFF format
    CString sFilePath = _T("c:/images/");
    sFilePath += (CString)(std::to_string(g_lImageCount).c_str()) + _T(".tiff");
    //printf("image:%s\n", sFilePath);
    std::string sPath((CStringA)sFilePath);
    std::cout << "image[" << g_lImageCount << "] " << sPath << "\n";
    if (g_lImageCount < 20)
    {
        if (J_Image_SaveFile(pImageInfo, (LPCWSTR)sFilePath) != J_ST_SUCCESS)
        {
            printf("Failed to save file:%s", sFilePath);
        }
    }


    if (nullptr != m_pCallback)
    {
        // 8bit
        unsigned int	iX;
        unsigned int	iY;
        int	iByteWord;
        int	iPixelStep;
        uint8_t		*pPixel8 = nullptr;


        switch (pImageInfo->iPixelType & J_GVSP_PIX_EFFECTIVE_PIXEL_SIZE_MASK)
        {
        case	J_GVSP_PIX_OCCUPY8BIT:
            iByteWord = 0;
            iPixelStep = 1;
            break;
        case	J_GVSP_PIX_OCCUPY16BIT:
            iByteWord = 1;
            iPixelStep = 1;
            break;
        case	J_GVSP_PIX_OCCUPY24BIT:
            iByteWord = 0;
            iPixelStep = 3;
            break;
        case	J_GVSP_PIX_OCCUPY48BIT:
            iByteWord = 1;
            iPixelStep = 3;
            break;
        }
        ImageBuffer pImageBuffer;
      
        pImageBuffer.iWidth = pImageInfo->iSizeX * iPixelStep;
        pImageBuffer.iHeight = pImageInfo->iSizeY;

        const unsigned int iSize = pImageBuffer.iWidth * pImageBuffer.iHeight;
        unsigned char* pBuffer = new unsigned char[iSize];
        for (iY = 0; iY < pImageInfo->iSizeY; iY++)
        {
            pPixel8 = pImageInfo->pImageBuffer + pImageInfo->iSizeX * iY * iPixelStep;
            for (iX = 0; iX < (pImageInfo->iSizeX * iPixelStep); iX++)
            {
                //*pPixel8 = ~*pPixel8;
              //  *pBuffer = (char)(*pPixel8);
                pBuffer[iY * pImageBuffer.iWidth + iX] = (char)(*pPixel8);

               // pBuffer++;
                pPixel8++;
            }
        }


        pImageBuffer.pBuffer = pBuffer;
        m_pCallback->Execute((void*)&pImageBuffer);
       // delete pImageBuffer;
        delete[] pBuffer;
    }
}

void CCameraJai::SetCallback(cCallback* pCall)
{
    m_pCallback = pCall;
}

void CCameraJai::UninitialCamera()
{
    SetEvent(g_hEventExit);
}

void CCameraJai::PauseCamera()
{

}