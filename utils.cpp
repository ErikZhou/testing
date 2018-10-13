#include "StdAfx.h"
#include "utils.h"

#include <ctime> //for DatePlusDays
#include <io.h>

//third library header
#include "boost/filesystem.hpp"//create directory
#include "boost/algorithm/string/split.hpp"
#include "boost/algorithm/string/classification.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "character_set_converter.h"

#include <stdio.h>
#include "app_performance.h"
#include "app_logger.h"

#ifdef WIN32
#include <windows.h>
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

BEGIN_NAMESPACE;

bool AppUtils::DeleteLocalFile(const std::string& sFilePath)
{
    namespace bf=boost::filesystem;
    if (!bf::exists(sFilePath))
    {
        _LOG_DEV_WARNING << (sFilePath + "Not exists when Delete File");
        return true;
    }

#   if defined(WIN32)
    //read only attributes
    DWORD dwAttrs = GetFileAttributesA(sFilePath.c_str());
    if (dwAttrs & FILE_ATTRIBUTE_READONLY)
    {
        SetFileAttributesA(sFilePath.c_str(),
            dwAttrs & ~FILE_ATTRIBUTE_READONLY);
    }
#   endif    
    try
    {
        (void)bf::remove(sFilePath);//delete file
    }
    catch (const bf::filesystem_error &e )
    {
        const std::string sError(e.what());
        _LOG_DEV_WARNING << (sError);
        return false;
    }
    return true;
}

void AppUtils::DeleteFiles(const std::vector<std::string>& FilePathArray)
{
    for(auto itr=FilePathArray.cbegin(); itr != FilePathArray.cend(); ++itr ) 
    { 
        DeleteLocalFile(*itr);
    }
}

void AppUtils::DeteteFolder(const std::string& sFolderName)
{
    boost::filesystem::path p = sFolderName;
    try
    {
        if(boost::filesystem::exists(p))
        {                        
            boost::filesystem::remove_all(p);
        }
    }
    catch(boost::filesystem::filesystem_error const & e)
    {
        //display error message 
        std::string slog = "DeteteFolder " + sFolderName + " failed:" + std::string(e.what());
        _LOG_DEV_ERROR<<slog;
        PRINTF_WITH_LINE_DEBUG("%s\n",slog.c_str());
    }
}

bool AppUtils::CreateMultiFolder(const std::string& sFolderName)
{
    namespace bfs = boost::filesystem;
    try{
        // create a directory, then check it for consistency
        //   take extra care to report problems, since if this fails
        //   many subsequent tests will fail
        if (bfs::is_directory(sFolderName)){
            return true;// already exists 
        }else if (bfs::exists(sFolderName)){
            return true; //is other
        }
        return bfs::create_directories(sFolderName);
    }
    catch ( const bfs::filesystem_error & x ){
        _LOG_DEV_ERROR<<("***** Creating directory: " + sFolderName + " failed.          *****\n" + std::string(x.what()));
        return false;
    }
}

bool AppUtils::WriteDataToFile(const std::string& sFileName, const std::string& sData)
{
    FILE* pFile;
    errno_t err=fopen_s(&pFile,sFileName.c_str(),"w");
    if(0 != err)
    {
        _LOG_DEV_ERROR<<"Failed to write the file :"<<sFileName;
        return false;
    }
    fprintf(pFile,"%s",sData.c_str());
    fclose(pFile);
    return true;
}

bool AppUtils::WriteFileInBinary(const std::string& sFilePath, const char* buffer, std::size_t size)
{
    if(nullptr == buffer || 0 == size) return false;

    std::ofstream ofs;// (sFilePath, std::ifstream::binary);
    ofs.open(sFilePath, std::ios::out | std::ios::binary);
    if (!ofs)
    {
        _LOG_DEV_ERROR<<"Failed to open file["<<sFilePath<<"].";
        return false;
    }

    ofs.write(buffer, size);
    ofs.close();
    return true;
}

