

#include <stdio.h>
#include "g_camera_driver_jai.h"
//int main1()
//{
//    printf("x\n");
//
//    CCameraJai* pThread = new CCameraJai();
//
//    pThread->OnInitDialog();
//    while (1)
//    {
//        Sleep(1000);
//        if (pThread->m_iStatus == 2)
//        {
//            pThread->CloseFactoryAndCamera();
//            delete pThread;
//            pThread = nullptr;
//            break;
//        }
//    }
//    return 0;
//}
//

#include "g_camera_factory.h"
using namespace std;

void TestDalsa()
{
    CAMERA_TYPE cam_type = CAMERA_DALSA;
    CameraFactory pFac;

    ICamera *pCamera = pFac.CreateCamera(cam_type);
    if (pCamera)
    {
        pCamera->OpenCamera();

    }
    else
    {
        cout << "That camera doesn't exist! Choose another!" << std::endl;
    }

    while (1)
    {
        Sleep(1000);
        int iIn = 0;
        std::cin >> iIn;

        if (iIn == 3)
        {
            pCamera->CloseCamera();

            if (pCamera)
            {
                pCamera->Free();

            }
            pCamera = NULL;

            break;
        }
    }

    printf("main end\n");
}

int main()
{
    TestDalsa();
    return 0;
}