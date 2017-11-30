

#include "g_camera_factory.h"
#include "g_camera_driver_jai.h"
#include "g_camera_driver_dalsa.h"

/* Animal factory constructor.
Private, called by the singleton accessor on first call.
Register the types of animals here.
*/
CameraFactory::CameraFactory()
{
    Register(CAMERA_STR_JAI, &CCameraJai::Create);
    Register(CAMERA_STR_DALSA, &CCameraDriverDalsa::Create);
}

CameraFactory::~CameraFactory()
{
    m_FactoryMap.clear(); 
}
void CameraFactory::Register(const std::string &animalName, CreateCameraFn pfnCreate)
{
    m_FactoryMap[animalName] = pfnCreate;
}
ICamera *CameraFactory::CreateCamera(const std::string &animalName)
{
    FactoryMap::iterator it = m_FactoryMap.find(animalName);
    if (it != m_FactoryMap.end())
        return it->second();
    return NULL;
}

ICamera *CameraFactory::CreateCamera(CAMERA_TYPE camera_type)
{
    switch (camera_type)
    {
    case CAMERA_JAI:
        {
            Register(CAMERA_STR_JAI, &CCameraJai::Create);
            return CreateCamera(CAMERA_STR_JAI);
        }
    	break;
    case CAMERA_DALSA:
        {
            Register(CAMERA_STR_DALSA, &CCameraDriverDalsa::Create);
            return CreateCamera(CAMERA_STR_DALSA);
        }
        break;
    default:
        break;
    }
    return nullptr;
}