bool AppUtils::ReadBufferFromFile(const std::string& sFilePath, char*& buffer, std::size_t* size)
{
    buffer = nullptr;
    if(nullptr == size) return false;

    std::ifstream ifs;// (sFilePath, std::ifstream::binary);
    ifs.open(sFilePath, std::ios::in | std::ios::binary);
    *size = 0;
    if (!ifs) {
        _LOG_DEV_ERROR<<"Failed to open file["<<sFilePath<<"].";
        return false;
    }

    // get pointer to associated buffer object
    std::filebuf* pbuf = ifs.rdbuf();
    // get file size using buffer's members
    *size = pbuf->pubseekoff (0,ifs.end,ifs.in);
    if (*size > 0){
        pbuf->pubseekpos (0,ifs.in);
        // allocate memory to contain file data
        buffer=new char[*size];
        // get file data
        pbuf->sgetn (buffer,*size);
    }
    ifs.close();
    // write content to stdout
    //std::cout.write (buffer,size);
    return true;
}


bool AppUtils::ReadBufferFromFile(const std::string& sFilePath, std::string& strBuffer)
{
    char* buffer = nullptr;
    std::size_t fileSize(0);
    if(!AppUtils::ReadBufferFromFile(sFilePath, buffer, &fileSize))
    {
        _LOG_DEV_ERROR<<"Failed to open file["<<sFilePath<<"].";
        return false;
    }
    if (0 == fileSize || nullptr == buffer){
        _LOG_DEV_ERROR<<"Failed to open file["<<sFilePath<<"].";
        return false;
    }

    strBuffer = buffer;
    DEL_ARRAY(buffer);
    return true;
}

void AppUtils::CopyFileInBinary(const std::string& sFilePathSrc, const std::string& sFilePathDst)
{
    LPCSTR sourceFilePath = sFilePathSrc.c_str();
    LPCSTR targetFilePath = sFilePathDst.c_str();
    CopyFileA(sourceFilePath, targetFilePath, FALSE);
}

template <typename T> 
bool AppUtils::ReadBufferFromString(
    std::string sSource, 
    std::string SplitString, 
    std::vector<T>& vValueList)
{
    std::vector<std::string> vecValue;
    (void)boost::split(vecValue, sSource, boost::is_any_of(SplitString));

    for (auto itr = vecValue.cbegin(); itr != vecValue.cend(); ++itr)
    {
        T value = boost::lexical_cast<T>((*itr).c_str());
        vValueList.push_back(value);
    }
    return true;
}

