
#ifndef UTILS_H_
#define UTILS_H_

#include <string>
#include <vector>
#include <map>

#include "app_defs.h"
#include "app_common.h"
#include "boost/date_time/gregorian/gregorian_types.hpp"
#include "boost/date_time/posix_time/posix_time_types.hpp"

BEGIN_NAMESPACE;

typedef boost::posix_time::ptime DATETIME_BOOST;
typedef boost::posix_time::time_duration TIME_BOOST;
typedef boost::gregorian::date DATE_BOOST;

class  EXPORT AppUtils
{
public:

    /////////////////////////////////////////////////////////////////
    ///  \brief            Delete File
    ///
    ///  \param[in]      const std::string& sFilePath
    ///
    ///  \param[out]    None
    ///  \return            None
    ///
    ///  \pre \e  
    /////////////////////////////////////////////////////////////////
    static bool DeleteLocalFile(const std::string& sFilePath);

    /////////////////////////////////////////////////////////////////
    ///  \brief            Delete Files
    ///
    ///  \param[in]      const std::vector<std::string>& FilePathArray
    ///
    ///  \param[out]    None
    ///  \return            None
    ///
    ///  \pre \e  
    /////////////////////////////////////////////////////////////////
    static void DeleteFiles(const std::vector<std::string>& FilePathArray);

    static void DeteteFolder(const std::string& sFolderName);

    /////////////////////////////////////////////////////////////////
    ///  \brief             Create folders and subfolders    
    ///                         Using Boost FileSystem 
    /// 
    ///  \param[in]     const std::string& sFolderName    
    ///                         eg. string strFilePath="E:/images/";
    ///                    
    ///  \param[out]   
    ///  \return            int 
    ///                         0:success to create a Child Folder    
    ///                         -1:fail
    ///  \pre \e        
    /////////////////////////////////////////////////////////////////
    static bool CreateMultiFolder(const std::string& sFolderName);

    /////////////////////////////////////////////////////////////////
    ///  \brief         ReadBufferFromFileDeleteDicomFiles
    ///                 Note: should delete buffer outside
    ///  \param[in]     const std::string& sFilePath
    ///  \param[out]    char* buffer, std::size_t* size
    ///  \return        bool
    ///
    ///  \pre \e  
    /////////////////////////////////////////////////////////////////
    static bool ReadBufferFromFile(const std::string& sFilePath, char*& buffer, std::size_t* size);
    static bool ReadBufferFromFile(const std::string& sFilePath, std::string& strBuffer);

    static void CopyFileInBinary(const std::string& sFilePathSrc, const std::string& sFilePathDst);

    static bool WriteFileInBinary(const std::string& sFileName, const char* buffer, std::size_t size);
    static bool WriteDataToFile(const std::string& sFileName, const std::string& sData);

    template <typename T> 
    static bool ReadBufferFromString(std::string sSource, std::string SplitString, std::vector<T>& vValueList);    

    static bool SaveToTrueColorBitmap(
        const std::string& path, 
        unsigned char* buffer,
        const int& height, 
        const int& width);

    static void SaveToGrayBitmap(
        const std::string& path, 
        unsigned char* buffer,
        const int& height, 
        const int& width, 
        unsigned char color = 255);

    //0018,1030 ProtocolName
    //0018,0015  BodyPartExamined
    static std::string GetLabel(int nSliceCount, 
        const std::string& Modality,
        const std::string& BodyPartExamined, 
        const std::string& SeriesDescription, 
        const std::string& StudyDescription,
        const std::string& ProtocolName);

    static APP_TYPE String2Enum_APP_TYPE(const char* user_input);

    //(0008,0020) Study Date StudyDate DA 1.            A string of characters of the format           
    //YYYYMMDD where YYYY shall contain year,           
    //MM shall contain the month, and DD shall            
    //contain the day, interpreted as a date of the            
    //Gregorian calendar system.            Example:19930822 would represent August 2
    static DATE_BOOST ConvertDate2Boost(const std::string& date);




    //(0008,0030) Study Time StudyTime TM 1.            
    //A string of characters of the format           
    //HHMMSS.FFFFFF; where HH contains hours            
    //(range 00 - 23), MM contains minutes            (range 00 - 59), SS contains seconds           
    //(range 00 - 60), and FFFFFF contains a            fractional part of a second as small as 1          
    //millionth of a second (range 000000 -            999999). A 24-hour clock is used. Midnight       
    //shall be represented by only 0000 since            2400 would violate the hour range. The         
    //string may be padded with trailing spaces.            Leading and embedded spaces are n
    static TIME_BOOST ConvertTime2Boost(const std::string& time);

    static DATETIME_BOOST ConvertDateTime2Boost(const std::string& datetime);

    static std::string GetCurrentDirPath();
    static std::string GetSystemRootPath();

    static std::string SimpleXmlParser(const std::string& logxml, const std::string& sKey);
    
    static std::string AppUtils::GetCurrentDate();
                                                                                                    
    static std::vector<std::string> Split( const std::string &s, const std::string &seperator );

    // eg. key = "-k"
    static std::vector<std::string> SplitSubstring(const std::string& str, const std::string& key);

    static std::string GetAIConfig();

    static std::wstring s2ws(const std::string& s);
    static std::string ws2s(std::wstring& inputws);
    static std::string Trim(const std::string str);

    // calc the date = today - numDays
    // 20181011
    static std::string DatePlusDays(int numDays);

    static void FindFilesRecursion(const std::string& path, std::vector<std::string>& files);
    static void FindFiles(const std::string& path, std::vector<std::string>& files);
    static void FindFolderRecursion(const std::string& path, std::vector<std::string>& folders);
};

END_NAMESPACE;

#endif  //UTILS_H_
