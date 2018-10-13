#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "boost/algorithm/string.hpp"
#include "boost/algorithm/string/classification.hpp"
#include <set>
#include <iostream>
#include <fstream>
#include <sstream>
#include <io.h>

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

void FindDicomFiles(const std::string& path, std::vector<std::string>& files)
{
    if (path.empty())
    {
        //_LOG_DEV_ERROR<<"path is empty.";
        return;
    }

    //文件句柄
    long hFile = 0;
    //文件信息
    struct _finddata_t fileinfo;  //很少用的文件信息读取结构
    std::string p;  //string类很有意思的一个赋值函数:assign()，有很多重载版本
    if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(),&fileinfo)) != -1)
    {
        do 
        {
            if ((fileinfo.attrib & _A_SUBDIR)) 
            {  //比较文件类型是否是文件夹
                if (strcmp(fileinfo.name,".") != 0 && strcmp(fileinfo.name,"..") != 0
                    && FilterFileName(fileinfo.name)) 
                {
                    FindDicomFiles(p.assign(path).append("\\").append(fileinfo.name), files);
                }
            } 
            else 
            {
                auto filePath = p.assign(path).append("\\").append(fileinfo.name);
                files.push_back(filePath);
            }
        } while (_findnext(hFile, &fileinfo) == 0);  //寻找下一个，成功返回0，否则-1
        _findclose(hFile);
    }
}

void usage()
{
    printf("test.exe folder\n");
    printf("output is d:/report.txt\n");
}

int main(int argc, char** argv)
{         
    usage();                                                                       
    std::string strDir = "input";
    if (argc < 2)
    {

        return -1;
    }
    strDir = argv[1];
    //std::string key = "546cecc41a8cddff5e901b05c0b78aee";


    std::vector<std::string> files;
    FindDicomFiles(strDir, files);

    std::ofstream fileReport;
    fileReport.open("d:/report.txt",static_cast<int>(std::ios::ate));

    std::string sLog="";
    sLog += "Files Count is " + std::to_string((long long) files.size());
    std::cout<<sLog<<"\n";
    for(auto itr=files.begin(); itr!=files.end(); ++itr) 
    { 
        sLog += "\n" + (*itr);
    }   

    (void)fileReport.write(sLog.c_str() ,static_cast<long long>(sLog.length()));
    fileReport.close();
    //std::string sValue;
    //if (found_start != std::string::npos &&
    //    found_end != std::string::npos)
    //{
    //    sValue = strInput.substr(found_start + fp_start.length(), found_end - found_start - fp_start.length());
    //}

   // (void)boost::split(vecString, strInput, boost::is_any_of("-k"), boost::algorithm::token_compress_on);

 

}