bool AppUtils::SaveToTrueColorBitmap(const std::string& path,
    unsigned char* buffer,
    const int& height,
    const int& width)
{

#ifdef WIN32
        //todo: check whether the file is existed

        // calculate the pad size of whole image
        int nStride = (width * 32 + 7) / 8;
        int paddedsize = nStride*height;

        // declare bmp structures 
        BITMAPFILEHEADER bmfh;
        BITMAPINFOHEADER info;

        // andinitialize them to zero
        memset(&bmfh, 0, sizeof(BITMAPFILEHEADER));
        memset(&info, 0, sizeof(BITMAPINFOHEADER));

        // fill the fileheader with data
        bmfh.bfType = 0x4d42;       // 0x4d42 = 'BM'
        bmfh.bfReserved1 = 0;
        bmfh.bfReserved2 = 0;
        bmfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + paddedsize;
        bmfh.bfOffBits = 0x36;		// number of bytes to start of bitmap bits

        // fill the infoheader

        info.biSize = sizeof(BITMAPINFOHEADER);
        info.biWidth = width;
        info.biHeight = height;
        info.biPlanes = 1;			// we only have one bitplane
        info.biBitCount = 32;		// RGB mode is 24 bits
        info.biCompression = BI_RGB;
        info.biSizeImage = 0;		// can be 0 for 24 bit images
        info.biXPelsPerMeter = 0x0ec4;     // paint and PSP use this values
        info.biYPelsPerMeter = 0x0ec4;
        info.biClrUsed = 0;			// we are in RGB mode and have no palette
        info.biClrImportant = 0;    // all colors are important

        // now we open the file to write to
        //#ifdef _DEBUG
        //    HANDLE file = CreateFileW(sPath.c_str() , GENERIC_WRITE, FILE_SHARE_READ,
        //        NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
        //#else
        std::wstring wsPath(path.begin(), path.end());
        HANDLE file = CreateFile(wsPath.c_str(), GENERIC_WRITE, FILE_SHARE_READ,
            NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        //#endif

        if (file == NULL)
        {
            CloseHandle(file);
            return false;
        }

        // write file header
        unsigned long bwritten;
        if (WriteFile(file, &bmfh, sizeof(BITMAPFILEHEADER), &bwritten, NULL) == false)
        {
            CloseHandle(file);
            return false;
        }
        // write infoheader
        if (WriteFile(file, &info, sizeof(BITMAPINFOHEADER), &bwritten, NULL) == false)
        {
            CloseHandle(file);
            return false;
        }
        // write image data
        if (WriteFile(file, buffer, paddedsize, &bwritten, NULL) == false)
        {
            CloseHandle(file);
            return false;
        }

        // and close file
        CloseHandle(file);
#else
        path;
        buffer;
        width;
        height;
#endif // _WINDOWS

        return true;
}


APP_TYPE AppUtils::String2Enum_APP_TYPE(const char* user_input)
{
    for (int t = 0; t < APP_TYPE_UNSUPPORTED; ++t)
    {    
        if (0 == strcmp(ENUM_TO_STRING_APP_TYPE[t], user_input))
        {
            return (APP_TYPE)t;
        }
    }
    return APP_TYPE_UNSUPPORTED;
};


bool FindSubString(const std::string& str, const std::string& find)
{
    std::string temp(str);
    (void)std::transform(str.begin(), str.end(), temp.begin(), static_cast<int(*)(int)>(std::toupper));
    size_t pos  = temp.find(find);  
    if (pos != std::string::npos )
    {
        return true;
    }
    return false;
}

std::string AppUtils::GetLabel(int nSliceCount, 
    const std::string& Modality,
    const std::string& BodyPartExamined, 
    const std::string& SeriesDescription, 
    const std::string& StudyDescription,
    const std::string& ProtocolName)
{

    //for CT MR PT by qiangqiang.zhou@20180906
    //rejected data
    if(nSliceCount>1 && nSliceCount < MIN_SLICE_COUNT)
    {
        _LOG_DEV_WARNING<<"nSliceCount>1 && nSliceCount < MIN_SLICE_COUNT :"<<std::to_string((long long )nSliceCount)<<"\n";
        return "";
    }

    std::string sLabel("");
    int index = -1;
    bool bFound = false;
    for (int t = 0; t < APP_PROTOCOL_KEY_UNSUPPORTED; ++t)
    {    
        index = t;
        sLabel = std::string(ENUM_TO_STRING_APP_PROTOCOL_KEY[t]);

        if (t == APP_PROTOCOL_KEY_CHEST_CN)
        {
            std::string temp;
            CharacterSetConverter::ToUtf8("GB18030",sLabel, temp);
            sLabel = temp;
        }
             
        if(FindSubString(ProtocolName,sLabel))
        {
            bFound = true;
            break;
        }

        if(FindSubString(BodyPartExamined,sLabel))
        {
            bFound = true;
            break;
        }

        if(FindSubString(SeriesDescription,sLabel))
        {
            bFound = true;
            break;
        }

        if(FindSubString(StudyDescription,sLabel))
        {
            bFound = true;
            break;
        }
    }

    if (!bFound || index < 0 || sLabel.empty())
    {
        return "";
    }



    switch (index)
    {
    case APP_PROTOCOL_KEY_CHEST:
        {
            sLabel = Modality == "CT" ? "CT_LUNG":"DR_CHEST";
        }
        break;
    case APP_PROTOCOL_KEY_HAND:
        {
            sLabel = "DR_HAND";
        }
        break;
    case APP_PROTOCOL_KEY_LUNG:
        {
            sLabel = "CT_LUNG";
        }
        break;
    case APP_PROTOCOL_KEY_RIB:
        {
            sLabel = "CT_RIB";
        }
        break;
    case APP_PROTOCOL_KEY_CHEST_CN:
        {
            sLabel = "DR_CHEST";
        }
        break;
    default:
        {
            sLabel ="";
        }
        break;
    }

    return sLabel;
}

void AppUtils::SaveToGrayBitmap(const std::string& path,
    unsigned char* buffer,
    const int& height,
    const int& width, unsigned char color){

        const int size = width * height;
        color = color < 0 ? 0 : color;
        color = color > 255 ? 255 : color;
        unsigned char* bmpBuffer = new unsigned char[size * 4];
        memset(bmpBuffer, 0, sizeof(unsigned char) * size * 4);
        for (int j = 0; j < size; ++j){
            if (buffer[j])
            {
                bmpBuffer[4 * j] = color;
                bmpBuffer[4 * j + 1] = color;
                bmpBuffer[4 * j + 2] = color;
                bmpBuffer[4 * j + 3] = 255;
            }
        }

        SaveToTrueColorBitmap(path, bmpBuffer, height, width);

        delete[]bmpBuffer;

}

DATE_BOOST AppUtils::ConvertDate2Boost(const std::string& date)
{
    //(0008,0020) Study Date StudyDate DA 1.            A string of characters of the format           
    //YYYYMMDD where YYYY shall contain year,           
    //MM shall contain the month, and DD shall            
    //contain the day, interpreted as a date of the            
    //Gregorian calendar system.            Example:19930822 would represent August 2
    if (date.empty())
    {
        boost::gregorian::date d;
        return d;
    }
    boost::gregorian::date d(boost::gregorian::from_undelimited_string(date));    
    return d;
}

TIME_BOOST AppUtils::ConvertTime2Boost(const std::string& time)
{
    std::string sTempTime = time;
    if (!sTempTime.empty() && sTempTime.length()>4)
    {
        (void)sTempTime.insert(2,":");
        (void)sTempTime.insert(5,":");
        boost::posix_time::time_duration t(boost::posix_time::duration_from_string(sTempTime));
        return t;
    }
    boost::posix_time::time_duration tt;
    return tt;
}

DATETIME_BOOST AppUtils::ConvertDateTime2Boost(const std::string& datetime)
{
    DATETIME_BOOST t(boost::posix_time::time_from_string(datetime));
    if (datetime.length() >18)
    {
        return t;
    }
    DATETIME_BOOST tt;
    return tt;
}

std::string AppUtils::GetSystemRootPath()
{   
    std::string sRootPath = AppUtils::GetCurrentDirPath();
    sRootPath = sRootPath.substr(0,sRootPath.find_last_of("\\"));
    replace(sRootPath.begin(), sRootPath.end(), '\\', '/' );
    return sRootPath;
}

std::string AppUtils::SimpleXmlParser(const std::string& logxml, const std::string& sKey)
{
    std::string fp_start = "<" + sKey + ">";
    std::string fp_end = "</" + sKey + ">";;
    size_t found_start = logxml.find(fp_start);
    size_t found_end = logxml.find(fp_end);
    std::string sValue;
    if (found_start != std::string::npos &&
        found_end != std::string::npos)
    {
        sValue = logxml.substr(found_start + fp_start.length(), found_end - found_start - fp_start.length());
    }
    return sValue;
}

std::string AppUtils::GetCurrentDate()
{
    // current date/time based on current system
    time_t now = time(0);
    tm *ltm = localtime(&now);

    // print various components of tm structure.
    //std::cout << "Year" << 1900 + ltm->tm_year<<std::endl;
    //std::cout << "Month: "<< 1 + ltm->tm_mon<< std::endl;
    //std::cout << "Day: "<<  ltm->tm_mday << std::endl;
    //std::cout << "Time: "<< 1 + ltm->tm_hour << ":";
    //std::cout << 1 + ltm->tm_min << ":";
    //std::cout << 1 + ltm->tm_sec << std::endl;
    std::string sDate("");

    char buffer [50]={0};
    sprintf (buffer, "%d%02d%02d", (1900 + ltm->tm_year), (1 + ltm->tm_mon), ltm->tm_mday );
    sDate = std::string(buffer);
    return sDate;
}

std::string AppUtils::GetCurrentDirPath()
{
    char cCurrentPath[FILENAME_MAX];
    if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath)))
    {
        PRINTF_WITH_LINE("GetCurrentDir failed.\n");
        return "";
    }
    std::string sRootPath (cCurrentPath);
    return sRootPath;
}


std::vector<std::string> AppUtils::Split( const std::string &s, const std::string &seperator )
{
    std::vector<std::string> result;
    typedef std::string::size_type string_size;
    string_size i = 0;

    while(i != s.size())
    {
        //找到字符串中首个不等于分隔符的字母；
        int flag = 0;
        while(i != s.size() && flag == 0)
        {
            flag = 1;
            for(string_size x = 0; x < seperator.size(); ++x)
            {
                if(s[i] == seperator[x])
                {
                    ++i;
                    flag = 0;
                    break;
                }
            }
        }

        //找到又一个分隔符，将两个分隔符之间的字符串取出；
        flag = 0;
        string_size j = i;
        while(j != s.size() && flag == 0)
        {
            for(string_size x = 0; x < seperator.size(); ++x)
            {
                if(s[j] == seperator[x])
                {
                    flag = 1;
                    break;
                }
                if(flag == 0)
                {
                    ++j;
                }
            }
        }
        if(i != j)
        {
            result.push_back(s.substr(i, j-i));
            i = j;
        }
    }
    return result;
}

std::vector<std::string> AppUtils::SplitSubstring(const std::string& str, const std::string& key)
{
    std::vector<size_t> vecPos;
    std::string data = str;
    size_t index = 0;
    const size_t nKey = key.size();
    std::vector<std::string> vecResult;
    while (!data.empty())
    {                         
        size_t pos = data.find(key);
        if (pos != std::string::npos)
        {
            std::string str = data.substr(0,pos);
            pos += nKey;

            vecResult.push_back(Trim(str));
            vecPos.push_back(index + pos);
            index += pos;

            data = data.substr(pos);
        }
        else
        {
            vecResult.push_back(Trim(data));
            data = "";
        }

    }
    return vecResult;
}

std::string AppUtils::GetAIConfig()
{
    std::string sXmlConfig = "appdata/uAIConfig.xml";
    sXmlConfig = AppUtils::GetSystemRootPath() + "/" + sXmlConfig;

    std::string sXmlData;
    if(!AppUtils::ReadBufferFromFile(sXmlConfig, sXmlData))
    {
        _LOG_DEV_ERROR<<"ReadBufferFromFile failed:"<<sXmlConfig;
    }
    return sXmlData;
}

//Converting a WChar string to a Ansi string
std::string WChar2Ansi(LPCWSTR pwszSrc)
{
    int nLen = WideCharToMultiByte(CP_ACP, 0, pwszSrc, -1, NULL, 0, NULL, NULL);

    if (nLen<= 0) return std::string("");

    char* pszDst = new char[nLen];
    if (NULL == pszDst) return std::string("");

    WideCharToMultiByte(CP_ACP, 0, pwszSrc, -1, pszDst, nLen, NULL, NULL);
    pszDst[nLen -1] = 0;

    std::string strTemp(pszDst);
    delete [] pszDst;

    return strTemp;
}

std::string AppUtils::ws2s(wstring& inputws)
{
    return WChar2Ansi(inputws.c_str()); 
}

//Converting a Ansi string to WChar string

std::wstring Ansi2WChar(LPCSTR pszSrc, int nLen)

{
    int nSize = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)pszSrc, nLen, 0, 0);
    if(nSize <= 0) return NULL;

    WCHAR *pwszDst = new WCHAR[nSize+1];
    if( NULL == pwszDst) return NULL;

    MultiByteToWideChar(CP_ACP, 0,(LPCSTR)pszSrc, nLen, pwszDst, nSize);
    pwszDst[nSize] = 0;

    if( pwszDst[0] == 0xFEFF) // skip Oxfeff
        for(int i = 0; i < nSize; i ++) 
            pwszDst[i] = pwszDst[i+1];

    wstring wcharString(pwszDst);
    delete pwszDst;

    return wcharString;
}

std::wstring AppUtils::s2ws(const std::string& s)
{ 
    return Ansi2WChar(s.c_str(),(int)(s.size()));
}

std::string AppUtils::Trim(const std::string str) 
{ 
    if (str.empty())
    {
        return "";
    }

    std::string tmp = str.substr(str.find_first_not_of(" ")); 
    return tmp.substr(0,tmp.find_last_not_of(" ")+1); 
}  

// Adjust date by a number of days +/-
void datePlusDays( struct tm* date, int days )
{
    const time_t ONE_DAY = 24 * 60 * 60 ;

    // Seconds since start of epoch
    time_t date_seconds = mktime( date ) + (days * ONE_DAY) ;

    // Update caller's date
    // Use localtime because mktime converts to UTC so may change date
    *date = *localtime( &date_seconds );
}
std::string AppUtils::DatePlusDays(int numDays)
{
    time_t t = time(0);   // get time now
    struct tm * now = localtime( & t );

    struct tm date = { 0, 0, 12 } ;  // nominal time midday (arbitrary).
    // Set up the date structure
    date.tm_year = now->tm_year;
    date.tm_mon = now->tm_mon;  // note: zero indexed
    date.tm_mday = now->tm_mday ;       // note: not zero indexed

    // Date, less 100 days
    datePlusDays(&date, - numDays ) ; 

    char       buf[80];
    strftime(buf, sizeof(buf), "%Y%m%d", &date);
    std::string newDate(buf);
    return newDate;
}

bool FilterFileName(char name[260])
{
    std::string sFileName(name);
    if (sFileName.length() < 4)
    {
        return false;
    }

    std::string sBMP = sFileName.substr(sFileName.length()-3, 3);
    std::string temp = sBMP;
    // (void)std::transform(sBMP.begin(), sBMP.end(), temp.begin(), temp.end(),static_cast<int(*)(int)>(::toupper));
    (void)std::transform(sBMP.begin(), sBMP.end(), temp.begin(),(int (*)(int))toupper);
    if (".db" == temp
        || "BMP" == temp
        || "JPG" == temp
        || "TXT" == temp
        || "DOC" == temp
        || "PDF" == temp
        || "PNG" == temp
        || "VOL" == temp
        )
    {
        return false;
    }
    return true;
}

void AppUtils::FindFilesRecursion(const std::string& path, std::vector<std::string>& files)
{
    if (path.empty())
    {
        _LOG_DEV_ERROR<<"path is empty.";
        return;
    }

    intptr_t  hFile = 0;
    struct _finddata_t fileinfo;  
    std::string p;
    if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(),&fileinfo)) != -1)
    {
        do 
        {
            if ((fileinfo.attrib & _A_SUBDIR)) 
            {
                if (strcmp(fileinfo.name,".") != 0 && strcmp(fileinfo.name,"..") != 0
                    && FilterFileName(fileinfo.name)) 
                {
                    FindFilesRecursion(p.assign(path).append("\\").append(fileinfo.name), files);
                }
            } 
            else 
            {
                auto filePath = p.assign(path).append("\\").append(fileinfo.name);
                files.push_back(filePath);
            }
        } while (_findnext(hFile, &fileinfo) == 0);  //success 0，else -1
        _findclose(hFile);
    }
}

void AppUtils::FindFiles(const std::string& path, std::vector<std::string>& files)
{
    if (path.empty())
    {
        _LOG_DEV_ERROR<<"path is empty.";
        return;
    }

    intptr_t  hFile = 0;
    struct _finddata_t fileinfo;
    std::string p;
    if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(),&fileinfo)) != -1)
    {
        do 
        {
            if (!(fileinfo.attrib & _A_SUBDIR)) 
            {  
                if (FilterFileName(fileinfo.name)) 
                {
                    auto filePath = p.assign(path).append("\\").append(fileinfo.name);
                    files.push_back(filePath);
                }
            }
        } while (_findnext(hFile, &fileinfo) == 0);  //寻找下一个，成功返回0，否则-1
        _findclose(hFile);
    }
}

void AppUtils::FindFolderRecursion(const std::string& path, std::vector<std::string>& folders)
{
    if (path.empty())
    {
        _LOG_DEV_ERROR<<"path is empty.";
        return;
    }
    intptr_t hFile = 0;
    struct _finddata_t fileinfo;
    std::string p;  
    if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(),&fileinfo)) != -1)
    {
        do 
        {
            if ((fileinfo.attrib & _A_SUBDIR)) 
            {  
                if (strcmp(fileinfo.name,".") != 0 && strcmp(fileinfo.name,"..") != 0) 
                {
                    FindFolderRecursion(p.assign(path).append("\\").append(fileinfo.name), folders);

                    auto filePath = p.assign(path).append("\\").append(fileinfo.name);
                    folders.push_back(filePath);
                }
            } 
        } while (_findnext(hFile, &fileinfo) == 0);  //寻找下一个，成功返回0，否则-1
        _findclose(hFile);
    }
}

END_NAMESPACE;
