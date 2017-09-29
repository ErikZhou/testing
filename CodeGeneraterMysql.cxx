#include "CodeGeneraterMysql.h"

#include <sstream>
#include <string>
#include <algorithm>
#include <time.h>   //for date time

#include "rt_tps_database_connect.h"
#include <iostream>

#define TEST_PERFORMANCE_ON
#include "tps_performance.h"

#pragma comment(lib, "libmysql.lib")

#define INFORMATION_SCHEMA "information_schema"
#define SHOW_TABLES "show tables"
#define SHOW_COLUMNS "select ORDINAL_POSITION, COLUMN_KEY, IS_NULLABLE, COLUMN_NAME,DATA_TYPE,CHARACTER_MAXIMUM_LENGTH,COLUMN_COMMENT from columns where table_name = "
#define GET_DBVERSION "select version from tmsdbversion limit 1"

CodeGeneraterMysql::CodeGeneraterMysql(void) : m_pMySql(nullptr),
    m_sUser(""),m_sPassword(""),
    m_sDatabaseName(""),
    m_sHostName(""),
    m_uiPort(0),
    m_sCurrentDate("")
{
    vTables.clear();
}

CodeGeneraterMysql::~CodeGeneraterMysql(void)
{

}


std::wstring Str2Wstr (std::string str )
{
    if (str.length() == 0)
        return L"";

    std::wstring wstr;
    wstr.assign (str.begin(), str.end());
    return wstr;
}

std::string ToUTF8(const wchar_t* buffer, int len)  
{  
    int size = ::WideCharToMultiByte(CP_UTF8, 0, buffer, len, NULL, 0, NULL,  
        NULL);  
    if (size == 0)  
        return "";  

    std::string newbuffer;  
    newbuffer.resize(size);  
    ::WideCharToMultiByte(CP_UTF8, 0, buffer, len,  
        const_cast<char*>(newbuffer.c_str()), size, NULL, NULL);  

    return newbuffer;  
}  

std::string ToUTF8(const std::wstring& str)  
{  
    return ToUTF8(str.c_str(), (int) str.size());  
}  

std::wstring MBytesToWString(const char* lpcszString)
{
//    size_t len = strlen(lpcszString);
    int unicodeLen = ::MultiByteToWideChar(CP_ACP, 0, lpcszString, -1, NULL, 0);
    wchar_t* pUnicode = new wchar_t[unicodeLen + 1];
    memset(pUnicode, 0, (unicodeLen + 1) * sizeof(wchar_t));
    ::MultiByteToWideChar(CP_ACP, 0, lpcszString, -1, (LPWSTR)pUnicode, unicodeLen);
    std::wstring wString = (wchar_t*)pUnicode;
    delete [] pUnicode;
    return wString;
}

std::wstring StringToWstring(const std::string& str)
{
    if (str.empty()) return L"";
    
    const int size = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstr(size, 0 );
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstr[0], size);
    return wstr;
}

std::string WStringToMBytes(const wchar_t* lpwcszWString)
{
    char* pElementText;
    int iTextLen;
    // wide char to multi char
    iTextLen = ::WideCharToMultiByte(CP_ACP, 0, lpwcszWString, -1, NULL, 0, NULL, NULL);
    pElementText = new char[iTextLen + 1];
    memset((void*)pElementText, 0, (iTextLen + 1) * sizeof(char));
    ::WideCharToMultiByte(CP_ACP, 0, lpwcszWString, 0, pElementText, iTextLen, NULL, NULL);
    std::string strReturn(pElementText);
    delete [] pElementText;
    return strReturn;
}

std::wstring UTF8ToWString(const char* lpcszString)
{
//    int len = strlen(lpcszString);
    int unicodeLen = ::MultiByteToWideChar(CP_UTF8, 0, lpcszString, -1, NULL, 0);
    wchar_t* pUnicode;
    pUnicode = new wchar_t[unicodeLen + 1];
    memset((void*)pUnicode, 0, (unicodeLen + 1) * sizeof(wchar_t));
    ::MultiByteToWideChar(CP_UTF8, 0, lpcszString, -1, (LPWSTR)pUnicode, unicodeLen);
    std::wstring wstrReturn(pUnicode);
    delete [] pUnicode;
    return wstrReturn;
}

std::string WStringToUTF8(const wchar_t* lpwcszWString)
{
    char* pElementText;
    int iTextLen = ::WideCharToMultiByte(CP_UTF8, 0, (LPWSTR)lpwcszWString, -1, NULL, 0, NULL, NULL);
    pElementText = new char[iTextLen + 1];
    memset((void*)pElementText, 0, (iTextLen + 1) * sizeof(char));
    ::WideCharToMultiByte(CP_UTF8, 0, (LPWSTR)lpwcszWString, -1, pElementText, iTextLen, NULL, NULL);
    std::string strReturn(pElementText);
    delete [] pElementText;
    return strReturn;
}

bool CodeGeneraterMysql::SetUp(std::string sIpAddress)
{
    //mysql
    m_sUser="root";
    m_sPassword="111111";
    m_sDatabaseName="rt_tms";
    //m_sHostName="127.0.0.1";
    m_sHostName = sIpAddress;
    m_uiPort = 3306;

    //{ test connect to DB
    if (!create_database (m_sUser, m_sPassword,m_sDatabaseName,m_sHostName,m_uiPort,
        m_pMySql)){
        printf("RtDatabaseWrapper has not been initialized probably caused by wrong config or mysql dead");
        return false;
    }

    m_sCurrentDate = this->GetCurrentData();

    m_DatabaseVersion = this->GetDatabaseVersion();
    return true;
}

/*
/// convert as UTF-8 - display as Unicode - write as UTF-8
void test2(const char* filenamein, const char* filenameout)
{
    std::cout << "Testing reading as wchar_t stream with UTF-8 conversion facet" << std::endl;
    std::wifstream fsi; std::wofstream fso;
    fsi.imbue(gel::stdx::utf8_locale); fso.imbue(gel::stdx::utf8_locale);
    fsi.open(filenamein);
    if(!fsi.good()) { std::cout << "cannot open input file [" << filenamein << "]\n" << std::endl; return; }
    wchar_t c; std::wstring s;
    while(fsi.get(c)) { s.push_back(c); std::cout << '.' << std::flush; }
    std::cout << std::endl << "Prompting messagebox as Unicode" <<
        "(if filein is UTF-8, output should be correct)" << std::endl;
   // MessageBoxW(0,s.c_str(),L"Unicode display of UTF-8 input from UTF-8",MB_OK);


    std::cout << "Writing back again as UTF-8" << std::endl;
    fso.open(filenameout,fso.out|fso.trunc);
    if(!fso.good()) { std::cout << "cannot open output file [" << filenameout << "]\n" << std::endl; return; }
    fso << s << std::flush;
    std::cout << "Done. Now " << filenamein << " and " << filenameout << "should be identical\n" << std::endl;
}

*/

//true or false
bool CodeGeneraterMysql::MysqlRealQuery(const std::string& sSQL) const
{
    try
    {
        if (nullptr == m_pMySql){
            return false;
        }
        printf("Executing:%s\n", sSQL.c_str());
        if (0 != mysql_real_query(m_pMySql, sSQL.c_str(), static_cast<unsigned int>(sSQL.length())))
        { //mysql_real_query() failed due to an error

            unsigned int errorNo = mysql_errno(m_pMySql);
            std::string sLog("");
            if (errorNo>0){
                std::stringstream ss;
                std::string str;
                ss<<errorNo;
                ss>>str;
                sLog = "mysql_error "+str+ " :"+ mysql_error(m_pMySql)+" with SQL is "+sSQL.c_str() +"!";
                printf("%s\n", sLog.c_str());
            }
            return false;
        }
    }
    catch (const std::exception& e){
        printf("std::exception:%s\n",e.what());
        return false;
    }
    return true;
}

void CodeGeneraterMysql::ShowTables()
{
    if (nullptr == m_pMySql)
    {
        printf("DBWrapper has not been initialized");
        return;
    }
    vTables.clear();

    MYSQL_RES *myquery = nullptr;
    const std::string sSQL = SHOW_TABLES;
    if(!this->MysqlRealQuery(sSQL)){
        return;
    }

    myquery = mysql_store_result(m_pMySql);
    if (nullptr == myquery){
        return;
    }

    MYSQL_ROW row = nullptr;
    const unsigned int num_fields = mysql_num_fields(myquery);
    if (num_fields >0 )
    {
        while (nullptr != (row = mysql_fetch_row(myquery)))
        {
            TableItem tab;
            tab.sTableName = row[0];
            tab.bIsMap = false;
            tab.bHasDateTime = false;

            //if (tab.sTableName == "tpspoi")//tps using tpspoi
            //{
            //    //continue;
            //    tab.sTableName = "tpspoiex";
            //}

            {
                tab.sShortName = tab.sTableName.substr(3,tab.sTableName.length());
                std::string sTableNameUpper = tab.sShortName;
                (void)transform(tab.sShortName.begin(), tab.sShortName.end(), sTableNameUpper.begin(), toupper);
                tab.sShortNameUpper = sTableNameUpper;
                tab.enTableType = String2EnumTableType(sTableNameUpper.c_str());//DataType

                //
                std::string sFirstUpper = tab.sShortName.substr(0,1);
                transform(sFirstUpper.begin(), sFirstUpper.end(), sFirstUpper.begin(), toupper);
                std::string sTableNameWithFirst = sFirstUpper + tab.sShortName.substr(1,tab.sShortName.length() - 1);
                tab.sClassName = "Rt" + sTableNameWithFirst;

				switch (tab.enTableType)
				{
				case TYPE_MACHINE:
				case TYPE_TRAY:
				case TYPE_ACCESSORY:
				case TYPE_APPLICATOR:
				case TYPE_COMMISSIONEDUNIT:
				case TYPE_MCPHOCOMMISSIONEDUNIT:
				case TYPE_COMMISSIONEDUNIT_TRAY:
				case TYPE_GOLDENSTT:
				case TYPE_ACCESSORYCHUNK:
				case TYPE_CONTOUR:
				case TYPE_KERNELWEIGHT:
				case TYPE_MEASUREDDATA:
				case TYPE_WEDGEFACTOR:
				{
					tab.bIsSeriesable = true;
					break;
				}
				default:
					tab.bIsSeriesable = false;
					break;
				}
            }

            //std::size_t found = tab.sTableName.find("_");
            //if (found != std::string::npos){
            //    tab.bIsMap = true;
            //}

            vTables.push_back(tab);
        }
    }
    //mysql_free_result
    if (nullptr != myquery){
        mysql_free_result(myquery);
        myquery = nullptr;
    }
}

void CodeGeneraterMysql::ShowColumns(TableItem& table)
{
    if (nullptr == m_pMySql){
        printf("DBWrapper has not been initialized");
        return;
    }

    MYSQL_RES *myquery = nullptr;
    const std::string sSQL = SHOW_COLUMNS \
        "'" + table.sTableName + "'";
    if(!this->MysqlRealQuery(sSQL)){
        return;
    }

    myquery = mysql_store_result(m_pMySql);
    if (nullptr == myquery){
        return;
    }

    //////////////////////////////////////////////////////////////////////////
    //change default table name
    if (table.sTableName == "tpspoi")//tps using tpspoi
    {
        table.sTableName = "tpspoiex";
    }

    MYSQL_ROW row = nullptr;
    table.vColumnItems.clear();
    table.bHasDateTime = false;
    const unsigned int num_fields = mysql_num_fields(myquery);
    if (num_fields >0 )
    {
        int iCountKey = 0;
        while (nullptr != (row = mysql_fetch_row(myquery)))
        {
            ColumnItem col;
            col.iPosition = atoi(row[0]);   //ORDINAL_POSITION
            col.bIsKey = strcmp(row[1], "PRI") == 0;   //COLUMN_KEY
            col.bIsNullable = strcmp(row[2], "YES") == 0;
            col.sColumnName = row[3];       //COLUMN_NAME
            std::string sNameUpper = col.sColumnName;
            (void)transform(col.sColumnName.begin(), col.sColumnName.end(), sNameUpper.begin(), toupper);
            col.sColumnNameUpper = sNameUpper;

            col.enDataType = String2EnumDataType(row[4]);//DataType
            if (_date == col.enDataType || _time == col.enDataType || _datetime == col.enDataType || _timestamp == col.enDataType){
                table.bHasDateTime = true;
            }
            col.iMaxLength = nullptr == row[5] ? 0 : atoi(row[5]);
            col.sComments = row[6];
            table.vColumnItems.push_back(col);
            if (col.bIsKey) ++iCountKey;
        }
        if (2 == iCountKey) table.bIsMap = true;
        table.iFkCount = iCountKey;
    }
    //mysql_free_result
    if (nullptr != myquery){
        mysql_free_result(myquery);
        myquery = nullptr;
    }
}

void CodeGeneraterMysql::ConvertDataType2Cpp(TableItem& table)
{
    for (auto itr=table.vColumnItems.begin(); itr!=table.vColumnItems.end(); ++itr)
    {
        (*itr).sCppDataType = std::string(Enum2StringCpp[(*itr).enDataType]);
        switch (table.enTableType)
        {
        case TYPE_COMMISSIONEDUNIT:
            {
                if ((*itr).sColumnName == "discretedoserate")
                {
                    (*itr).sCppDataType = "std::vector<float>";
                    break;
                }
            }
            break;
        case TYPE_RTIMAGE:
            {
                if ((*itr).sColumnName == "pixeldata")
                {
                    (*itr).sCppDataType = "char*";
                    break;
                }
            }
            break;
        case TYPE_MACHINE:
            {
                if ((*itr).sColumnName == "leafboundaries")
                {
                    (*itr).sCppDataType = "std::vector<double>";
                    break;
                }
            }
            break;
        default:
            break;
        }
    }
}

void CodeGeneraterMysql::rt_tps_database_interface_object_h(TableItem& table)
{
    //write rt_tps_database_interface_object_XXX_h
    //std::ofstream file_rt_tps_database_interface_object_h;
    std::string shortName = table.sTableName.substr(3,table.sTableName.length());
    std::string sFileName="rt_tps_database_interface_object_" + shortName + ".h";
    const std::string sFileNameOutput="output//objects//" + sFileName;
    //file_rt_tps_database_interface_object_h.open(sFileNameOutput);

    std::string sData("");
    sData += "//////////////////////////////////////////////////////////////////////////";
    sData += "\n/// \\defgroup Radio Therapy Business Unit";
    sData += "\n///  Copyright, (c) Shanghai United Imaging Healthcare Inc., 2016";
    sData += "\n///  All rights reserved.";
    sData += "\n///";
    sData += "\n///  \\author  ZHOU qiangqiang  mailto:qiangqiang.zhou@united-imaging.com";
    sData += "\n///";
    sData += "\n///  \\file      rt_tps_database_interface_object.h";
    sData += "\n///  \\brief     This file was generated by CodeGenerater.exe ";
    sData += "\n///              From database version: " + m_DatabaseVersion;
    sData += "\n///";
    sData += "\n///  \\version 1.0";
    sData += "\n///  \\date    " + m_sCurrentDate;
    sData += "\n///  \\{";
    sData += "\n//////////////////////////////////////////////////////////////////////////";

    std::string sTableNameUpper = shortName;
    transform(shortName.begin(), shortName.end(), sTableNameUpper.begin(), toupper);
    std::string sFirstUpper = shortName.substr(0,1);
    transform(sFirstUpper.begin(), sFirstUpper.end(), sFirstUpper.begin(), toupper);
    std::string sTableNameWithFirst = sFirstUpper + shortName.substr(1,shortName.length() - 1);

    const std::string sClassName = "Rt" + sTableNameWithFirst;
    const std::string sClassNameImp = sClassName + "Imp";

    sData += "\n";
    sData += "\n#ifndef RT_TPS_DATABASE_INTERFACE_OBJECT_" + sTableNameUpper + "_H_";
    sData += "\n#define RT_TPS_DATABASE_INTERFACE_OBJECT_" + sTableNameUpper + "_H_";

    sData += "\n";
    sData += "\n#include \"RtTpsDatabaseWrapper/rt_tps_database_defs.h\"";
    sData += "\n#include \"RtTpsDatabaseWrapper/rt_tps_database_interface_object_base.h\"";
    sData += "\n#include \"RtTpsDatabaseWrapper/rt_tps_database_common_enums.h\"";

    if (table.bHasDateTime)
    {
        sData += "\n#include \"boost/date_time/gregorian/gregorian_types.hpp\"";
        sData += "\n#include \"boost/date_time/posix_time/posix_time_types.hpp\"";
    }

	if (table.bIsSeriesable)
	{
		sData += "\n#include \"boost/archive/xml_iarchive.hpp\"";
		sData += "\n#include \"boost/archive/xml_oarchive.hpp\"";
	}
	if (table.bHasDateTime)
	{
		sData += "\n#include \"boost/date_time/posix_time/time_serialize.hpp\"";
	}
	

    switch(table.enTableType)
    {
    case TYPE_POI:
        {
            sData += "\n#include <map>";
        }
        break;
    case TYPE_BLOCK:
        {
            sData += "\n#include <vector>";
            sData += "\n#include \"RtTpsDatabaseWrapper/rt_tps_database_data.h\"";
        }
        break;
    case TYPE_COMMISSIONEDUNIT:
        {
            sData += "\n#include <vector>";
            sData += "\n#include <map>";
        }
        break;
    case TYPE_CONTOUR:
        {
            sData += "\n#include <vector>";
            sData += "\n#include \"RtTpsDatabaseWrapper/rt_tps_database_data.h\"";
        }
        break;
    case TYPE_CT2DENSITY:
        {
            sData += "\n#include <map>";
        }
        break;
    case TYPE_MLCSHAPE:
        {
            sData += "\n#include \"RtTpsDatabaseWrapper/rt_tps_database_data.h\"";
        }
        break;
    case TYPE_VOI:
        {
            sData += "\n#include <vector>";
        }
        break;
    case TYPE_APPLICATOR:
        {
            sData += "\n#include \"RtTpsDatabaseWrapper/rt_tps_database_data.h\"";
        }
        break;
    default:
        break;
    }

    sData += "\n";
    sData += "\nRT_TPS_DATABASE_BEGIN_NAMESPACE;";

    //////////////////////////////////////////////////////////////////////////
    //enum PLAN_FIELD
    sData += "\n";
    sData += "\nenum " + table.sShortNameUpper + "_FIELD";
    sData += "\n{";
    int iTemp(0);
    std::string sEnum2String = "\n};";

    sEnum2String += "\n";
    sEnum2String += "\nstatic const char* ENUM2STRING_" + table.sShortNameUpper + "_FIELD[] =";
    sEnum2String += "\n{";
    
    std::string sEnum2StringNull = "\n};";
    sEnum2StringNull += "\n\nstatic const char* ENUM2STRING_" + table.sShortNameUpper + "_FIELD_NULL = \"";
    for (auto itr=table.vColumnItems.cbegin(); itr!=table.vColumnItems.cend(); ++itr)
    {
        //if(table.sShortName=="voi"){
        //    if ((*itr).sColumnName == "red"
        //        ||(*itr).sColumnName == "green"
        //        ||(*itr).sColumnName == "blue"
        //        ||(*itr).sColumnName == "alpha"
        //        ||(*itr).sColumnName == "pat2volumematrix"
        //        ||(*itr).sColumnName == "interpolate"     
        //        ){
        //            continue;
        //    }
        //}
        //else if (table.sShortName == "beamsegment"){  
        //    if ((*itr).sColumnName == "t_beam_to_pat"){

        //        continue;
        //    }
        //}

        if (0 == iTemp)
        {
            sData += "\n    " + table.sShortNameUpper + "_" + (*itr).sColumnNameUpper + " = 0,";
        } 
        else
        {
            sData += "\n    " + table.sShortNameUpper + "_" + (*itr).sColumnNameUpper + ",";
        }
        sEnum2String += "\n    \"" + (*itr).sColumnNameUpper + "\",";
        sEnum2StringNull += (*itr).bIsNullable ? "0" : "1";
        ++iTemp;
    }
    sData += "\n    " + table.sShortNameUpper + "_FIELD_MAX";
    sEnum2String += "\n    \"" + table.sShortNameUpper + "_FIELD_MAX\"";
    sEnum2StringNull += "\";";

    sData += sEnum2String;

    sData += sEnum2StringNull;

    //////////////////////////////////////////////////////////////////////////
    if (table.bHasDateTime)
    {
        sData += "\n";
        sData += "\ntypedef boost::posix_time::ptime DATETIME_BOOST;";
        sData += "\ntypedef boost::posix_time::time_duration TIME_BOOST;";
        sData += "\ntypedef boost::gregorian::date DATE_BOOST;";
    }
    sData += "\n";
    sData += "\nclass " + sClassNameImp +";";
    switch(table.enTableType)
    {
    case TYPE_SERIES:
        {
            sData += "\nclass RtImage3DHeader;";
        }
        break;
    case TYPE_BEAMSEGMENT:
        {
            sData += "\nclass RtMlcshape;";
            sData += "\nclass RtContour;";
            sData += "\nclass RtMlcshape;";
        }
        break;
    case TYPE_NORMGROUP:
    case TYPE_PLAN:
        {
            sData += "\nclass RtDosegrid;";
        }
        break;
    case TYPE_BEAM:
        {
            sData += "\nclass RtBeamsegment;";
            sData += "\nclass RtBlock;";
            sData += "\nclass RtDosegrid;";
            sData += "\nclass RtVoi;";
            sData += "\nclass RtApplicator;";
        }
        break;
    case TYPE_VOI:
        {
            sData += "\nclass RtContour;";
        }
        break;
    default:
        break;
    }

    sData += "\n";
    sData += "\nclass RT_DB_EXPORT Rt" + sTableNameWithFirst + " : public RtDatabaseObject";
    sData += "\n{";

    switch(table.enTableType)
    {
    case TYPE_BLOCK:
        {
            sData += "\n  friend class RtBeamImp;";
            sData += "\n  friend class RtDatabaseHelper;";

            sData += "\n";
            sData += "\nprotected:";
            sData += "\n    Rt" + sTableNameWithFirst +"();";
            sData += "\n";
            sData += "\npublic:";
        }
        break;
    //case TYPE_MLCSHAPE:
    //    {
    //        //sData += "\n    friend class RtBeamsegmentImp;";
    //        //sData += "\n    friend class RtBeamsegment;";
    //        //sData += "\n    friend class RtDatabaseWrapper;";
    //        //sData += "\n    friend class RtDatabaseHelper;";
    //        sData += "\n";
    //        sData += "\npublic:";
    //        sData += "\n    Rt" + sTableNameWithFirst +"();";
    //        sData += "\n";
    //        sData += "\npublic:";
    //    }
    //    break;
    default:
        sData += "\npublic:";
        sData += "\n";
        if (!table.bIsMap && table.HasNameUid()){
            sData += "\n    //default is false to improve performance";
            sData += "\n    Rt" + sTableNameWithFirst +"(bool bGeneraterUid = false);";
        } 
        else{
            sData += "\n    Rt" + sTableNameWithFirst +"();";
        }
        break;
    }

    sData += "\n";
    sData += "\n    ~Rt" + sTableNameWithFirst +"();";

    //RtBlock(const RtBlock& block);
    sData += "\n";
    sData += "\n    " + sClassName +"(const " + sClassName + "& " +shortName +");";

    //RtBlock& operator=(const RtBlock& block);
    sData += "\n";
    sData += "\n    " + sClassName +"& operator = (const " + sClassName + "& " +shortName +");";

    //const std::string& get_uid() const;
    //void set_uid(const std::string& uid);
    WriteFileInUTF8(sFileNameOutput, sData);

    std::wstring wData(L"");
    rt_tps_database_interface_object_h_getset(table, wData);
    WriteFileInUTF8Added(sFileNameOutput, wData);

    sData = "";
    //////////////////////////////////////////////////////////////////////////
    sData += "\n";
    switch(table.enTableType){
    case TYPE_PLAN:
        {
            sData += "\n";
            sData += "\n    //////////////////////////////////////////////////////////////////////////";
            sData += "\n    RtDosegrid* get_dosegrid() const;";
        }
        break;
    case TYPE_POI:
        {
            sData += "\n    //////////////////////////////////////////////////////////////////////////";
            sData += "\n    std::map<std::string, float> get_poidosemap() const;";
            sData += "\n    void set_poidosemap(std::map<std::string, float>& poi_dosemap);";
            sData += "\n    float get_poidose(const std::string& beamuid) const;";
            sData += "\n    void set_poidose(const std::string& beamuid, float poi_dose);";
            sData += "\n    float get_dose() const;";
            sData += "\n    void set_dose(const float& fdose);";
        }
        break;
    case TYPE_BEAM:
        {
            sData += "\n    //////////////////////////////////////////////////////////////////////////";
            sData += "\n    //manually NOT from database columns!";
            sData += "\n    std::vector<RtBeamsegment*> get_beamsegments() const;";
            sData += "\n    void set_beamsegments(std::vector<RtBeamsegment*> beamsegments);";
            sData += "\n";
            sData += "\n    RtDosegrid* get_dosegrid();";
            sData += "\n    RtDosegrid* get_dosegrid() const;";

            sData += "\n";
            sData += "\n    void create_aperture_block();";
            sData += "\n    void remove_aperture_block();";
            sData += "\n    //Should NOT delete the pointer outside!";
            sData += "\n    RtBlock* get_aperture_block() const;";
            sData += "\n    ";
            sData += "\n    void create_shield_block();";
            sData += "\n    void remove_shield_block();";
            sData += "\n    //Should NOT delete the pointer outside!";
            sData += "\n    RtBlock* get_shield_block() const;";
        }
        break;
    case TYPE_BEAMSEGMENT:
        {
            sData += "\n    //////////////////////////////////////////////////////////////////////////";
            sData += "\n    //if null then create one. Should NOT delete the pointer outside!";
            sData += "\n    RtContour* get_beamoutline();";
            sData += "\n";
            sData += "\n    //must have. Should NOT delete the pointer outside!";
            sData += "\n    RtMlcshape* get_startmlcshape() const;";
            sData += "\n    RtMlcshape* get_startmlcshape();";
            sData += "\n";
            sData += "\n    //must have. Should NOT delete the pointer outside!";
            sData += "\n    RtMlcshape* get_endmlcshape() const;";
            sData += "\n    RtMlcshape* get_endmlcshape();";
        }
        break;
    case TYPE_COMMISSIONEDUNIT:
        {
            sData += "\n    //////////////////////////////////////////////////////////////////////////";
            sData += "\n    const std::map<std::string, float>& get_discrete_trayfactor() const;";
            sData += "\n    void set_discrete_trayfactor(std::map<std::string, float>& mapTrayfactors);";
        }
        break;

    case TYPE_NORMGROUP:
        {
            sData += "\n    //////////////////////////////////////////////////////////////////////////";
            sData += "\n    RtDosegrid* get_dosegrid() const;";
        }
        break;
    case TYPE_SERIES:
        {
            sData += "\n    //////////////////////////////////////////////////////////////////////////";
            sData += "\n    RtImage3DHeader* get_header();";
            sData += "\n    const RtImage3DHeader* get_header() const;";
            sData += "\n";
            sData += "\n    char* get_imagedata(unsigned long* ulSize) const;";
            sData += "\n    void set_imagedata(char* imagedata, unsigned long ulSize);";
            sData += "\n";
            sData += "\n    std::string get_slicethickness() const;";
            sData += "\n    void set_slicethickness(const std::string& slicethickness);";
            sData += "\n    ";
            sData += "\n    std::string get_studyid() const;";
            sData += "\n    void set_studyid(const std::string& studyid);";
            sData += "\n    ";
            sData += "\n    std::string get_studydescription() const;";
            sData += "\n    void set_studydescription(const std::string& studydescription);";
            sData += "\n    ";
            sData += "\n    std::string get_patientid() const;";
            sData += "\n    void set_patientid(const std::string& patientid);";
            sData += "\n    ";
            sData += "\n    std::string get_patientbirthdate() const;";
            sData += "\n    void set_patientbirthdate(const std::string& patientbirthdate);";
            sData += "\n    ";
            sData += "\n    std::string get_patientage() const;";
            sData += "\n    void set_patientage(const std::string& patientage);";
            sData += "\n    ";
            sData += "\n    std::string get_patientsex() const;";
            sData += "\n    void set_patientsex(const std::string& patientsex);";
        }
        break;
    case TYPE_IMAGE:
        {
            sData += "\n    //dcmfiledata mediumblob For TPS";
            sData += "\n    char* get_dcmfiledata(unsigned long* ulSize) const;";
            sData += "\n    void set_dcmfiledata(char* pData, unsigned long ulSize);";
        }
        break;
    case TYPE_VOI:
        {
            sData += "\n    //////////////////////////////////////////////////////////////////////////";
            sData += "\n    const float* get_color() const;";
            sData += "\n    void set_color(const float* color);";
            sData += "\n";
            sData += "\n    const float* get_pat2volumematrix() const;";
            sData += "\n    void set_pat2volumematrix(const float* value);";
            sData += "\n";
            sData += "\n    std::vector<bool> get_interpolate() const;";
            sData += "\n    void set_interpolate(const std::vector<bool>& value);";
            sData += "\n";
            sData += "\n    std::vector<RtContour*> get_contours() const;";
            sData += "\n    std::vector<RtContour*> get_contours();";
            sData += "\n    void set_contours(const std::vector<RtContour*>& vtContour);";
            sData += "\n";
            sData += "\n    std::string get_seriesuid() const;";
            sData += "\n    void set_seriesuid(const std::string& seriesuid);";
        }
        break;
    default: 
        break;
    }

    //////////////////////////////////////////////////////////////////////////
    //private
    sData += "\n";
    sData += "\nprivate:";

	if (table.bIsSeriesable)
	{
		sData += "\n    friend class boost::serialization::access;";
		sData += "\n    template<class Archive>";
		sData += "\n    void serialize( Archive &ar,const unsigned int version) {";
		sData += "\n  	  version;";
		sData += "\n  	  ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(RtDatabaseObject);";
		sData += "\n  	  ar & BOOST_SERIALIZATION_NVP(m_pImp);";
		sData += "\n    }";
	}
	
    sData += "\n    " + sClassNameImp + "*                   m_pImp;"; 

    sData += "\n};";


    //////////////////////////////////////////////////////////////////////////
    //class RtPatientImp
    rt_tps_database_interface_object_h_imp_inline(table, sData);

    //class Imp private
    rt_tps_database_interface_object_h_imp_private(table, sData);

    //////////////////////////////////////////////////////////////////////////
    sData += "\n";
    sData += "\nRT_TPS_DATABASE_END_NAMESPACE";
    sData += "\n#endif";

    //////////////////////////////////////////////////////////////////////////
    WriteFileInUTF8(sFileNameOutput, sData, true);
}

void CodeGeneraterMysql::rt_tps_database_interface_object_cpp(TableItem& table)
{
    //write rt_tps_database_interface_object_XXX_cpp
    //std::ofstream file_rt_tps_database_interface_object_h;
    std::string shortName = table.sTableName.substr(3,table.sTableName.length());
    std::string sFileName="rt_tps_database_interface_object_" + shortName + ".cpp";
    const std::string sFileNameOutput="output//objects//" + sFileName;
    //file_rt_tps_database_interface_object_h.open(sFileNameOutput, std::ios::out | std::ios::binary);

    std::string sData("");
    sData += "//////////////////////////////////////////////////////////////////////////";
    sData += "\n/// \\defgroup Radio Therapy Business Unit";
    sData += "\n///  Copyright, (c) Shanghai United Imaging Healthcare Inc., 2016";
    sData += "\n///  All rights reserved.";
    sData += "\n///";
    sData += "\n///  \\author  ZHOU qiangqiang  mailto:qiangqiang.zhou@united-imaging.com";
    sData += "\n///";
    sData += "\n///  \\file      " + sFileName;
    sData += "\n///  \\brief     This file was generated by CodeGenerater.exe ";
    sData += "\n///              From database version: " + m_DatabaseVersion;
    sData += "\n///";
    sData += "\n///  \\version 1.0";
    sData += "\n///  \\date    " + m_sCurrentDate;
    sData += "\n///  \\{";
    sData += "\n//////////////////////////////////////////////////////////////////////////";

    std::string sTableNameUpper = shortName;
    transform(shortName.begin(), shortName.end(), sTableNameUpper.begin(), toupper);
    std::string sFirstUpper = shortName.substr(0,1);
    transform(sFirstUpper.begin(), sFirstUpper.end(), sFirstUpper.begin(), toupper);
    std::string sTableNameWithFirst = sFirstUpper + shortName.substr(1,shortName.length() - 1);
                                                                  
    const std::string sClassName = "Rt" + sTableNameWithFirst;
    const std::string sClassNameImp = sClassName + "Imp";

    sData += "\n";
    sData += "\n#include \"StdAfx.h\"";
    sData += "\n#include \"RtTpsDatabaseWrapper/rt_tps_database_interface_object_" + shortName + ".h\"";

    if(shortName=="series"){
        sData += "\n#include \"RtTpsDatabaseWrapper/rt_tps_database_interface_object_image3d.h\"";
    }
    else if (shortName == "normgroup"){
        sData += "\n#include \"RtTpsDatabaseWrapper/rt_tps_database_interface_object_dosegrid.h\"";
    }
    else if (shortName == "plan"){
        sData += "\n#include \"RtTpsDatabaseWrapper/rt_tps_database_interface_object_dosegrid.h\"";
    }
    else if (shortName == "beamsegment"){
        sData += "\n#include \"RtTpsDatabaseWrapper/rt_tps_database_interface_object_contour.h\"";
        sData += "\n#include \"RtTpsDatabaseWrapper/rt_tps_database_interface_object_mlcshape.h\"";
    }

    switch(table.enTableType)
    {
    case TYPE_CT2DENSITY:
        {
            sData += "\n#include <map>";
        }
        break;
    case TYPE_BEAM:
        {
            sData += "\n#include \"RtTpsDatabaseWrapper/rt_tps_database_interface_object_beamsegment.h\"";
            sData += "\n#include \"RtTpsDatabaseWrapper/rt_tps_database_interface_object_dosegrid.h\"";
            sData += "\n#include \"RtTpsDatabaseWrapper/rt_tps_database_interface_object_block.h\"";
            sData += "\n#include \"RtTpsDatabaseWrapper/rt_tps_database_interface_object_voi.h\"";
        }
        break;
    case TYPE_VOI:
        {
            sData += "\n#include \"RtTpsDatabaseWrapper/rt_tps_database_interface_object_contour.h\"";
        }
        break;
    default:
        break;
    }

    //other header files
    if (!table.bIsMap)
    {
        sData += "\n#include \"RtTpsDatabaseWrapper/rt_tps_database_uid_generater.h\"";
    }

    sData += "\n";
    sData += "\nRT_TPS_DATABASE_BEGIN_NAMESPACE;";

    //////////////////////////////////////////////////////////////////////////
    //RtNormgroupImp::RtNormgroupImp():
    rt_tps_database_interface_object_cpp_imp_copy(table, sData);

    //////////////////////////////////////////////////////////////////////////
    sData += "\n\n//////////////////////////////////////////////////////////////////////////";

    if (!table.bIsMap && table.HasNameUid())
    {
        sData += "\n" + sClassName +"::" + sClassName + "(bool bGeneraterUid /*= false*/)";
        sData += "\n{";
        sData += "\n    m_pImp = new " + sClassNameImp +"(bGeneraterUid);";
    } 
    else
    {
        sData += "\n" + sClassName +"::" + sClassName + "()";
        sData += "\n{";
        sData += "\n    m_pImp = new " + sClassNameImp +"();";
    }
    sData += "\n    set_flags(" + sTableNameUpper +"_FIELD_MAX);";
    sData += "\n    set_field_null((char*)ENUM2STRING_" + sTableNameUpper +"_FIELD_NULL);";
    sData += "\n}";

    sData += "\n";
    sData += "\n" + sClassName +"::" + sClassName + "(const " + sClassName +"& " + shortName +"):";
    sData += " RtDatabaseObject(" + shortName + "),";
    sData += "\n   m_pImp(new " +sClassNameImp + "(*" + shortName +".m_pImp))";
    sData += "\n{";
    sData += "\n}";

    sData += "\n";
    sData += "\n" + sClassName +"& " + sClassName + "::operator = (const " + sClassName +"& " + shortName +")";
    sData += "\n{";
    sData += "\n    if(this != &" +shortName +")";
    sData += "\n    {";
    sData += "\n        RtDatabaseObject::operator=(" + shortName + ");";
    sData += "\n        *this->m_pImp = *" + shortName + ".m_pImp;";
    sData += "\n    }";
    sData += "\n    return *this;";
    sData += "\n}";

    sData += "\n";
    sData += "\n" + sClassName +"::~" + sClassName + "()";
    sData += "\n{";
    sData += "\n    DEL_PTR(m_pImp);";
    sData += "\n}";

    //////////////////////////////////////////////////////////////////////////
    for (auto itr=table.vColumnItems.cbegin(); itr!=table.vColumnItems.cend(); ++itr)
    {

        sData += "\n";
        sData += "\n//" + (*itr).sColumnName;

        if(shortName=="voi"){
            if ((*itr).sColumnName == "red"
                ||(*itr).sColumnName == "green"
                ||(*itr).sColumnName == "blue"
                ||(*itr).sColumnName == "alpha"
                ||(*itr).sColumnName == "pat2volumematrix"
                ||(*itr).sColumnName == "interpolate"     
                ){
                    continue;
            }
        }
        else if (shortName == "beamsegment"){  
            if ((*itr).sColumnName == "t_beam_to_pat"){
                sData += "\nconst float* RtBeamsegment::get_t_beam_to_pat() const { return m_pImp->get_t_beam_to_pat();}";
                sData += "\nvoid RtBeamsegment::set_t_beam_to_pat(const float* t_beam_to_pat) { m_pImp->set_t_beam_to_pat(t_beam_to_pat);}"; 
                continue;
            }
        }

        switch(table.enTableType)
        {
        case TYPE_MLCSHAPE:
            {
                if ((*itr).sColumnName == "leafpositions"){
                    sData += "\n//dLeafLowerPos dLeafUpperPos";
                    sData += "\nstd::vector<db_Point2d> RtMlcshape::get_leafpositions() const { return m_pImp->get_leafpositions();}";
                    sData += "\nstd::vector<db_Point2d> RtMlcshape::get_leafpositions() { return m_pImp->get_leafpositions();}";
                    sData += "\nvoid RtMlcshape::set_leafpositions(const std::vector<db_Point2d>& vLeafPos) { m_pImp->set_leafpositions(vLeafPos);}";
                    continue;
                }
            }
            break;
        case TYPE_APPLICATOR:
            {
                if ((*itr).sColumnName == "leafpositions"){
                    sData += "\n//dLeafLowerPos dLeafUpperPos";
                    sData += "\nstd::vector<db_Point2d> RtApplicator::get_leafpositions() const { return m_pImp->get_leafpositions();}";
                    sData += "\nstd::vector<db_Point2d> RtApplicator::get_leafpositions() { return m_pImp->get_leafpositions();}";
                    sData += "\nvoid RtApplicator::set_leafpositions(const std::vector<db_Point2d>& vLeafPos) { m_pImp->set_leafpositions(vLeafPos);}";
                    continue;
                }
            }
            break;
        case TYPE_BLOCK:
            {
                if ((*itr).sColumnName == "points"){
                    sData += "\nconst std::vector<db_Point2f> RtBlock::get_points() const { return m_pImp->get_points();}";
                    sData += "\nstd::vector<db_Point2f> RtBlock::get_points() { return m_pImp->get_points();}";
                    sData += "\nvoid RtBlock::set_points(const std::vector<db_Point2f>& vPoints) { m_pImp->set_points(vPoints);}";
                    continue;
                }
            }
            break;
        case TYPE_RTIMAGE:
            {
                if ((*itr).sColumnName == "pixeldata"){
                    sData += "\nvoid RtRtimage::set_pixel_data_buffer(char* pBuffer, unsigned long lLen) { m_pImp->set_pixel_data_buffer(pBuffer, lLen); }";
                    sData += "\nchar* RtRtimage::get_pixel_data_buffer(unsigned long* lLen) const { return m_pImp->get_pixel_data_buffer(lLen);}";
                    continue;
                }
            }
            break;
        case TYPE_CONTOUR:
            {
                if ((*itr).sColumnName == "points"){
                    sData += "\nstd::vector<db_Point3f> RtContour::get_contour_points() { return m_pImp->get_contour_points();}";
                    sData += "\nstd::vector<db_Point3f> RtContour::get_contour_points() const { return m_pImp->get_contour_points();}";
                    sData += "\nvoid RtContour::set_contour_points(const std::vector<db_Point3f>& vPoints) { m_pImp->set_contour_points(vPoints);}";
                    continue;
                }
            }
            break;
        case TYPE_CT2DENSITY:
            {
                if ((*itr).sColumnName == "ct2densityvalue"){
                    sData += "\nconst std::map<int, float>& RtCt2density::get_ct2densitymap() const {return m_pImp->get_ct2densitymap();}";
                    sData += "\nconst float* RtCt2density::get_ct2densitybuffer() const {return m_pImp->get_ct2densitybuffer();}";
                    sData += "\nvoid RtCt2density::set_ct2densitymap(const std::map<int, float>& ct2densityMap) { m_pImp->set_ct2densitymap(ct2densityMap);}";
                    continue;
                }
            }
            break;
        case TYPE_DOSEGRID:
            {
                if ((*itr).sColumnName == "grid_to_pat_t"){
                    sData += "\nconst float* RtDosegrid::get_grid_to_pat_t() const { return m_pImp->get_grid_to_pat_t(); }";
                    sData += "\nvoid RtDosegrid::set_grid_to_pat_t(const float* grid_to_pat_t) { m_pImp->set_grid_to_pat_t(grid_to_pat_t); }";
                    continue;
                }
                else if ((*itr).sColumnName == "dosegridvalue"){
                    sData += "\nconst float* RtDosegrid::get_dosegrid_buffer() const { return m_pImp->get_dosegrid_buffer(); }";
                    sData += "\nfloat* RtDosegrid::get_dosegrid_buffer() { return m_pImp->get_dosegrid_buffer(); }";
                    sData += "\nvoid RtDosegrid::clear_dosegrid_buffer() { m_pImp->clear_dosegrid_buffer(); }";
                    sData += "\nfloat* RtDosegrid::create_dosegrid_buffer() { return m_pImp->create_dosegrid_buffer(); }";
                    continue;
                }
            }
            break;
        case TYPE_TABLECONTOUR:
            {
                if ((*itr).sColumnName == "points"){
                    sData += "\nstd::vector<db_Point3d> RtTablecontour::get_points() const{ return m_pImp->get_points();}";
                    sData += "\nstd::vector<db_Point3d> RtTablecontour::get_points() { return m_pImp->get_points();}";
                    sData += "\nvoid RtTablecontour::set_points(const std::vector<db_Point3d>& vPoints){ m_pImp->set_points(vPoints);}";
                    continue;
                }
            }
            break;
        default:
            break;
        }

        const std::string sType = (*itr).sCppDataType;
        sData += "\n" +sType +" " + sClassName + "::get_" + (*itr).sColumnName + "() const { return m_pImp->get_" + (*itr).sColumnName + "();}"; 
        sData += "\nvoid " + sClassName + "::set_" + (*itr).sColumnName + 
            "(const " + sType + "& " + (*itr).sColumnName + ")";
        sData += "\n{";
        sData += "\n    m_pImp->set_" + (*itr).sColumnName + "(" + (*itr).sColumnName + ");";
        sData += "\n    enable_field(" + sTableNameUpper + "_" + (*itr).sColumnNameUpper + ");";
        sData += "\n    setdirty_field(" + sTableNameUpper + "_" + (*itr).sColumnNameUpper + ", true);";
        sData += "\n}"; 
    }

    //////////////////////////////////////////////////////////////////////////
    //manually part
    switch(table.enTableType)
    {
    case TYPE_PLAN:
        {
            sData += "\n";
            sData += "\n//////////////////////////////////////////////////////////////////////////";
            sData += "\nRtDosegrid* RtPlan::get_dosegrid() const {return m_pImp->get_dosegrid();}";
        }
        break;
    case TYPE_POI:
        {
            sData += "\n//////////////////////////////////////////////////////////////////////////";
            sData += "\nstd::map<std::string, float> RtPoi::get_poidosemap() const { return m_pImp->get_poidosemap();}";
            sData += "\nvoid RtPoi::set_poidosemap(std::map<std::string, float>& poi_dosemap) { m_pImp->set_poidosemap(poi_dosemap);}";
            sData += "\nfloat RtPoi::get_poidose(const std::string& beamuid) const { return m_pImp->get_poidose(beamuid);}";
            sData += "\nvoid RtPoi::set_poidose(const std::string& beamuid, float poi_dose) { m_pImp->set_poidose(beamuid, poi_dose);}";
            sData += "\n";
            sData += "\nfloat RtPoi::get_dose() const { return m_pImp->get_dose();}";
            sData += "\nvoid RtPoi::set_dose(const float& fdose) { m_pImp->set_dose(fdose);}";
        }
        break;
    case TYPE_BEAM:
        {
            sData += "\n//////////////////////////////////////////////////////////////////////////";
            sData += "\n//manually NOT from database columns!";
            sData += "\nstd::vector<RtBeamsegment*> RtBeam::get_beamsegments() const {return m_pImp->get_beamsegments();}";
            sData += "\nvoid RtBeam::set_beamsegments(std::vector<RtBeamsegment*> beamsegments) {m_pImp->set_beamsegments(beamsegments);}";
            sData += "\n";
            sData += "\nRtDosegrid* RtBeam::get_dosegrid() {return m_pImp->get_dosegrid();}";
            sData += "\nRtDosegrid* RtBeam::get_dosegrid() const {return m_pImp->get_dosegrid();}";
            sData += "\n";
            sData += "\nvoid RtBeam::create_aperture_block() { m_pImp->create_aperture_block();}";
            sData += "\nRtBlock* RtBeam::get_aperture_block() const {return m_pImp->get_aperture_block();}";
            sData += "\nvoid RtBeam::remove_aperture_block() {m_pImp->remove_aperture_block();}";
            sData += "\n";
            sData += "\nvoid RtBeam::create_shield_block() { m_pImp->create_shield_block();}";
            sData += "\nRtBlock* RtBeam::get_shield_block() const {return m_pImp->get_shield_block();}";
            sData += "\nvoid RtBeam::remove_shield_block() {m_pImp->remove_shield_block();}";

        }
        break;
    case TYPE_BEAMSEGMENT:
        {
            sData += "\n//////////////////////////////////////////////////////////////////////////";
            sData += "\n//if null then create one. Should NOT delete the pointer outside!";
            sData += "\nRtContour* RtBeamsegment::get_beamoutline() { return m_pImp->get_beamoutline(); }";
            sData += "\n";
            sData += "\n//must have. Should NOT delete the pointer outside!";
            sData += "\nRtMlcshape* RtBeamsegment::get_startmlcshape(){ return m_pImp->get_startmlcshape(); }";
            sData += "\nRtMlcshape* RtBeamsegment::get_startmlcshape() const { return m_pImp->get_startmlcshape(); }";
            sData += "\n";
            sData += "\n//must have. Should NOT delete the pointer outside!";
            sData += "\nRtMlcshape* RtBeamsegment::get_endmlcshape() { return m_pImp->get_endmlcshape(); }";
            sData += "\nRtMlcshape* RtBeamsegment::get_endmlcshape() const { return m_pImp->get_endmlcshape(); }";
        }
        break;
    case TYPE_COMMISSIONEDUNIT:
        {
            sData += "\n//////////////////////////////////////////////////////////////////////////";
            sData += "\nconst std::map<std::string, float>& RtCommissionedunit::get_discrete_trayfactor() const { return m_pImp->get_discrete_trayfactor(); }";
            sData += "\nvoid RtCommissionedunit::set_discrete_trayfactor(std::map<std::string, float>& mapTrayfactors) { m_pImp->set_discrete_trayfactor(mapTrayfactors); }";
        }
        break;
    case TYPE_NORMGROUP:
        {
            sData += "\n//////////////////////////////////////////////////////////////////////////";
            sData += "\nRtDosegrid* RtNormgroup::get_dosegrid() const {return m_pImp->get_dosegrid();}";
        }
        break;
    case TYPE_SERIES:
        {
            sData += "\n//////////////////////////////////////////////////////////////////////////";
            sData += "\nRtImage3D* RtSeries::get_image3d() const { return m_pImp->get_image3d();}";
            sData += "\n";
            sData += "\nstd::string RtSeries::get_slicethickness() const { return m_pImp->get_slicethickness();}";
            sData += "\nvoid RtSeries::set_slicethickness(const std::string& slicethickness) { m_pImp->set_slicethickness(slicethickness);}";
            sData += "\n";
            sData += "\nstd::string RtSeries::get_studyid() const { return m_pImp->get_studyid();}";
            sData += "\nvoid RtSeries::set_studyid(const std::string& studyid) { m_pImp->set_studyid(studyid);}";
            sData += "\n";
            sData += "\nstd::string RtSeries::get_studydescription() const { return m_pImp->get_studydescription();}";
            sData += "\nvoid RtSeries::set_studydescription(const std::string& studydescription) { m_pImp->set_studydescription(studydescription);}";
            sData += "\n";
            sData += "\nstd::string RtSeries::get_patientid() const { return m_pImp->get_patientid();}";
            sData += "\nvoid RtSeries::set_patientid(const std::string& patientid) { m_pImp->set_patientid(patientid);}";
            sData += "\n";
            sData += "\nstd::string RtSeries::get_patientbirthdate() const { return m_pImp->get_patientbirthdate();}";
            sData += "\nvoid RtSeries::set_patientbirthdate(const std::string& patientbirthdate) { m_pImp->set_patientbirthdate(patientbirthdate);}";
            sData += "\n";
            sData += "\nstd::string RtSeries::get_patientage() const { return m_pImp->get_patientage();}";
            sData += "\nvoid RtSeries::set_patientage(const std::string& patientage) { m_pImp->set_patientage(patientage);}";
            sData += "\n";
            sData += "\nstd::string RtSeries::get_patientsex() const { return m_pImp->get_patientsex();}";
            sData += "\nvoid RtSeries::set_patientsex(const std::string& patientsex) { m_pImp->set_patientsex(patientsex);}";
        }
        break;
    case TYPE_IMAGE:
        {
            sData += "\n//////////////////////////////////////////////////////////////////////////";
            sData += "\n//dcmfiledata";
            sData += "\nchar* RtImage::get_dcmfiledata(unsigned long* ulSize) const { return m_pImp->get_dcmfiledata(ulSize);}";
            sData += "\nvoid RtImage::set_dcmfiledata(char* pData, unsigned long ulSize) { m_pImp->set_dcmfiledata(pData, ulSize);}";
        }
        break;
    case TYPE_VOI:
        {
            sData += "\n//////////////////////////////////////////////////////////////////////////";
            sData += "\nconst float* RtVoi::get_color() const { return m_pImp->get_color();}";
            sData += "\nvoid RtVoi::set_color(const float* color) {m_pImp->set_color(color);}";
            sData += "\n";
            sData += "\nconst float* RtVoi::get_pat2volumematrix() const {return m_pImp->get_pat2volumematrix();}";
            sData += "\nvoid RtVoi::set_pat2volumematrix(const float* value) {m_pImp->set_pat2volumematrix(value);}";
            sData += "\n";
            sData += "\nstd::vector<bool> RtVoi::get_interpolate() const {return m_pImp->get_interpolate();}";
            sData += "\nvoid RtVoi::set_interpolate(const std::vector<bool>& value) {m_pImp->set_interpolate(value);}";
            sData += "\n";
            sData += "\nstd::vector<RtContour*> RtVoi::get_contours() const {return m_pImp->get_contours();}";
            sData += "\nstd::vector<RtContour*> RtVoi::get_contours() {return m_pImp->get_contours();}";
            sData += "\nvoid RtVoi::set_contours(const std::vector<RtContour*>& vtContour) { m_pImp->set_contours(vtContour);}";
            sData += "\n";
            sData += "\nstd::string RtVoi::get_seriesuid() const {return m_pImp->get_seriesuid();}";
            sData += "\nvoid RtVoi::set_seriesuid(const std::string& seriesuid) {m_pImp->set_seriesuid(seriesuid);}";
        }
        break;
    default:
        break;
    }

    sData += "\n";
    sData += "\nRT_TPS_DATABASE_END_NAMESPACE";

    //file_rt_tps_database_interface_object_h.write(sData.c_str() ,sData.length());
    //file_rt_tps_database_interface_object_h.close();
    WriteFileInUTF8(sFileNameOutput, sData);
}

void CodeGeneraterMysql::rt_tps_database_interface_object_h_getset(TableItem& table, std::wstring& wData)
{
    setlocale( LC_ALL, "chs" );

    for (auto itr=table.vColumnItems.cbegin(); itr!=table.vColumnItems.cend(); ++itr){

        wData += L"\n";
        std::wstring sItemComments = StringToWstring((*itr).sComments);
        //int size = MultiByteToWideChar(CP_UTF8, 0, &sItemComments[0], (int)sItemComments.size(), NULL, 0);
        //std::wstring wstr(size, 0 );
        //MultiByteToWideChar(CP_UTF8, 0, &sItemComments[0], (int)sItemComments.size(), &wstr[0], size);
        //std::wstring wComments = (wstr);//Str2Wstr
        //std::wcout<<"com:\t"<<wComments<<std::endl;

        wData += L"\n    /*" + StringToWstring((*itr).sColumnName) + L" " 
            + StringToWstring(Enum2StringDataType[(*itr).enDataType]) +  L"\n      " + sItemComments + L"*/";
        switch(table.enTableType)
        {
        case TYPE_MLCSHAPE:
        case TYPE_APPLICATOR:
            {
                if ((*itr).sColumnName == "leafpositions"){
                    wData += L"\n    //dLeafLowerPos dLeafUpperPos";
                    wData += L"\n    std::vector<db_Point2d> get_leafpositions() const;";
                    wData += L"\n    std::vector<db_Point2d> get_leafpositions();";
                    wData += L"\n    void set_leafpositions(const std::vector<db_Point2d>& vLeafPos);";
                    continue;
                }
            }
            break;
        case TYPE_VOI:
            {
                if ((*itr).sColumnName == "red"
                    ||(*itr).sColumnName == "green"
                    ||(*itr).sColumnName == "blue"
                    ||(*itr).sColumnName == "alpha"
                    ||(*itr).sColumnName == "pat2volumematrix"
                    ||(*itr).sColumnName == "interpolate"     
                    ){
                        continue;
                }
                else if ((*itr).sColumnName == "slices"){
                    wData += L"\n    float* get_slices() const;";
                    wData += L"\n    void set_slices(float*& slices);";
                    continue;
                }
            }
            break;
        case TYPE_CONTOUR:
            {
                if ((*itr).sColumnName == "points"){
                    wData += L"\n    std::vector<db_Point3f> get_contour_points() const;";
                    wData += L"\n    std::vector<db_Point3f> get_contour_points();";
                    wData += L"\n    void set_contour_points(const std::vector<db_Point3f>& vPoints);";
                    continue;
                }
            }
            break;
        case TYPE_BEAMSEGMENT:
            {
                if ((*itr).sColumnName == "t_beam_to_pat"){
                    wData += L"\n    const float* get_t_beam_to_pat() const;";
                    wData += L"\n    void set_t_beam_to_pat(const float* t_beam_to_pat);"; 
                    continue;
                }
            }
            break;
        case TYPE_DOSEGRID:
            {
                if ((*itr).sColumnName == "grid_to_pat_t"){
                    wData += L"\n    const float* get_grid_to_pat_t() const;";
                    wData += L"\n    void set_grid_to_pat_t(const float* grid_to_pat_t);";
                    continue;
                }
                else if ((*itr).sColumnName == "dosegridvalue"){
                    wData += L"\n    const float* get_dosegrid_buffer() const;";
                    wData += L"\n    float* get_dosegrid_buffer();";
                    wData += L"\n    void clear_dosegrid_buffer();";
                    wData += L"\n    float* create_dosegrid_buffer();";
                    wData += L"\n    void set_dosegrid_buffer(float* pBuffer);";
                    continue;
                }
            }
            break;
        case TYPE_BLOCK:
            {
                if ((*itr).sColumnName == "points"){
                    wData += L"\n    const std::vector<db_Point2f> get_points() const;";
                    wData += L"\n    std::vector<db_Point2f> get_points();";
                    wData += L"\n    void set_points(const std::vector<db_Point2f>& vPoints);";
                    continue;
                }
            }
            break;
        case TYPE_COMMISSIONEDUNIT:
            {
            }
            break;
        case TYPE_CT2DENSITY:
            {
                if ((*itr).sColumnName == "ct2densityvalue"){
                    wData += L"\n    const std::map<int, float>& get_ct2densitymap() const;";
                    wData += L"\n    void set_ct2densitymap(const std::map<int, float>& ct2densityMap);";
                    wData += L"\n    const float* get_ct2densitybuffer() const;";
                    continue;
                }
            }
            break;
        case TYPE_IMAGETRANSFORMATION:
            {
                if ((*itr).sColumnName == "registrationmatrix"){
                    wData += L"\n    const float* get_registrationmatrix() const;";
                    wData += L"\n    void set_registrationmatrix(const float* registrationmatrix);";
                    continue;
                }
            }
            break;
        case TYPE_MACHINE:
            {
                if ((*itr).sColumnName == "leafboundaries"){
                    wData += L"\n    std::vector<double> get_leafboundaries() const;";
                    wData += L"\n    void set_leafboundaries(const std::vector<double>& leafbounds);";
                    continue;
                }
            }
            break;
        case TYPE_NORMGROUP:
            {
                if ((*itr).sColumnName == "calculationpointuidlist"){
                    wData += L"\n    float* get_calculationpointuidlist() const;";
                    wData += L"\n    void set_calculationpointuidlist(float*& calculationpointuidlist);";
                    continue;
                }
            }
            break;
        case TYPE_RTIMAGE:
            {
                if ((*itr).sColumnName == "pixeldata"){
                    wData += L"\n    //PixelDataBuffer unsigned long lLen = m_iRows * m_iColumns * m_iFrame * m_iBitsAllocated/8 * m_iSamplePerPixel;";
                    wData += L"\n    void set_pixel_data_buffer(char* pBuffer, unsigned long lLen);";
                    wData += L"\n    char* get_pixel_data_buffer(unsigned long* lLen) const;";
                    continue;
                }
            }
            break;
        default:
            break;
        }

        const std::wstring sType = StringToWstring((*itr).sCppDataType);
        wData += L"\n    " +sType + L" get_" + StringToWstring((*itr).sColumnName) + L"() const;"; 
        wData += L"\n    void set_" + StringToWstring((*itr).sColumnName) + L"(const " + sType + L"& " + StringToWstring((*itr).sColumnName) + L");"; 
    }
}

void CodeGeneraterMysql::rt_tps_database_interface_object_h_imp_inline(TableItem& table, std::string& sData)
{
    std::string sClassNameImp = table.sClassName + "Imp";
    sData += "\n";
    sData += "\n";
    sData += "\n//////////////////////////////////////////////////////////////////////////";
    sData += "\n//for imp";
    sData += "\n";
    sData += "\nclass " + sClassNameImp;
    sData += "\n{";

    sData += "\n\npublic:";

    if (!table.bIsMap && table.HasNameUid()){
        sData += "\n    //default is false to improve performance";
		if (table.bIsSeriesable)
		{
			sData += "\n    RT_DB_EXPORT " + sClassNameImp + "(bool bGeneraterUid = false);";
		}
        
    } 
    else{
		if (table.bIsSeriesable)
		{
			sData += "\n    RT_DB_EXPORT " + sClassNameImp + "();";
		}
    }

    sData += "\n\n    virtual ~" + sClassNameImp +"();";

    //RtBlock(const RtBlock& block);
    sData += "\n";
    sData += "\n    " + sClassNameImp +"(const " + sClassNameImp + "& " +table.sShortName +");";

    //RtBlock& operator=(const RtBlock& block);
    sData += "\n";
    sData += "\n    " + sClassNameImp +"& operator = (const " + sClassNameImp + "& " +table.sShortName +");";

    for (auto itr=table.vColumnItems.cbegin(); itr!=table.vColumnItems.cend(); ++itr){

        //if(table.sShortName=="beam"){
        //    if ((*itr).sColumnName == "red"
        //        ||(*itr).sColumnName == "green"
        //        ||(*itr).sColumnName == "blue"
        //        ||(*itr).sColumnName == "alpha"
        //        ){
        //            continue;
        //    }
        //}
        //else 
            if(table.sShortName=="voi"){
            if ((*itr).sColumnName == "red"
                ||(*itr).sColumnName == "green"
                ||(*itr).sColumnName == "blue"
                ||(*itr).sColumnName == "alpha"
                ||(*itr).sColumnName == "pat2volumematrix"
                ||(*itr).sColumnName == "interpolate"     
                ){
                    continue;
            }
        }
        else if (table.sShortName == "beamsegment"){  
            if ((*itr).sColumnName == "t_beam_to_pat"){
                sData += "\n";
                sData += "\n    //" + (*itr).sColumnName;
                sData += "\n    inline const float* get_t_beam_to_pat() const { return m_t_beam_to_pat;}";
                sData += "\n    inline void set_t_beam_to_pat(const float* t_beam_to_pat) { memcpy(m_t_beam_to_pat, t_beam_to_pat, sizeof(float)*16);}"; 
                continue;
            }
        }

        sData += "\n";
        sData += "\n    //" + (*itr).sColumnName;
        const std::string sType = (*itr).sCppDataType;
        const std::string col = (*itr).sColumnName;

        switch(table.enTableType)
        {
        case TYPE_MLCSHAPE:
        case TYPE_APPLICATOR:
            {
                if ((*itr).sColumnName == "leafpositions"){
                    sData += "\n    //dLeafLowerPos dLeafUpperPos";
                    sData += "\n    inline std::vector<db_Point2d> get_leafpositions() const { return m_leafpositions;}";
                    sData += "\n    inline std::vector<db_Point2d> get_leafpositions() { return m_leafpositions;}";
                    sData += "\n    inline void set_leafpositions(const std::vector<db_Point2d>& vLeafPos) { m_leafpositions = vLeafPos;}";
                    continue;
                }
            }
            break;
        case TYPE_BLOCK:
            {
                if ((*itr).sColumnName == "points"){
                    sData += "\n    inline const std::vector<db_Point2f> get_points() const { return m_points;}";
                    sData += "\n    inline std::vector<db_Point2f> get_points() { return m_points; }";
                    sData += "\n    inline void set_points(const std::vector<db_Point2f>& vPoints) { m_points = vPoints; }";
                    continue;
                }
            }
            break;
        case TYPE_CONTOUR:
            {
                if ((*itr).sColumnName == "points"){
                    sData += "\n    inline std::vector<db_Point3f> get_contour_points() const { return m_contour_points;}";
                    sData += "\n    inline std::vector<db_Point3f> get_contour_points() { return m_contour_points;}";
                    sData += "\n    inline void set_contour_points(const std::vector<db_Point3f>& vPoints) { m_contour_points = vPoints;}";
                    continue;
                }
            }
            break;
        case TYPE_CT2DENSITY:
            {
                if ((*itr).sColumnName == "ct2densityvalue"){
                    sData += "\n    inline std::map<int, float>& get_ct2densitymap() {return m_ct2densityMap;}";
                    sData += "\n    inline const std::map<int, float>& get_ct2densitymap() const {return m_ct2densityMap;}";
                    sData += "\n    inline void set_ct2densitymap(const std::map<int, float>& ct2densityMap) {m_ct2densityMap = ct2densityMap;}";
                    sData += "\n    inline const float* get_ct2densitybuffer() const {return m_ct2densityBuffer;}";
                    continue;
                }
            }
            break;
        case TYPE_DOSEGRID:
            {
                if ((*itr).sColumnName == "grid_to_pat_t"){
                    sData += "\n    inline const float* get_grid_to_pat_t() const { return m_grid_to_pat_t;}";
                    sData += "\n    inline void set_grid_to_pat_t(const float* grid_to_pat_t) { memcpy(m_grid_to_pat_t, grid_to_pat_t, sizeof(float)*16);}"; 
                    continue;
                }
                else if ((*itr).sColumnName == "dosegridvalue"){
                    sData += "\n    inline const float* get_dosegrid_buffer() const {return m_dosegridvalue;}";
                    sData += "\n    inline float* get_dosegrid_buffer() {return m_dosegridvalue;}";
                    sData += "\n    inline void clear_dosegrid_buffer()";
                    sData += "\n    {";
                    sData += "\n        const int iSize = m_xcount * m_ycount * m_zcount;";
                    sData += "\n        if (iSize>0) memset(m_dosegridvalue, 0, sizeof(float) * iSize);";
                    sData += "\n    }";
                    sData += "\n    inline float* create_dosegrid_buffer()";
                    sData += "\n    {";
                    sData += "\n        const int iSize = m_xcount * m_ycount * m_zcount;";
                    sData += "\n        if (iSize>0) {";
                    sData += "\n            m_dosegridvalue = new float[iSize];";
                    sData += "\n            memset(m_dosegridvalue, 0, sizeof(float) * iSize);";
                    sData += "\n        }";
                    sData += "\n        return m_dosegridvalue;";
                    sData += "\n    }";

                    sData += "\n";
                    sData += "\n    inline void set_dosegrid_buffer(float* pBuffer)";
                    sData += "\n    {";
                    sData += "\n        if(nullptr == pBuffer){";
                    sData += "\n            if(nullptr != m_dosegridvalue) {";
                    sData += "\n                delete m_dosegridvalue;";
                    sData += "\n                m_dosegridvalue = nullptr;";
                    sData += "\n            }";
                    sData += "\n        }";
                    sData += "\n        else{";
                    sData += "\n            const int iSize = m_xcount * m_ycount * m_zcount;";
                    sData += "\n            if(nullptr != m_dosegridvalue && iSize > 0)";
                    sData += "\n                memcpy(m_dosegridvalue, pBuffer, sizeof(float) * iSize);";
                    sData += "\n        }";
                    sData += "\n    }";
                    continue;
                }
            }
            break;
        case TYPE_IMAGETRANSFORMATION:
            {
                if ((*itr).sColumnName == "registrationmatrix"){
                    sData += "\n    inline const float* get_registrationmatrix() const { return m_registrationmatrix;}";
                    sData += "\n    inline void set_registrationmatrix(const float* registrationmatrix) \
                             { memcpy(m_registrationmatrix, registrationmatrix, sizeof(float)*16);}";
                    continue;
                }
            }
            break;
        case TYPE_MACHINE:
            {
                if ((*itr).sColumnName == "leafboundaries"){
                    sData += "\n    inline std::vector<double> get_leafboundaries() const { return m_leafboundaries;};";
                    sData += "\n    inline void set_leafboundaries(const std::vector<double>& leafbounds) {m_leafboundaries = leafbounds;}";
                    continue;
                }
            }
            break;
        case TYPE_RTIMAGE:
            {
                if ((*itr).sColumnName == "pixeldata"){
                    sData += "\n    //PixelDataBuffer unsigned long lLen = m_iRows * m_iColumns * m_iFrame * m_iBitsAllocated/8 * m_iSamplePerPixel;";
                    sData += "\n    void set_pixel_data_buffer(char* pBuffer, unsigned long lLen);";
                    sData += "\n    char* get_pixel_data_buffer(unsigned long* lLen) const;";
                    continue;
                }
            }
            break;
        case TYPE_VOI:
            {
                if ((*itr).sColumnName == "slices"){
                    sData += "\n    inline float* get_slices() const { return m_slices;}";
                    sData += "\n    inline void set_slices(float*& slices) { m_slices = slices;}";
                    continue;
                }
            }
            break;
        //case TYPE_COMMISSIONEDUNIT:
        //    {
        //        if ((*itr).sColumnName == "discretedoserate"){
        //            sData += "\n    std::vector<float> get_discretedoserate() const { return m_discretedoserate;}";
        //            sData += "\n    void set_discretedoserate(std::vector<float>& discretedoserate) { m_discretedoserate = discretedoserate;}";
        //            continue;
        //        }
        //    }
        //    break;
        default:
            break;
        }

        sData += "\n    inline " +sType +" get_" + col + "() const { return m_" + col + ";}"; 
        sData += "\n    inline void set_" + col + "(const " + sType + "& " + col + ") { m_" + col + " = " + col + ";}"; 

    }

    //////////////////////////////////////////////////////////////////////////
    //more interfaces manually
    switch(table.enTableType)
    {
    case TYPE_CT2DENSITY:
        {
            //sData += "\n";
            //sData += "\n    //////////////////////////////////////////////////////////////////////////";
            //sData += "\n    inline std::map<int, float>& get_ct2densitymap() {return m_ct2densityMap;}";
            //sData += "\n    inline const std::map<int, float>& get_ct2densitymap() const {return m_ct2densityMap;}";
            //sData += "\n    inline void set_ct2densitymap(const std::map<int, float>& ct2densityMap) {m_ct2densityMap = ct2densityMap;}";
            //sData += "\n    inline const float* get_ct2densitybuffer() const {return m_ct2densityBuffer;}";
        }
        break;
    case TYPE_PLAN:
        {
            sData += "\n";
            sData += "\n    //////////////////////////////////////////////////////////////////////////";
            sData += "\n    inline RtDosegrid* get_dosegrid() const {return m_dosegrid;}";
        }
        break;
    case TYPE_POI:
        {
            sData += "\n    //////////////////////////////////////////////////////////////////////////";
            sData += "\n    inline std::map<std::string, float> get_poidosemap() const { return m_poi_dosemap;}";
            sData += "\n    inline void set_poidosemap(std::map<std::string, float>& poi_dosemap) { m_poi_dosemap = poi_dosemap;}";
            sData += "\n    inline float get_poidose(const std::string& beamuid) const";
            sData += "\n    {";
            sData += "\n        auto itr = m_poi_dosemap.find(beamuid);";
            sData += "\n        if (itr == m_poi_dosemap.end()) return 0.f;";
            sData += "\n        return itr->second;";
            sData += "\n    }";
            sData += "\n    inline void set_poidose(const std::string& beamuid, float poi_dose)";
            sData += "\n    {";
            sData += "\n        auto itr = m_poi_dosemap.find(beamuid);";
            sData += "\n        if (itr == m_poi_dosemap.end())";
            sData += "\n            m_poi_dosemap[beamuid] = poi_dose;";
            sData += "\n        else";
            sData += "\n            itr->second = poi_dose;";
            sData += "\n    }";
            sData += "\n";
            sData += "\n    inline float get_dose() const { return m_fDose;}";
            sData += "\n    inline void set_dose(const float& fdose) { m_fDose = fdose;}";
        }
        break;
    case TYPE_BEAM:
        {
            sData += "\n    //////////////////////////////////////////////////////////////////////////";
            sData += "\n    //manually NOT from database columns!";
            sData += "\n    inline std::vector<RtBeamsegment*> get_beamsegments() const { return m_beamsegments;}";
            sData += "\n    inline void set_beamsegments(std::vector<RtBeamsegment*> beamsegments) {m_beamsegments = beamsegments;}";
            sData += "\n";
            sData += "\n    RtDosegrid* get_dosegrid();";
            sData += "\n    RtDosegrid* get_dosegrid() const;";
            sData += "\n";
            sData += "\n    void create_aperture_block();";
            sData += "\n    void remove_aperture_block();";
            sData += "\n    RtBlock* get_aperture_block() const;";
            sData += "\n";
            sData += "\n    void create_shield_block();";
            sData += "\n    void remove_shield_block();";
            sData += "\n    RtBlock* get_shield_block() const;";

        }
        break;
    case TYPE_BEAMSEGMENT:
        {
            sData += "\n    //////////////////////////////////////////////////////////////////////////";
            sData += "\n    //if null then create one. Should NOT delete the pointer outside!";
            sData += "\n    inline RtContour* get_beamoutline() { return m_beam_outline;}";
            sData += "\n    inline RtContour* get_beamoutline() const { return m_beam_outline;}";
            sData += "\n";
            sData += "\n    //must have. Should NOT delete the pointer outside!";
            sData += "\n    inline RtMlcshape* get_startmlcshape() { return m_start_mlcshape;}";
            sData += "\n    inline RtMlcshape* get_startmlcshape() const { return m_start_mlcshape;}";
            sData += "\n";
            sData += "\n    //must have. Should NOT delete the pointer outside!";
            sData += "\n    inline RtMlcshape* get_endmlcshape() { return m_end_mlcshape;}";
            sData += "\n    inline RtMlcshape* get_endmlcshape() const { return m_end_mlcshape;}";
        }
        break;
    case TYPE_COMMISSIONEDUNIT:
        {
            sData += "\n    //////////////////////////////////////////////////////////////////////////";
            sData += "\n    inline const std::map<std::string, float>& get_discrete_trayfactor() const { return m_map_trayfactors; }";
            sData += "\n    inline void set_discrete_trayfactor(std::map<std::string, float>& mapTrayfactors) { m_map_trayfactors = mapTrayfactors; }";
        }
        break;

    case TYPE_NORMGROUP:
        {
            sData += "\n    //////////////////////////////////////////////////////////////////////////";
            sData += "\n    RtDosegrid* get_dosegrid() const;";
        }
        break;
    case TYPE_SERIES:
        {
            sData += "\n";
            sData += "\n    //////////////////////////////////////////////////////////////////////////";
            sData += "\n    inline RtImage3DHeader* get_header() { return m_header;}";
            sData += "\n    inline const RtImage3DHeader* get_header() const { return m_header;}";
            sData += "\n";
            sData += "\n    inline char* get_imagedata(unsigned long* ulSize) const { *ulSize=m_imagesize; return m_imagedata;}";
            sData += "\n    inline void set_imagedata(char* imagedata, unsigned long ulSize) ";
            sData += "\n    {";
            sData += "\n        if (nullptr != imagedata && imagedata != m_imagedata && ulSize>0)";
            sData += "\n        {";
            sData += "\n            DEL_ARRAY(m_imagedata);";
            sData += "\n            m_imagedata = new char[ulSize];";
            sData += "\n            memcpy(m_imagedata, imagedata, ulSize * sizeof(char));";
            sData += "\n            m_imagesize = ulSize;";
            sData += "\n        }";
            sData += "\n    }";
            sData += "\n";

            sData += "\n    inline std::string get_slicethickness() const { return m_slicethickness;}";
            sData += "\n    inline void set_slicethickness(const std::string& slicethickness) { m_slicethickness = slicethickness;}";
            sData += "\n    ";
            sData += "\n    inline std::string get_studyid() const { return m_studyid;}";
            sData += "\n    inline void set_studyid(const std::string& studyid) { m_studyid = studyid;}";
            sData += "\n    ";
            sData += "\n    inline std::string get_studydescription() const { return m_studydescription;}";
            sData += "\n    inline void set_studydescription(const std::string& studydescription) { m_studydescription = studydescription;}";
            sData += "\n    ";
            sData += "\n    inline std::string get_patientid() const { return m_patientid;}";
            sData += "\n    inline void set_patientid(const std::string& patientid) { m_patientid = patientid;}";
            sData += "\n    ";
            sData += "\n    inline std::string get_patientbirthdate() const { return m_patientbirthdate;}";
            sData += "\n    inline void set_patientbirthdate(const std::string& patientbirthdate) { m_patientbirthdate = patientbirthdate;}";
            sData += "\n    ";
            sData += "\n    inline std::string get_patientage() const { return m_patientage;}";
            sData += "\n    inline void set_patientage(const std::string& patientage) { m_patientage = patientage;}";
            sData += "\n    ";
            sData += "\n    inline std::string get_patientsex() const { return m_patientsex;}";
            sData += "\n    inline void set_patientsex(const std::string& patientsex) { m_patientsex = patientsex;}";
        }
        break;
    case TYPE_IMAGE:
        {
            sData += "\n";
            sData += "\n    //////////////////////////////////////////////////////////////////////////";
            sData += "\n    //dcmfiledata";
            sData += "\n    inline char* get_dcmfiledata(unsigned long* ulSize) const { *ulSize = m_size; return m_dcmfiledata;}";
            sData += "\n    inline void set_dcmfiledata(char* pData, unsigned long ulSize)";
            sData += "\n    {";
            sData += "\n        if (pData != m_dcmfiledata && nullptr != pData && ulSize>0){";
            sData += "\n            DEL_ARRAY(m_dcmfiledata);";
            sData += "\n            m_dcmfiledata = new char[ulSize];";
            sData += "\n            memcpy(m_dcmfiledata, pData, ulSize * sizeof(char));";
            sData += "\n            m_size = ulSize;";
            sData += "\n        }";
            sData += "\n    }";
        }
        break;
    case TYPE_VOI:
        {
            sData += "\n    //////////////////////////////////////////////////////////////////////////";
            sData += "\n    inline const float* get_color() const {return m_color;}";
            sData += "\n    inline void set_color(const float* color) {memcpy(m_color, color, 4 * sizeof(float));}";
            sData += "\n";
            sData += "\n    inline const float* get_pat2volumematrix() const {return m_pat2volumematrix;}";
            sData += "\n    inline void set_pat2volumematrix(const float* value) {memcpy(m_pat2volumematrix, value, sizeof(float)*16);}";
            sData += "\n";
            sData += "\n    inline std::vector<bool> get_interpolate() const {return m_vInterpolate;}";
            sData += "\n    inline void set_interpolate(const std::vector<bool>& value) {m_vInterpolate = value;}";
            sData += "\n";
            sData += "\n    inline std::vector<RtContour*> get_contours() {return m_vtContour;}";
            sData += "\n    inline std::vector<RtContour*> get_contours() const {return m_vtContour;}";
            sData += "\n    inline void set_contours(const std::vector<RtContour*>& contours) { m_vtContour = contours;}";
            sData += "\n";
            sData += "\n    //seriesuid";
            sData += "\n    inline std::string get_seriesuid() const { return m_seriesuid;}";
            sData += "\n    inline void set_seriesuid(const std::string& seriesuid) { m_seriesuid = seriesuid;}";
        }
        break;
    default:
        break;
    }
}

void CodeGeneraterMysql::rt_tps_database_interface_object_h_imp_private(TableItem& table, std::string& sData)
{
    const std::string shortName = table.sShortName;
    std::stringstream ss;
    std::string strNum;
    ss<<table.vColumnItems.size();
    ss>>strNum;
    std::string strWrite("");
    sData += "\n";
    sData += "\nprivate:    //"+strNum+" parameters";

	if (table.bIsSeriesable)
	{
		sData += "\n    friend class boost::serialization::access;";
		sData += "\n    template<class Archive>";
		sData += "\n    void serialize( Archive &ar,const unsigned int version) {";
		sData += "\n  	  version;";
		for (auto itr = table.vColumnItems.cbegin(); itr != table.vColumnItems.cend(); ++itr)
		{
			sData += "\n	    ar & BOOST_SERIALIZATION_NVP(m_" + itr->sColumnName + ");";
		}
		sData += "\n    }";
	}

	for (auto itr = table.vColumnItems.cbegin(); itr != table.vColumnItems.cend(); ++itr){

        std::string sttt = "\t\t\t\t\t\t";

        /*        if(shortName=="beam"){
        if ((*itr).sColumnName == "red"
        ||(*itr).sColumnName == "green"
        ||(*itr).sColumnName == "blue"
        ||(*itr).sColumnName == "alpha"
        ){
        continue;
        }
        }   */    
        if(shortName=="voi"){
            if ((*itr).sColumnName == "red"
                ||(*itr).sColumnName == "green"
                ||(*itr).sColumnName == "blue"
                ||(*itr).sColumnName == "alpha"
                ||(*itr).sColumnName == "pat2volumematrix"
                ||(*itr).sColumnName == "interpolate"
                ){
                    continue;
            }
        }  
        else if (shortName == "beamsegment"){  
            if ((*itr).sColumnName == "t_beam_to_pat"){ 
                sData += "\n    float                           m_t_beam_to_pat[16];"; 
                continue;
            }
        }


        const std::string sType = (*itr).sCppDataType;
        switch ((*itr).enDataType){
        case _int:
            sttt = "\t\t\t\t\t\t\t\t";
            break;
        case _tinyint:
        case _bool:
        case _float:
        case _double:  
            sttt = "\t\t\t\t\t\t\t";
            break;
        case _blob:
        case _mediumblob:
        case _longblob:
            sttt = "\t\t\t\t\t\t";
            break;
        case _datetime:
        case _timestamp:
            sttt = "\t\t\t\t\t";
            break;
        }

        //if (_blob == (*itr).enDataType){
        //    sData += "\n    " +sType + sttt +"m_" + (*itr).sColumnName + ";"; 
        //    sData += "\n    int" +sttt +"\tm_size_" + (*itr).sColumnName + ";"; 
        //}
        //else
        
        switch(table.enTableType)
        {
        case TYPE_MLCSHAPE:
        case TYPE_APPLICATOR:
            {
                if ((*itr).sColumnName == "leafpositions"){
                    sData += "\n    std::vector<db_Point2d>			m_leafpositions;//dLeafLowerPos dLeafUpperPos";
                    continue;
                }
            }
            break;
        case TYPE_BLOCK:
            {
                if ((*itr).sColumnName == "points"){
                    sData += "\n    std::vector<db_Point2f>         m_points;";
                    continue;
                }
            }
            break;
        case TYPE_CONTOUR:
            {
                if ((*itr).sColumnName == "points"){
                    sData += "\n    std::vector<db_Point3f>         m_contour_points;";
                    continue;
                }
            }
            break;
        case TYPE_CT2DENSITY:
            {
                if ((*itr).sColumnName == "ct2densityvalue"){
                    sData += "\n    std::map<int, float>            m_ct2densityMap;";
                    sData += "\n    float                           m_ct2densityBuffer[6025];// 这里根据晓清的输入，是定长的数组";
                    continue;
                }
            }
            break;
        case TYPE_IMAGETRANSFORMATION:
            {
                if ((*itr).sColumnName == "registrationmatrix"){
                    sData += "\n    float                           m_registrationmatrix[16];";
                    continue;
                }
            }
            break;
        case TYPE_MACHINE:
            {
                if ((*itr).sColumnName == "leafboundaries"){
                    sData += "\n    std::vector<double>              m_leafboundaries;";
                    continue;
                }
            }
            break;
        case TYPE_DOSEGRID:
            {
                if ((*itr).sColumnName == "grid_to_pat_t"){
                    sData += "\n    float                           m_grid_to_pat_t[16];"; 
                    continue;
                }
                else if ((*itr).sColumnName == "dosegridvalue"){
                    sData += "\n    float*                          m_dosegridvalue;"; 
                    continue;
                }
            }
            break;
        default:
            break;
        }


        sData += "\n    " +sType + sttt +"m_" + (*itr).sColumnName + ";"; 
        
    }

    if (shortName == "beamsegment"){
        sData += "\n    //////////////////////////////////////////////////////////////////////////";
        sData += "\n    RtContour*                      m_beam_outline;";
        sData += "\n    RtMlcshape*                     m_start_mlcshape;";
        sData += "\n    RtMlcshape*                     m_end_mlcshape;";
    }

    //else if(shortName=="dosegrid"){
    //    sData += "\n    //////////////////////////////////////////////////////////////////////////";
    //    sData += "\n    float*                          m_pBuffer;";
    //}
    else if(shortName=="normgroup"){
        sData += "\n    //////////////////////////////////////////////////////////////////////////";
        sData += "\n    RtDosegrid*                     m_dosegrid;";
    }
    else if(shortName=="plan"){
        sData += "\n    //////////////////////////////////////////////////////////////////////////";
        sData += "\n    RtDosegrid*                     m_dosegrid;";
    }

    switch(table.enTableType)
    {
    case TYPE_POI:
        {
            sData += "\n    //////////////////////////////////////////////////////////////////////////";
            sData += "\n    std::map<std::string, float>    m_poi_dosemap;";
            sData += "\n    float                           m_fDose;";
        }
        break;
    case TYPE_BEAM:
        {
            sData += "\n    //////////////////////////////////////////////////////////////////////////";
            sData += "\n    std::vector<RtBeamsegment*>     m_beamsegments;";
            sData += "\n    RtBlock*                        m_aperture_block;";
            sData += "\n    RtBlock*                        m_shield_block;";
            sData += "\n    RtDosegrid*                     m_dosegrid;";
        }
        break;
    case TYPE_BLOCK:
        {
            //sData += "\n    //////////////////////////////////////////////////////////////////////////";
            //sData += "\n    std::vector<db_Point2f>         m_points;";
        }
        break;
    case TYPE_COMMISSIONEDUNIT:
        {
            sData += "\n    //////////////////////////////////////////////////////////////////////////";
            sData += "\n    std::map<std::string, float>    m_map_trayfactors;";
        }
        break;
    case TYPE_SERIES:
        {
            sData += "\n    //////////////////////////////////////////////////////////////////////////";
            sData += "\n    RtImage3DHeader*                m_header;";
            sData += "\n    char*                           m_imagedata;// Pointer to ImagePixelData";
            sData += "\n    unsigned long                   m_imagesize;";
            sData += "\n    //from the requirement document of Dicom_Tag_of_ImageSet.pdf";
            sData += "\n    std::string                     m_slicethickness;";
            sData += "\n    std::string                     m_studyid;";
            sData += "\n    std::string                     m_studydescription;";
            sData += "\n    std::string                     m_patientid;";
            sData += "\n    std::string                     m_patientbirthdate;";
            sData += "\n    std::string                     m_patientage;";
            sData += "\n    std::string                     m_patientsex;";
        }
        break;
    case TYPE_IMAGE:
        {
            sData += "\n    //////////////////////////////////////////////////////////////////////////";
            sData += "\n    char*                           m_dcmfiledata;";
            sData += "\n    unsigned long                   m_size;";
        }
        break;
    case TYPE_VOI:
        {
            sData += "\n    //////////////////////////////////////////////////////////////////////////";
            sData += "\n    float                           m_color[4];";
            sData += "\n    float                           m_pat2volumematrix[16]; // 16 element of float";
            sData += "\n    std::vector<bool>               m_vInterpolate;";
            sData += "\n    std::vector<RtContour*>         m_vtContour;";
            sData += "\n    std::string                     m_seriesuid;";
        }
        break;
    default:
        break;
    }
    sData += "\n};";

}

void CodeGeneraterMysql::rt_tps_database_interface_object_cpp_imp_copy(TableItem& table, std::string& sData)
{
    const std::string sClassNameImp = table.sClassName + "Imp";
    const std::string shortName = table.sShortName;

    sData += "\n\n//////////////////////////////////////////////////////////////////////////";
    sData += "\n";

    if (!table.bIsMap && table.HasNameUid()){
        sData += "\n" + sClassNameImp +"::" + sClassNameImp + "(bool bGeneraterUid /*= false*/):";
    } 
    else{
        sData += "\n" + sClassNameImp +"::" + sClassNameImp + "():";
    }

    int iCount(0);
    for (auto itr=table.vColumnItems.cbegin(); itr!=table.vColumnItems.cend(); ++itr, ++iCount){

        if(shortName=="voi"){
            if ((*itr).sColumnName == "red"
                ||(*itr).sColumnName == "green"
                ||(*itr).sColumnName == "blue"
                ||(*itr).sColumnName == "alpha"
                ||(*itr).sColumnName == "pat2volumematrix"
                ||(*itr).sColumnName == "interpolate"
                ){
                    continue;
            }
        }
        else if (shortName == "beamsegment"){
            if ((*itr).sColumnName == "t_beam_to_pat"){
                continue;
            }
        }

        switch(table.enTableType)
        {
        case TYPE_APPLICATOR:
            {
                if ((*itr).sColumnName == "leafpositions"){
                    continue;
                }
            }
            break;
        case TYPE_CONTOUR:
        case TYPE_TABLECONTOUR:
        case TYPE_BLOCK:
            {
                if ((*itr).sColumnName == "points"){
                    continue;
                }
            }
            break;
        case TYPE_CT2DENSITY:
            {
                if ((*itr).sColumnName == "ct2densityvalue"){
                    continue;
                }
            }
            break;
        case TYPE_DOSEGRID:
            {
                if ((*itr).sColumnName == "dosegridvalue"){
                    sData += "\n    m_dosegridvalue(nullptr),";
                    continue;
                }
                else if ((*itr).sColumnName == "grid_to_pat_t"){
                    continue;
                }
            }
            break;
        case TYPE_MLCSHAPE:
            {
                if ((*itr).sColumnName == "leafpositions"){
                    continue;
                }
            }
            break;
        case TYPE_IMAGE:
            {
                if ((*itr).sColumnName == "dcmfileuid"){
                    sData += "\n    m_dcmfiledata(nullptr),";
                    sData += "\n    m_size(0),";
                }
            }
            break;
        default:
            break;
        }
        
        if ((*itr).sColumnName == "discretedoserate"){
            continue;
        }
        else if ((*itr).sColumnName == "leafboundaries"){
            continue;
        }

        std::string sLast = iCount == table.vColumnItems.size() -1 ? ")" : "),";
        std::string sName = "\n    m_" +(*itr).sColumnName +"("; 

        switch((*itr).enDataType){
        case _int:
        case _bigint:
            {  
                sData += sName + "0";
            }
            break;

        case _float:
            {  
                sData += sName + "0.f";
            }
            break;

        case _double:
            {  
                sData += sName + "0.";
            }
            break;

        case _bool:
        case _tinyint:
            {
                sData += sName + "false";
            }
            break;

        case _datetime:
        case _date:
        case _time:
        case _timestamp:
            {
                sData += sName + "boost::date_time::not_a_date_time";
            }
            break;

        case _blob:
        case _mediumblob:
        case _longblob:
            {
                if ((*itr).sCppDataType == "char*"){
                    sData += sName + "nullptr";
                } 
                else if ((*itr).sCppDataType == "std::string"){
                    sData += sName + "\"\"";
                }
                else{
                }
            }
            break;
        case _varchar:
        case _char:
        case _unknown:
        default:
            sData += sName + "\"\"";
            break;
        }

        sData += sLast;
    }

    sData += "\n{";
    if (!table.bIsMap && table.HasNameUid()){
        sData += "\n    //create uid";
        sData += "\n    m_uid = bGeneraterUid ? RtUidGenerater::GeneraterUid() : \"\";"; 
    }

    switch(table.enTableType)
    {
    case TYPE_CT2DENSITY:
        {
            sData += "\n    m_ct2densityMap.clear();";
            sData += "\n    memset(m_ct2densityBuffer, 0, sizeof(float) * 6025);";
        }
        break;
    case TYPE_PLAN:
    case TYPE_NORMGROUP:
        {
            sData += "\n    m_dosegrid = new RtDosegrid(bGeneraterUid);";
        }
        break;
    case TYPE_BEAM:
        {
            sData += "\n    m_beamsegments.clear();";
            sData += "\n    m_aperture_block = nullptr;";
            sData += "\n    m_shield_block = nullptr;";
            sData += "\n    m_dosegrid = new RtDosegrid(bGeneraterUid);";
        }
        break;
    case TYPE_BEAMSEGMENT:
        {
            sData += "\n    memset(m_t_beam_to_pat, 0, sizeof(float)*16);";
            sData += "\n    m_start_mlcshape = new RtMlcshape(bGeneraterUid);";
            sData += "\n    m_start_mlcshape->set_isstartmlcshape(true);";
            sData += "\n    m_end_mlcshape = new RtMlcshape(bGeneraterUid);";
            sData += "\n    m_end_mlcshape->set_isstartmlcshape(false);";
            sData += "\n    m_beam_outline = new RtContour(bGeneraterUid);";
        }
        break;
    case TYPE_VOI:
        {
            sData += "\n    memset(m_color, 0, sizeof(float)*4);";
            sData += "\n    memset(m_pat2volumematrix, 0, sizeof(float)*16);";
            sData += "\n    m_vInterpolate.clear();";
            sData += "\n    m_vtContour.clear();";
            sData += "\n    m_seriesuid = \"\";";
        }
        break;
    case TYPE_DOSEGRID:
        {
            sData += "\n    memset(m_grid_to_pat_t, 0, sizeof(float)*16);";
        }
        break;
    case TYPE_SERIES:
        {
            sData += "\n    m_image3d = new RtImage3D();";
        }
        break;
    //case TYPE_COMMISSIONEDUNIT:
    //    {
    //        sData += "\n    m_sc = nullptr;";
    //        sData += "\n    m_sar = nullptr;";
    //        sData += "\n    m_flatness = nullptr;";
    //        sData += "\n    m_offaxisdatax = nullptr;";
    //        sData += "\n    m_offaxisdatay = nullptr;";
    //        sData += "\n    m_mudatax = nullptr;";
    //        sData += "\n    m_mudatay = nullptr;";
    //        sData += "\n    m_dmudatay = nullptr;";
    //        sData += "\n    m_outputmuy = nullptr;";
    //        sData += "\n    m_ddmudr = nullptr;";
    //    }
    //    break;
    default:
        break;
    }

    sData += "\n}";

    //////////////////////////////////////////////////////////////////////////
    //RtBlock::RtBlock(const RtBlock& block):
    sData += "\n";
    sData += "\n" + sClassNameImp +"::" +sClassNameImp + "(const " + sClassNameImp +"& " + shortName +"):";
    iCount = 0;
    for (auto itr=table.vColumnItems.cbegin(); itr!=table.vColumnItems.cend(); ++itr, ++iCount)
    {
       if(shortName=="voi"){
            if ((*itr).sColumnName == "red"
                ||(*itr).sColumnName == "green"
                ||(*itr).sColumnName == "blue"
                ||(*itr).sColumnName == "alpha"
                ||(*itr).sColumnName == "pat2volumematrix"
                ||(*itr).sColumnName == "interpolate"
                ){
                    continue;
            }
        }
        else if (shortName == "beamsegment"){
            if ((*itr).sColumnName == "t_beam_to_pat"){
                continue;
            }
        }

        switch(table.enTableType)
        {
        case TYPE_MLCSHAPE:
            {
                if ((*itr).sColumnName == "leafpositions") {
                    continue;
                }
            }
            break;
        case TYPE_RTIMAGE:
            {
                if ((*itr).sColumnName == "pixeldata") {
                    continue;
                }
            }
            break;
        case TYPE_CONTOUR:
            {
                if ((*itr).sColumnName == "points"){
                    continue;
                }
            }
            break;
        case TYPE_CT2DENSITY:
            {
                if ((*itr).sColumnName == "ct2densityvalue"){
                    continue;
                }
            }
            break;
        case TYPE_DOSEGRID:
            {
                if ((*itr).sColumnName == "dosegridvalue"){
                    sData += "\n    m_dosegridvalue(nullptr),";
                    continue;
                }
                else if ((*itr).sColumnName == "grid_to_pat_t"){
                    continue;
                }
            }
            break;
        default:
            break;
        }

        std::string sLast = iCount == table.vColumnItems.size() -1 ? ")" : "),";
        std::string sName = "\n    m_" +(*itr).sColumnName +"(" + shortName +".get_" + (*itr).sColumnName + "()"; 
        sData += sName;
        sData += sLast;
    }

    sData += "\n{";

    switch(table.enTableType)
    {
    case TYPE_CT2DENSITY:
        {
            sData += "\n    m_ct2densityMap = ct2density.get_ct2densitymap();";
            sData += "\n    if(nullptr != ct2density.get_ct2densitybuffer()) ";
            sData += "\n        memcpy(m_ct2densityBuffer, ct2density.get_ct2densitybuffer(), sizeof(float) * 6025);";
        }
        break;
    case TYPE_BEAM:
        {
            sData += "\n    const std::vector<RtBeamsegment*> vseg = beam.get_beamsegments();";
            sData += "\n    for (auto itr=vseg.begin(); itr!=vseg.end(); ++itr){";
            sData += "\n        RtBeamsegment* pSeg = new RtBeamsegment(*(*itr));";
            sData += "\n        m_beamsegments.push_back(pSeg);";
            sData += "\n    }";
            sData += "\n";
            sData += "\n    m_aperture_block = nullptr;";
            sData += "\n    if (nullptr != beam.get_aperture_block()) m_aperture_block = new RtBlock(*beam.get_aperture_block());";
            sData += "\n    m_shield_block = nullptr;";
            sData += "\n    if (nullptr != beam.get_shield_block()) m_shield_block = new RtBlock(*beam.get_shield_block());";
            sData += "\n    m_dosegrid = new RtDosegrid(*beam.get_dosegrid());";
        }
        break;
    case TYPE_BEAMSEGMENT:
        {
            sData += "\n    this->set_t_beam_to_pat(beamsegment.get_t_beam_to_pat());";
            sData += "\n    this->m_beam_outline = new RtContour(*beamsegment.get_beamoutline());";
            sData += "\n    this->m_start_mlcshape = new RtMlcshape(*beamsegment.get_startmlcshape());";
            sData += "\n    this->m_end_mlcshape = new RtMlcshape(*beamsegment.get_endmlcshape());";
        }
        break;
    case TYPE_BLOCK:
        {
            sData += "\n    this->m_points = block.get_points();";
        }
        break;
    case TYPE_COMMISSIONEDUNIT:
        {
            sData += "\n    m_map_trayfactors = commissionedunit.get_discrete_trayfactor();";
        }
        break;
    case TYPE_CONTOUR:
        {
            sData += "\n    m_contour_points = contour.get_contour_points();";
        }
        break;
    case TYPE_DOSEGRID:
        {
            sData += "\n    this->set_grid_to_pat_t(dosegrid.get_grid_to_pat_t());";
            sData += "\n    if (nullptr != dosegrid.get_dosegrid_buffer()){";
            sData += "\n        const int iSize = m_xcount * m_ycount * m_zcount;";
            sData += "\n        if (iSize > 0 ){";
            sData += "\n            this->create_dosegrid_buffer();";
            sData += "\n            memcpy(m_dosegridvalue, dosegrid.get_dosegrid_buffer(),sizeof(float) * iSize);";
            sData += "\n        }";
            sData += "\n    }";
        }
        break;
    case TYPE_MLCSHAPE:
        {
            sData += "\n    m_leafpositions = mlcshape.get_leafpositions();";
        }
        break;
    case TYPE_RTIMAGE:
        {
            sData += "\n    unsigned long len;";
            sData += "\n    char* buffer = rtimage.get_pixel_data_buffer(&len);";
            sData += "\n    m_pixeldata = nullptr;";
            sData += "\n    if (len > 0 && nullptr != buffer){";
            sData += "\n        m_pixeldata = new char[len];";
            sData += "\n        memcpy(m_pixeldata, buffer, len);";
            sData += "\n    }";
        }
        break;
    case TYPE_SERIES:
        {
            sData += "\n    m_image3d = new RtImage3D(*series.get_image3d());";
        }
        break;
    case TYPE_IMAGE:
        {
            sData += "\n    char* data = image.get_dcmfiledata(&m_size);";
            sData += "\n    if (m_size > 0 && nullptr != data){";
            sData += "\n        m_dcmfiledata = new char[m_size];";
            sData += "\n        memcpy(m_dcmfiledata, data, m_size * sizeof(char));";
            sData += "\n    }";
        }
        break;
    case TYPE_PLAN:
        {
            sData += "\n    m_dosegrid = new RtDosegrid(*plan.get_dosegrid());";
        }
        break;
    case TYPE_NORMGROUP:
        {
            sData += "\n    m_dosegrid = new RtDosegrid(*normgroup.get_dosegrid());";
        }
        break;
    case TYPE_VOI:
        {
            sData += "\n    this->set_color(voi.get_color());";
            sData += "\n    this->set_pat2volumematrix(voi.get_pat2volumematrix());";
            sData += "\n    this->set_interpolate(voi.get_interpolate());";
            sData += "\n    this->set_seriesuid(voi.get_seriesuid());";
            sData += "\n    std::vector<RtContour*> cons = voi.get_contours();";
            sData += "\n    std::vector<RtContour*> cons_new;";
            sData += "\n    for (auto itr=cons.cbegin(); itr!=cons.cend(); ++itr) cons_new.push_back(new RtContour(*(*itr)));";
            sData += "\n    this->set_contours(cons_new);";
        }
        break;
    default:
        break;
    }

    sData += "\n}";

    //////////////////////////////////////////////////////////////////////////
    //RtBlock& RtBlock::operator=(const RtBlock& block)
    sData += "\n";
    sData += "\n" +sClassNameImp +"& " + sClassNameImp +"::operator=(const " + sClassNameImp +"& " + shortName +")";
    sData += "\n{";
    sData += "\n    if (this != &" + shortName +"){";
    for (auto itr=table.vColumnItems.cbegin(); itr!=table.vColumnItems.cend(); ++itr, ++iCount){

        if(shortName=="voi"){
            if ((*itr).sColumnName == "red"
                ||(*itr).sColumnName == "green"
                ||(*itr).sColumnName == "blue"
                ||(*itr).sColumnName == "alpha"

                ){
                    continue;
            }
            else if((*itr).sColumnName == "pat2volumematrix"){
                sData += "\n        this->set_pat2volumematrix(voi.get_pat2volumematrix());";
                continue;
            }
            else if((*itr).sColumnName == "interpolate"){
                sData += "\n        this->set_interpolate(voi.get_interpolate());";
                continue;
            }
        }  
        else if (shortName == "beamsegment"){
            if ((*itr).sColumnName == "t_beam_to_pat"){
                //sData += "\n        this->set_t_beam_to_pat(beamsegment.get_t_beam_to_pat());";
                continue;
            }
        }

        switch (table.enTableType)
        {
        case TYPE_RTIMAGE:
            {
                if ((*itr).sColumnName == "pixeldata") {
                    continue;
                }
            }
            break;
        case TYPE_CONTOUR:
            {
                if ((*itr).sColumnName == "points"){
                    continue;
                }
            }
            break;
        case TYPE_CT2DENSITY:
            {
                if ((*itr).sColumnName == "ct2densityvalue"){
                    continue;
                }
            }
            break;
        case TYPE_DOSEGRID:
            {
                if ((*itr).sColumnName == "dosegridvalue"){
                    sData += "\n        this->m_dosegridvalue = nullptr;";
                    continue;
                }
                else if ((*itr).sColumnName == "grid_to_pat_t"){
                    sData += "\n        this->set_grid_to_pat_t(dosegrid.get_grid_to_pat_t());";
                    continue;
                }
            }
            break;
        default:
            break;
        }

        std::string sName = "\n        this->m_" +(*itr).sColumnName +" = " + shortName +".get_" + (*itr).sColumnName + "();"; 
        sData += sName;
    }

    switch(table.enTableType)
    {
    case TYPE_CT2DENSITY:
        {
            sData += "\n        m_ct2densityMap = ct2density.get_ct2densitymap();";
            sData += "\n        if(nullptr != ct2density.get_ct2densitybuffer()) ";
            sData += "\n            memcpy(m_ct2densityBuffer, ct2density.get_ct2densitybuffer(), sizeof(float) * 6025);";
        }
        break;
    case TYPE_BEAM:
        {
            sData += "\n        const std::vector<RtBeamsegment*> vseg = beam.get_beamsegments();";
            sData += "\n        for (auto itr=vseg.begin(); itr!=vseg.end(); ++itr){";
            sData += "\n            RtBeamsegment* pSeg = new RtBeamsegment(*(*itr));";
            sData += "\n            m_beamsegments.push_back(pSeg);";
            sData += "\n        }";
            sData += "\n";
            sData += "\n        m_aperture_block = nullptr;";
            sData += "\n        if (nullptr != beam.get_aperture_block()) m_aperture_block = new RtBlock(*beam.get_aperture_block());";
            sData += "\n        m_shield_block = nullptr;";
            sData += "\n        if (nullptr != beam.get_shield_block()) m_shield_block = new RtBlock(*beam.get_shield_block());";
            sData += "\n        m_dosegrid = new RtDosegrid(*beam.get_dosegrid());";
        }
        break;
    case TYPE_NORMGROUP:
        {
            sData += "\n        this->m_dosegrid = new RtDosegrid(*normgroup.get_dosegrid());";
        }
        break;
    case TYPE_PLAN:
        {
            sData += "\n        this->m_dosegrid = new RtDosegrid(*plan.get_dosegrid());";
        }
        break;
    case TYPE_BEAMSEGMENT:
        {
            sData += "\n        this->set_t_beam_to_pat(beamsegment.get_t_beam_to_pat());";
            sData += "\n        *this->m_beam_outline = *beamsegment.get_beamoutline();";
            sData += "\n        *this->m_start_mlcshape = *beamsegment.get_startmlcshape();";
            sData += "\n        *this->m_end_mlcshape = *beamsegment.get_endmlcshape();";
        }
        break;
    case TYPE_BLOCK:
        {
            sData += "\n        this->m_points = block.get_points();";
        }
        break;
    case TYPE_COMMISSIONEDUNIT:
        {
            sData += "\n        this->m_map_trayfactors = commissionedunit.get_discrete_trayfactor();";
        }
        break;
    case TYPE_CONTOUR:
        {
            sData += "\n        this->m_contour_points = contour.get_contour_points();";
        }
        break;
    case TYPE_DOSEGRID:
        {
            sData += "\n        if (nullptr != dosegrid.get_dosegrid_buffer()){";
            sData += "\n            const int iSize = m_xcount * m_ycount * m_zcount;";
            sData += "\n            if (iSize > 0 ){";
            sData += "\n                this->create_dosegrid_buffer();";
            sData += "\n                memcpy(this->m_dosegridvalue, dosegrid.get_dosegrid_buffer(),sizeof(float) * iSize);";
            sData += "\n            }";
            sData += "\n        }";
        }
        break;
    case TYPE_RTIMAGE:
        {
            sData += "\n        unsigned long len;";
            sData += "\n        char* buffer = rtimage.get_pixel_data_buffer(&len);";
            sData += "\n        m_pixeldata = nullptr;";
            sData += "\n        if (len > 0 && nullptr != buffer){";
            sData += "\n            m_pixeldata = new char[len];";
            sData += "\n            memcpy(m_pixeldata, buffer, len);";
            sData += "\n        }";
        }
        break;
    case TYPE_SERIES:
        {
            sData += "\n        *this->m_image3d = *series.get_image3d();";
        }
        break;
    case TYPE_IMAGE:
        {
            sData += "\n        char* data = image.get_dcmfiledata(&this->m_size);";
            sData += "\n        if (this->m_size > 0 && nullptr != data){";
            sData += "\n            this->m_dcmfiledata = new char[m_size];";
            sData += "\n            memcpy(this->m_dcmfiledata, data, m_size * sizeof(char));";
            sData += "\n        }";
        }
        break;
    case TYPE_VOI:
        {
            sData += "\n        this->set_color(voi.get_color());";
            sData += "\n        this->set_seriesuid(voi.get_seriesuid());";
            sData += "\n        std::vector<RtContour*> cons = voi.get_contours();";
            sData += "\n        std::vector<RtContour*> cons_new;";
            sData += "\n        for (auto itr=cons.cbegin(); itr!=cons.cend(); ++itr) cons_new.push_back(new RtContour(*(*itr)));";
            sData += "\n        this->set_contours(cons_new);";
        }
        break;
    default:
        break;
    }

    sData += "\n    }";
    sData += "\n    return *this;";
    sData += "\n}";

    //////////////////////////////////////////////////////////////////////////
    sData += "\n";
    sData += "\n" + sClassNameImp + "::~" + sClassNameImp + "()";
    sData += "\n{";

    if(shortName=="series"){
        sData += "\n    DEL_PTR(m_image3d);";
    }
    else if (shortName == "normgroup"){
        sData += "\n    DEL_PTR(m_dosegrid);";
    }
    else if (shortName == "plan"){
        sData += "\n    DEL_PTR(m_dosegrid);";
    }
    else if (shortName == "beamsegment"){
        sData += "\n    DEL_PTR(m_beam_outline);";
        sData += "\n    DEL_PTR(m_start_mlcshape);";
        sData += "\n    DEL_PTR(m_end_mlcshape);";
    }

    switch(table.enTableType)
    {
    case TYPE_BEAM:
        {
           sData += "\n    //for (auto itr=m_beamsegments.begin(); itr!=m_beamsegments.end(); ++itr)";
           sData += "\n    //    DEL_PTR(*itr);";
           sData += "\n    m_beamsegments.clear();";
           sData += "\n";
           sData += "\n    DEL_PTR(m_aperture_block);";
           sData += "\n    DEL_PTR(m_shield_block);";
           sData += "\n    DEL_PTR(m_dosegrid);";
        }
        break;
    case TYPE_RTIMAGE:
        {
            sData += "\n    DEL_ARRAY(m_pixeldata);";
        }
        break;
    case TYPE_VOI:
        {
            sData += "\n    for (auto itr=m_vtContour.begin(); itr!=m_vtContour.end(); ++itr) DEL_PTR(*itr);";
            sData += "\n    m_vtContour.clear();";
        }
        break;
    case TYPE_DOSEGRID:
        {
            sData += "\n    DEL_ARRAY(m_dosegridvalue);";
        }
        break;
    case TYPE_IMAGE:
        {
            sData += "\n    DEL_ARRAY(m_dcmfiledata);";
        }
        break;
    default:
        break;
    }

    sData += "\n}";

    sData += "\n";

    //////////////////////////////////////////////////////////////////////////
    switch(table.enTableType)
    {
    case TYPE_BEAM:
        {
            sData += "\n";
            sData += "\nRtDosegrid* RtBeamImp::get_dosegrid()";
            sData += "\n{";
            sData += "\n    return m_dosegrid;";
            sData += "\n}";
            sData += "\n";
            sData += "\nRtDosegrid* RtBeamImp::get_dosegrid() const";
            sData += "\n{";
            sData += "\n    return m_dosegrid;";
            sData += "\n}";
            sData += "\n";
            sData += "\nvoid RtBeamImp::create_aperture_block()";
            sData += "\n{";
            sData += "\n    DEL_PTR(m_aperture_block);";
            sData += "\n    m_aperture_block =  new RtBlock(true);";
            sData += "\n    m_aperture_block->set_blocktype(RtDbDef::APERTURE);";
            sData += "\n}";
            sData += "\nRtBlock* RtBeamImp::get_aperture_block() const";
            sData += "\n{";
            sData += "\n    return m_aperture_block;";
            sData += "\n}";
            sData += "\nvoid RtBeamImp::remove_aperture_block()";
            sData += "\n{";
            sData += "\n    DEL_PTR(m_aperture_block);";
            sData += "\n}";
            sData += "\n";
            sData += "\nvoid RtBeamImp::create_shield_block()";
            sData += "\n{";
            sData += "\n    DEL_PTR(m_shield_block);";
            sData += "\n    m_shield_block =  new RtBlock(true);";
            sData += "\n    m_shield_block->set_blocktype(RtDbDef::SHIELDING);";
            sData += "\n}";
            sData += "\nRtBlock* RtBeamImp::get_shield_block() const";
            sData += "\n{";
            sData += "\n    return m_shield_block;";
            sData += "\n}";
            sData += "\nvoid RtBeamImp::remove_shield_block()";
            sData += "\n{";
            sData += "\n    DEL_PTR(m_shield_block);";
            sData += "\n}";
        }
        break;
    case TYPE_NORMGROUP:
        {
            sData += "\nRtDosegrid* RtNormgroupImp::get_dosegrid() const {return m_dosegrid;}";
        }
        break;
    case TYPE_RTIMAGE:
        {
            sData += "\n//////////////////////////////////////////////////////////////////////////";
            sData += "\nvoid RtRtimageImp::set_pixel_data_buffer(char* pBuffer, unsigned long lLen)";
            sData += "\n{";
            sData += "\n    if (lLen > 0)";
            sData += "\n    {";
            sData += "\n        DEL_ARRAY(m_pixeldata);";
            sData += "\n        m_pixeldata = new char[lLen];";
            sData += "\n        memcpy(m_pixeldata, pBuffer, lLen*sizeof(char));";
            sData += "\n    }";
            sData += "\n}";
            sData += "\n";
            sData += "\nchar* RtRtimageImp::get_pixel_data_buffer(unsigned long* lLen) const";
            sData += "\n{";
            sData += "\n    *lLen = m_rows * m_columns * m_frame * m_bitsallocated/8 * m_samplesperpixel;";
            sData += "\n    return m_pixeldata;";
            sData += "\n}";
        }
        break;
    default:
        break;
    }
}
/*
void CodeGeneraterMysql::rt_tps_database_object_helper_h(TableItem& table)
{
    //write rt_tps_database_object_helper_xxx.h
    std::string shortName = table.sTableName.substr(3,table.sTableName.length());
    std::string sFileName="rt_tps_database_object_helper_" + shortName + ".h";
    const std::string sFileNameOutput="output//" + sFileName;
    //file_rt_tps_database_interface_object_h.open(sFileNameOutput);

    std::string sData("");
    sData += "//////////////////////////////////////////////////////////////////////////";
    sData += "\n/// \\defgroup Radio Therapy Business Unit";
    sData += "\n///  Copyright, (c) Shanghai United Imaging Healthcare Inc., 2016";
    sData += "\n///  All rights reserved.";
    sData += "\n///";
    sData += "\n///  \\author  ZHOU qiangqiang  mailto:qiangqiang.zhou@united-imaging.com";
    sData += "\n///";
    sData += "\n///  \\file      " + sFileName;
    sData += "\n///  \\brief     This file was generated by CodeGenerater.exe ";
    sData += "\n///";
    sData += "\n///  \\version 1.0";
    sData += "\n///  \\date    " + m_sCurrentDate;
    sData += "\n///  \\{";
    sData += "\n//////////////////////////////////////////////////////////////////////////";

    std::string sTableNameUpper = shortName;
    transform(shortName.begin(), shortName.end(), sTableNameUpper.begin(), toupper);
    std::string sFirstUpper = shortName.substr(0,1);
    transform(sFirstUpper.begin(), sFirstUpper.end(), sFirstUpper.begin(), toupper);
    std::string sTableNameWithFirst = sFirstUpper + shortName.substr(1,shortName.length() - 1);

    const std::string sClassName = "Rt" + sTableNameWithFirst;

    sData += "\n";
    sData += "\n#ifndef RT_TPS_DATABASE_OBJECT_HELPER_" + sTableNameUpper + "_H_";
    sData += "\n#define RT_TPS_DATABASE_OBJECT_HELPER_" + sTableNameUpper + "_H_";

    sData += "\n";
    sData += "\n#include \"RtTpsDatabaseWrapper/rt_tps_database_defs.h\"";

    sData += "\n";
    sData += "\nstruct st_mysql_stmt;";

    sData += "\n";
    sData += "\nRT_TPS_DATABASE_BEGIN_NAMESPACE;";

    sData += "\n";
    sData += "\nclass " + sClassName +";";
    sData += "\nclass Rt" + sTableNameWithFirst + "HelperApi";
    sData += "\n{";
    sData += "\npublic:";
    
    //this is old version unsupported blob
    //sData += "\n";
    //sData += "\n    static void CopyObjectFromTable(const MYSQL_ROW& row, unsigned int iNumFields, " + sClassName + "* pDis);";
    //sData += "\n";
    //sData += "\n    static void InsertTableFromObject(const " + sClassName + "* pSrc, std::string& sSql);";

    sData += "\n";
    sData += "\n    static bool InsertUpdateDBFromObject(st_mysql_stmt* stmt, const " + sClassName + "& src, bool bInsert);";

    sData += "\n";
    sData += "\n    static bool FetchDataFromDB(const std::string& sSql, st_mysql_stmt* stmt, std::vector<" + sClassName + "*>& vList);";

    //////////////////////////////////////////////////////////////////////////
    sData += "\n};";
    sData += "\n";
    sData += "\nRT_TPS_DATABASE_END_NAMESPACE";
    sData += "\n#endif";

    WriteFileInUTF8(sFileNameOutput, sData);
}
*/

void CodeGeneraterMysql::rt_tps_database_object_helper_h()
{
    //write rt_tps_database_object_helper.h
    std::string sFileName="rt_tps_database_object_helper.h";
    const std::string sFileNameOutput="output//" + sFileName;

    std::string sData("");
    sData += "//////////////////////////////////////////////////////////////////////////";
    sData += "\n/// \\defgroup Radio Therapy Business Unit";
    sData += "\n///  Copyright, (c) Shanghai United Imaging Healthcare Inc., 2016";
    sData += "\n///  All rights reserved.";
    sData += "\n///";
    sData += "\n///  \\author  ZHOU qiangqiang  mailto:qiangqiang.zhou@united-imaging.com";
    sData += "\n///";
    sData += "\n///  \\file      " + sFileName;
    sData += "\n///  \\brief     This file was generated by CodeGenerater.exe ";
    sData += "\n///              From database version: " + m_DatabaseVersion;
    sData += "\n///";
    sData += "\n///  \\version 1.0";
    sData += "\n///  \\date    " + m_sCurrentDate;
    sData += "\n///  \\{";
    sData += "\n//////////////////////////////////////////////////////////////////////////";

    sData += "\n";
    sData += "\n#ifndef RT_TPS_DATABASE_OBJECT_HELPER_H_";
    sData += "\n#define RT_TPS_DATABASE_OBJECT_HELPER_H_";

    sData += "\n";
    sData += "\n#include \"RtTpsDatabaseWrapper/rt_tps_database_defs.h\"";
    sData += "\n#include \"RtTpsDatabaseWrapper/rt_tps_database_common_enums.h\"";
    sData += "\n#include \"boost/date_time/gregorian/gregorian_types.hpp\"";
    sData += "\n#include \"boost/date_time/posix_time/posix_time_types.hpp\"";

    sData += "\n";
    sData += "\nRT_TPS_DATABASE_BEGIN_NAMESPACE;";

    sData += "\n";
    sData += "\n#define START_TRANSACTION \"START TRANSACTION\"";
    sData += "\n#define SELECT_FROM \"SELECT * FROM \"";
    sData += "\n#define GET_DBVERSION \"select version from tmsdbversion limit 1\"";

    sData += "\n";
    for (auto itr=vTables.begin(); itr!=vTables.end(); ++itr){
        TableItem table = (*itr);
        sData += "\n#define TABLE_" + table.sShortNameUpper +" \" " + table.sTableName + " \" ";
    }

    //#define SELECT_TABLE_
    sData += "\n";
    for (auto itr=vTables.begin(); itr!=vTables.end(); ++itr){
        TableItem table = (*itr);
        sData += "\n#define SELECT_TABLE_" + table.sShortNameUpper +" \"SELECT * FROM " + table.sTableName + " \" ";
    }

    //typedef boost::posix_time::ptime DATETIME_BOOST;
    sData += "\n";
    sData += "\ntypedef boost::posix_time::ptime DATETIME_BOOST;";
    sData += "\ntypedef boost::posix_time::time_duration TIME_BOOST;";
    sData += "\ntypedef boost::gregorian::date DATE_BOOST;";

    for (auto itr=vTables.begin(); itr!=vTables.end(); ++itr){
        TableItem table = (*itr);
        sData += "\nclass " + table.sClassName +";";
    }

    //class RtDatabaseHelper
    sData += "\n";
    sData += "\nclass RtDatabaseHelper";
    sData += "\n{";
    sData += "\npublic:";

    sData += "\n    RtDatabaseHelper(MYSQL* pMySql);";
    sData += "\n    ~RtDatabaseHelper();";
    sData += "\n";
    sData += "\n    //common";
    sData += "\n    bool QueryDBInTable(const std::string& sUid, const std::string& sTable) const;";
    sData += "\n    bool QueryDBInTable(const std::string& sUid, RtDbDef::OBJECT_TYPE objType) const;";
    sData += "\n    bool QueryDBInTable(const std::string& sFirstUid, const std::string& sSecondUid, ";
    sData += "\n        RtDbDef::OBJECT_TYPE objType) const;";
    sData += "\n    bool QueryDBInPatient(const std::string& sPatientUID) const;";
    sData += "\n    bool MysqlRealQuery(const std::string& sSQL) const;";
    sData += "\n    std::string ConvertDateTime(const DATETIME_BOOST& datetime);";
    sData += "\n    std::string ConvertDateTime(const DATE_BOOST& date);";
    sData += "\n    std::string ConvertDateTime(const TIME_BOOST& time);";
    sData += "\n    void ConvertDateTime2DB(const DATETIME_BOOST& datetime, MYSQL_TIME* my_datetime);";
    sData += "\n    void ConvertDateTime2DB(const DATE_BOOST& date, MYSQL_TIME* my_datetime);";
    sData += "\n    void ConvertDateTime2DB(const TIME_BOOST& time, MYSQL_TIME* my_datetime);";
    sData += "\n";
    sData += "\n    void ConvertDateTime2Boost(const MYSQL_TIME& my_datetime, DATETIME_BOOST* datetime);";
    sData += "\n    void ConvertDateTime2Boost(const MYSQL_TIME& my_datetime, DATE_BOOST* datetime);";
    sData += "\n    void ConvertDateTime2Boost(const MYSQL_TIME& my_datetime, TIME_BOOST* datetime);";

    //RtSeries
    for (auto itr=vTables.begin(); itr!=vTables.end(); ++itr){
        TableItem table = (*itr);
        sData += "\n";
        sData += "\n    //" + table.sClassName ;
        sData += "\n    bool InsertUpdateDBFromObject(const " + table.sClassName +"& src, bool bInsert);";
        sData += "\n    bool FetchDataFromDB(const std::string& sSql, std::vector<" + table.sClassName +"*>& vList);";
    }

    sData += "\n";
    sData += "\nprivate:";
    sData += "\n    MYSQL*  m_pMySql;";
    //////////////////////////////////////////////////////////////////////////
    sData += "\n};";
    sData += "\n";
    sData += "\nRT_TPS_DATABASE_END_NAMESPACE";
    sData += "\n#endif";

    WriteFileInUTF8(sFileNameOutput, sData);
}

void CodeGeneraterMysql::WriteCommonInfo()
{
    //write rt_tps_database_common_info.h
    std::string sFileName="rt_tps_database_common_info.h";
    const std::string sFileNameOutput="output//" + sFileName;
    std::string sData("");
    sData += "//////////////////////////////////////////////////////////////////////////";
    sData += "\n/// \\defgroup Radio Therapy Business Unit";
    sData += "\n///  Copyright, (c) Shanghai United Imaging Healthcare Inc., 2016";
    sData += "\n///  All rights reserved.";
    sData += "\n///";
    sData += "\n///  \\author  ZHOU qiangqiang  mailto:qiangqiang.zhou@united-imaging.com";
    sData += "\n///";
    sData += "\n///  \\file      " + sFileName;
    sData += "\n///  \\brief     This file was generated by CodeGenerater.exe ";
    sData += "\n///";
    sData += "\n///  \\version 1.0";
    sData += "\n///  \\date    " + m_sCurrentDate;
    sData += "\n///  \\{";
    sData += "\n//////////////////////////////////////////////////////////////////////////";

    sData += "\n";
    sData += "\n#ifndef RT_TPS_DATABASE_OBJECT_HELPER_H_";
    sData += "\n#define RT_TPS_DATABASE_OBJECT_HELPER_H_";

    sData += "\n";
    sData += "\nRT_TPS_DATABASE_BEGIN_NAMESPACE;";

     //OBJECT_TYPE
    sData += "\n    typedef enum";
    sData += "\n    {";
    int iCount(0);
    for (auto itr=vTables.cbegin(); itr!=vTables.cend(); ++itr,++iCount){
        sData += "\n    TYPE_" + (*itr).sShortNameUpper + (0==iCount ? " = 0," : ",");
    }
    sData += "\n    }OBJECT_TYPE;";

    //ENUM_TO_STRING_OBJECT_TYPE
    sData += "\n";
    sData += "\nstatic const char * ENUM_TO_STRING_OBJECT_TYPE[] = ";
    sData += "\n{";
    for (auto itr=vTables.cbegin(); itr!=vTables.cend(); ++itr){
        sData += "\n    \"" + (*itr).sShortNameUpper + "\",";
    }
    sData += "\n};";

    //////////////////////////////////////////////////////////////////////////
    sData += "\n};";
    sData += "\n";
    sData += "\nRT_TPS_DATABASE_END_NAMESPACE";
    sData += "\n#endif";
    WriteFileInUTF8(sFileNameOutput, sData);
}

void CodeGeneraterMysql::WriteFileInUTF8(const std::string& sFileName, const std::string& sData, bool bAdd)
{
    FILE* pFile;

    std::string sType = bAdd ? "a+" : "w+";
    errno_t err=fopen_s(&pFile,sFileName.c_str(),sType.c_str());
    if(err==0){
        //printf("The file was opened\n");
    }
    else{
        //printf("The file was not opened\n");
    }

    if (!bAdd){
        const unsigned char szBOM[4]={0xEF,0xBB,0xBF,0}; //(char)
        fprintf(pFile,"%s",szBOM);
    }

    std::wstring wOuttext = MBytesToWString(sData.c_str());
    std::string outtext = WStringToUTF8(wOuttext.c_str());
    fprintf(pFile,"%s",outtext.c_str());
    fclose(pFile);
}

void CodeGeneraterMysql::WriteFileInUTF8Added(const std::string& sFileName, const std::wstring& wData)
{
    FILE* pFile;
    errno_t err=fopen_s(&pFile,sFileName.c_str(),"a");
    if(err==0){
        //printf("The file was opened\n");
    }
    else{
        //printf("The file was not opened\n");
    }

    //const unsigned char szBOM[4]={0xEF,0xBB,0xBF,0}; //(char)
    //fprintf(pFile,"%s",szBOM);

  //  std::wstring wOuttext = MBytesToWString(sData.c_str());
    std::string outtext = WStringToUTF8(wData.c_str());
    fprintf(pFile,"%s",outtext.c_str());
    fclose(pFile);
}

//////////////////////////////////////////////////////////////////////////
//core here for insert,update,retrieve 
//mysql_stmt_prepare(), INSERT failed,Column count doesn't match value count at row 1
//solution:
////char* insertSQL="INSERT INTO tmsblobtest VALUES(?, ?)";
//char* insertSQL="INSERT INTO tmsblobtest (uid,type) VALUES(?, ?)";

void CodeGeneraterMysql::rt_tps_database_object_helper_cpp(TableItem& table)
{
    //write rt_tps_database_object_helper_xxx.cpp
    //std::ofstream file;
    std::string shortName = table.sTableName.substr(3,table.sTableName.length());
    const std::string sFileName="rt_tps_database_object_helper_" + shortName + ".cpp";
    const std::string sFileNameOutput="output//" + sFileName;
    //file.open(sFileNameOutput);

    std::string sData("");
    sData += "//////////////////////////////////////////////////////////////////////////";
    sData += "\n/// \\defgroup Radio Therapy Business Unit";
    sData += "\n///  Copyright, (c) Shanghai United Imaging Healthcare Inc., 2016";
    sData += "\n///  All rights reserved.";
    sData += "\n///";
    sData += "\n///  \\author  ZHOU qiangqiang  mailto:qiangqiang.zhou@united-imaging.com";
    sData += "\n///";
    sData += "\n///  \\file      " + sFileName;
    sData += "\n///  \\brief     This file was generated by CodeGenerater.exe ";
    sData += "\n///             From database version: " + m_DatabaseVersion;
    sData += "\n///";
    sData += "\n///  \\version 1.0";
    sData += "\n///  \\date    " + m_sCurrentDate;
    sData += "\n///  \\{";
    sData += "\n//////////////////////////////////////////////////////////////////////////";

    std::string sTableNameUpper = shortName;
    transform(shortName.begin(), shortName.end(), sTableNameUpper.begin(), toupper);
    std::string sFirstUpper = shortName.substr(0,1);
    transform(sFirstUpper.begin(), sFirstUpper.end(), sFirstUpper.begin(), toupper);
    std::string sTableNameWithFirst = sFirstUpper + shortName.substr(1,shortName.length() - 1);

    const std::string sClassName = "Rt" + sTableNameWithFirst;

    std::stringstream ss;
    std::string strNum;
    ss<<table.vColumnItems.size();
    ss>>strNum;

    sData += "\n";
    sData += "\n#include \"StdAfx.h\"";
    sData += "\n#include \"rt_tps_database_object_helper.h\"";
    sData += "\n#include \"RtTpsDatabaseWrapper/rt_tps_database_interface_object_" + shortName + ".h\"";
    if (TYPE_MACHINE == table.enTableType)
    {
        sData += "\n#include \"rt_tps_database_cryptohash.h\"";
    }
    if(TYPE_MLCSHAPE == table.enTableType
        ||TYPE_DOSEGRID == table.enTableType
        || TYPE_BEAMSEGMENT == table.enTableType
        || TYPE_IMAGETRANSFORMATION == table.enTableType
        || TYPE_CONTOUR == table.enTableType
        || TYPE_COMMISSIONEDUNIT == table.enTableType
        || TYPE_VOI == table.enTableType
        || TYPE_MACHINE == table.enTableType
        || TYPE_BLOCK == table.enTableType
        || TYPE_CT2DENSITY == table.enTableType
        || TYPE_APPLICATOR == table.enTableType){
        sData += "\n#include \"boost/algorithm/string/split.hpp\"";
        sData += "\n#include \"boost/algorithm/string/classification.hpp\"";
       // sData += "\n#include \"boost/lexical_cast.hpp\"";
    }   
    if (TYPE_MACHINE == table.enTableType)
    {
        sData += "\n#include \"boost/math/special_functions/round.hpp\"";
    }

    if (table.bHasDateTime){
        sData += "\n#include \"boost/date_time/posix_time/time_parsers.hpp\"";
        sData += "\ntypedef boost::posix_time::ptime DATETIME_BOOST;";
        sData += "\ntypedef boost::posix_time::time_duration TIME_BOOST;";
        sData += "\ntypedef boost::gregorian::date DATE_BOOST;";
    }
                                                                         

    if (TYPE_APPLICATOR == table.enTableType)
    {
        sData += "\n#include \"RtTpsDatabaseWrapper/rt_tps_database_data.h\"";
    }

    sData += "\n";
    sData += "\nRT_TPS_DATABASE_BEGIN_NAMESPACE;";

    //////////////////////////////////////////////////////////////////////////
    //some local fuctions
    switch(table.enTableType)
    {
    case TYPE_CT2DENSITY:
        {
            sData += "\n";
            sData += "\nvoid SetupCt2DensityMap_i(const char* buf, RtCt2density* ct2densityMap)";
            sData += "\n{";
            sData += "\n    std::vector<std::string> vecValue;";
            sData += "\n    (void)boost::split(vecValue, buf, boost::is_any_of(\"|\"));";
            sData += "\n";
            sData += "\n    std::vector<int> vCT;";
            sData += "\n    std::map<int, float> refMap;//= ct2densityMap->GetCt2densitymap();";
            sData += "\n    for (std::vector<std::string>::iterator it = vecValue.begin();it != vecValue.end(); ++it){";
            sData += "\n        string strValue = *it;";
            sData += "\n        size_t iPos = strValue.find(',');";
            sData += "\n        std::string sKey = strValue.substr(0,iPos);";
            sData += "\n        int iCT = atoi(sKey.c_str());";
            sData += "\n";
            sData += "\n        vCT.push_back(iCT);";
            sData += "\n        float fDensity = static_cast<float>(atof(strValue.substr(iPos + 1,strValue.length() - iPos -1).c_str()));";
            sData += "\n        refMap.insert(std::make_pair(iCT,fDensity));";
            sData += "\n    }";
            sData += "\n";
            sData += "\n    ct2densityMap->set_ct2densitymap(refMap);";
            sData += "\n}";
        }
        break;
    default:
        break;
    }

    //////////////////////////////////////////////////////////////////////////
    //InsertUpdateDBFromObject start
    sData += "\n";
    sData += "\nbool RtDatabaseHelper::InsertUpdateDBFromObject(const " + sClassName + "& src, bool bInsert)";
    sData += "\n{";
    sData += "\n    MYSQL_STMT* stmt = mysql_stmt_init(m_pMySql);";
    sData += "\n    if (!stmt)";
    sData += "\n    {";
    sData += "\n        TPS_LOG_DEV_ERROR<<\"mysql_stmt_init(), out of memory\";";
    sData += "\n        return false;";
    sData += "\n    }";

    sData += "\n    //" + strNum + " parameters";
    sData += "\n    unsigned int field_num = 0;";
    //sData += "\n    unsigned int field_offset = 0;";
    //sData += "\n    std::string sSql = \"INSERT INTO " + table.sTableName + " VALUES(?";
    //for (int i(1); i<table.vColumnItems.size(); ++i)
    //{
    //     sData += ", ?";
    //     if (0 == i%5){
    //         sData += "\"";
    //         sData += "\n        \"";
    //     }
    //}
    //sData += ")\";";
    sData += "\n    std::string sSql(\"\");";
    sData += "\n    std::string sName(\"\");";

    //std::string sFkCount = std::to_string((long long)table.iFkCount);
    sData += "\n    if (!bInsert)";
    sData += "\n    {";

    //sData += "\n        field_num -= " +  sFkCount + ";";
    //sData += "\n        field_offset = " + sFkCount + ";";
    sData += "\n        for (unsigned int i(0); i < " + sTableNameUpper + "_FIELD_MAX; ++i)";

    if (table.vColumnItems.size() < 3){
        sData +=";";
    }
    //for (int i(table.iFkCount); i<table.vColumnItems.size(); ++i)
    //{//skip PK and FK
    //    const std::string sSuffix = (i==table.vColumnItems.size()-1) ? "\";" : ",\""; 
    //    sData += "\n            \"" + table.vColumnItems[i].sColumnName + "=?" + sSuffix;
    //}
    sData += "\n        {";
    sData += "\n            if (!src.is_dirty(i)) continue;";
    sData += "\n            const char* file_name = ENUM2STRING_" + sTableNameUpper +"_FIELD[i];";
    sData += "\n            if (sName.empty())";
    sData += "\n            {";
    sData += "\n                sName += std::string(file_name) + \"=?\";";
    sData += "\n            }";
    sData += "\n            else";
    sData += "\n            {";
    sData += "\n                sName += \",\" + std::string(file_name) + \"=?\";";
    sData += "\n            }";
    sData += "\n            ++field_num;";
    sData += "\n        }";
    
    if (table.bIsMap){//for map two keys
        if (table.vColumnItems.size() > 1)
        {
            std::string s1 = table.vColumnItems[0].sColumnName;
            std::string s2 = table.vColumnItems[1].sColumnName;
            sData += "\n        sSql = \"UPDATE tms" + shortName + " SET \" + sName + \" WHERE " + 
                s1 + "='\" + src.get_" + s1 + "() + \"' AND " + s2 + "='\" + src.get_" + s2  + "() + \"'\";";
        }
    }
    else{
        sData += "\n        sSql = \"UPDATE tms" + shortName + " SET \" + sName + \" WHERE uid='\" + src.get_uid() + \"'\";";
    }
    sData += "\n    }";

    //////////////////////////////////////////////////////////////////////////
    //else
    sData += "\n    else";
    sData += "\n    {";
    sData += "\n        std::string sValues(\"\");";
    sData += "\n        for (unsigned int i(0); i < " + sTableNameUpper +"_FIELD_MAX; ++i)";
    sData += "\n        {";
    sData += "\n            if (!src.is_dirty(i)) continue;";
    sData += "\n            const char* file_name = ENUM2STRING_" + sTableNameUpper +"_FIELD[i];";
    sData += "\n            if (sName.empty())";
    sData += "\n            {";
    sData += "\n                sName += std::string(file_name);";
    sData += "\n                sValues += \"?\";";
    sData += "\n            }";
    sData += "\n            else";
    sData += "\n            {";
    sData += "\n                sName += \",\" + std::string(file_name);";
    sData += "\n                sValues += \",?\";";
    sData += "\n            }";
    sData += "\n            ++field_num;";
    sData += "\n        }";
    sData += "\n        sSql = \"INSERT INTO tms" + shortName +  " (\" + sName + \") VALUES (\" + sValues + \")\";";
    sData += "\n    }";

    sData += "\n    if (mysql_stmt_prepare(stmt, sSql.c_str(), (unsigned long)sSql.size()))";
    sData += "\n    {";
    sData += "\n        TPS_LOG_DEV_ERROR<<\" mysql_stmt_prepare(), failed \"<< mysql_stmt_error(stmt);";
    sData += "\n        TPS_PRINTF_DEBUG(\"mysql_stmt_prepare(), failed %s\\n\", mysql_stmt_error(stmt));";
    sData += "\n        return false;";
    sData += "\n    }";

    //////////////////////////////////////////////////////////////////////////
    sData += "\n";
    sData += "\n    MYSQL_BIND* bind = new MYSQL_BIND[field_num];";
    sData += "\n    memset(bind, 0, sizeof(MYSQL_BIND)*field_num);";
    sData += "\n    my_bool     isnull = true;";
    {
        for (auto itr=table.vColumnItems.cbegin(); itr!=table.vColumnItems.cend(); ++itr){
            const std::string sType = (*itr).sCppDataType;
            switch((*itr).enDataType){
            case _int:
            case _bigint:
                {  
                    sData += "\n    " + sType +"\t\t\tdata_" + (*itr).sColumnName + " = 0;";
                }
                break;
            case _float:
                {  
                    sData += "\n    " + sType +"\t\tdata_" + (*itr).sColumnName + " = 0.f;";
                }
                break;
            case _double:
                {  
                    sData += "\n    " + sType +"\t\tdata_" + (*itr).sColumnName + " = 0.;";
                }
                break;
            case _bool:
            case _tinyint:
                {
                    sData += "\n    " + sType +"\t\tdata_" + (*itr).sColumnName + " = false;";
                }
                break;

            case _datetime:
            case _timestamp:
            case _date:
            case _time:
                {
                    sData += "\n    MYSQL_TIME\tdata_" + (*itr).sColumnName + ";";
                }
                break;
            case _blob:
            case _mediumblob:
            case _longblob:
                {
                    if ("char*" == (*itr).sCppDataType){
                        sData += "\n    " + sType +"\t\tdata_" + (*itr).sColumnName + " = nullptr;";
                    }
                    //else if ("std::string" == (*itr).sCppDataType){
                    //    sData += "\n    " + sType +"\tdata_" + (*itr).sColumnName + " = \"\";";
                    //}
                    else if ("discretedoserate" == (*itr).sColumnName
                        || ("leafboundaries" == (*itr).sColumnName && TYPE_MACHINE == table.enTableType)){
                        sData += "\n    std::string\tdata_" + (*itr).sColumnName + " = \"\";";
                    }
                    else if ("dosegridvalue" == (*itr).sColumnName){
                        sData += "\n    uint32_t*   data_dosegridvalue = nullptr;";
                    } 
                    else
                    {
                        sData += "\n    " + sType +"\tdata_" + (*itr).sColumnName + " = \"\";";
                    }
                }
                break;

            case _varchar:
            case _char:
            case _unknown:
            default:  
                sData += "\n    " + sType +"\tdata_" + (*itr).sColumnName + " = \"\";";
                break;
            }
        }
    }
    ////{
    //if(TYPE_DOSEGRID == table.enTableType
    //    ||TYPE_VOI == table.enTableType
    //    || TYPE_BEAMSEGMENT == table.enTableType
    //    || TYPE_IMAGETRANSFORMATION == table.enTableType){
    //    sData += "\n    std::string value_matrix(\"\");";
    //}
    //else if(TYPE_CONTOUR == table.enTableType){
    //    sData += "\n    std::string value_points(\"\");";
    //}
    ////}


    //////////////////////////////////////////////////////////////////////////
    sData += "\n";
    sData += "\n    unsigned int iCount = 0;";
    sData += "\n    for (unsigned int i(0); i < " + sTableNameUpper +"_FIELD_MAX; ++i)";
    sData += "\n    {";
    sData += "\n        if (!src.is_dirty(i)) continue;";
    sData += "\n        if(!src.has_field(i))";
    sData += "\n        {";
    sData += "\n            bind[iCount].is_null = &isnull;";
    sData += "\n            ++iCount;";
    sData += "\n            continue;";
    sData += "\n        }";
    sData += "\n        switch (i)";
    sData += "\n        {";

    for (auto itr=table.vColumnItems.cbegin(); itr!=table.vColumnItems.cend(); ++itr)
    {
        std::stringstream ssIndex;
        std::string sIndex;
        ssIndex<<( (*itr).iPosition -1 );//start from 0
        ssIndex>>sIndex;
        
        sData += "\n        case " + sIndex + ": \t//" + (*itr).sColumnName + " "+ std::string(Enum2StringDataType[(*itr).enDataType]);
        sData += "\n            {";

        if(table.enTableType == TYPE_VOI){
            if ((*itr).sColumnName == "red")
            {
                sData += "\n                data_red = src.get_color()[0];";
                sData += "\n                bind[iCount].buffer_type = MYSQL_TYPE_FLOAT;";
                sData += "\n                bind[iCount].buffer = (void*)&data_red;";
                sData += "\n                bind[iCount].buffer_length = sizeof(float);";
                sData += "\n            }";
                sData += "\n            break;";
                continue;
            }
            else if((*itr).sColumnName == "green"){
                sData += "\n                data_green = src.get_color()[1];";
                sData += "\n                bind[iCount].buffer_type = MYSQL_TYPE_FLOAT;";
                sData += "\n                bind[iCount].buffer = (void*)&data_green;";
                sData += "\n                bind[iCount].buffer_length = sizeof(float);";
                sData += "\n            }";
                sData += "\n            break;";
                continue;
            }
            else if((*itr).sColumnName == "blue"){
                sData += "\n                data_blue = src.get_color()[2];";
                sData += "\n                bind[iCount].buffer_type = MYSQL_TYPE_FLOAT;";
                sData += "\n                bind[iCount].buffer = (void*)&data_blue;";
                sData += "\n                bind[iCount].buffer_length = sizeof(float);";
                sData += "\n            }";
                sData += "\n            break;";
                continue;
            }
            else if((*itr).sColumnName == "alpha"){
                sData += "\n                data_alpha = src.get_color()[3];";
                sData += "\n                bind[iCount].buffer_type = MYSQL_TYPE_FLOAT;";
                sData += "\n                bind[iCount].buffer = (void*)&data_alpha;";
                sData += "\n                bind[iCount].buffer_length = sizeof(float);";
                sData += "\n            }";
                sData += "\n            break;";
                continue;
            }
        }

        switch((*itr).enDataType){

        case _int:
            {  
                sData += "\n                data_" + (*itr).sColumnName + " = src.get_" + (*itr).sColumnName + "();";
                sData += "\n                bind[iCount].buffer_type = MYSQL_TYPE_LONG;";
                sData += "\n                bind[iCount].buffer = (void*)&data_" + (*itr).sColumnName +";";
                sData += "\n                bind[iCount].buffer_length = sizeof(int);";
            }
            break;

        case _bigint:
            {  
                sData += "\n                data_" + (*itr).sColumnName + " = src.get_" + (*itr).sColumnName + "();";
                sData += "\n                bind[iCount].buffer_type = MYSQL_TYPE_LONGLONG;";
                sData += "\n                bind[iCount].buffer = (void*)&data_" + (*itr).sColumnName +";";
                sData += "\n                bind[iCount].buffer_length = sizeof(long long);";
            }
            break;

        case _float:
            {  
                sData += "\n                data_" + (*itr).sColumnName + " = src.get_" + (*itr).sColumnName + "();";
                sData += "\n                bind[iCount].buffer_type = MYSQL_TYPE_FLOAT;";
                sData += "\n                bind[iCount].buffer = (void*)&data_" + (*itr).sColumnName +";";
                sData += "\n                bind[iCount].buffer_length = sizeof(float);";
            }
            break;

        case _double:
            {  
                sData += "\n                data_" + (*itr).sColumnName + " = src.get_" + (*itr).sColumnName + "();";
                sData += "\n                bind[iCount].buffer_type = MYSQL_TYPE_DOUBLE;";
                sData += "\n                bind[iCount].buffer = (void*)&data_" + (*itr).sColumnName +";";
                sData += "\n                bind[iCount].buffer_length = sizeof(double);";
            }
            break;

        case _bool:
        case _tinyint:
            {
                sData += "\n                data_" + (*itr).sColumnName + " = src.get_" + (*itr).sColumnName + "();";
                sData += "\n                bind[iCount].buffer_type = MYSQL_TYPE_TINY;";
                sData += "\n                bind[iCount].buffer = (void*)&data_" + (*itr).sColumnName +";";
                sData += "\n                bind[iCount].buffer_length = sizeof(bool);";
            }
            break;

        case _datetime:
            {
               sData += "\n                ConvertDateTime2DB(src.get_" + (*itr).sColumnName + "(), &data_" + (*itr).sColumnName + ");";
                sData += "\n                bind[iCount].buffer_type = MYSQL_TYPE_DATETIME;";
                sData += "\n                bind[iCount].buffer = (void*)&data_" + (*itr).sColumnName + ";";
                sData += "\n                if (src.get_" + (*itr).sColumnName + "().is_not_a_date_time()) bind[iCount].is_null = &isnull;";
            }
            break;
        case _timestamp:
            {
                sData += "\n                ConvertDateTime2DB(src.get_" + (*itr).sColumnName + "(), &data_" + (*itr).sColumnName + ");";
                sData += "\n                bind[iCount].buffer_type = MYSQL_TYPE_TIMESTAMP;";
                sData += "\n                bind[iCount].buffer = (void*)&data_" + (*itr).sColumnName + ";";
                sData += "\n                if (src.get_" + (*itr).sColumnName + "().is_not_a_date_time()) bind[iCount].is_null = &isnull;";
            }
            break;

        case _date:
            {
                sData += "\n                ConvertDateTime2DB(src.get_" + (*itr).sColumnName + "(), &data_" + (*itr).sColumnName + ");";
                sData += "\n                bind[iCount].buffer_type = MYSQL_TYPE_DATE;";
                sData += "\n                bind[iCount].buffer = (void*)&data_" + (*itr).sColumnName + ";";
                //sData += "\n                bind[i].buffer_length = 0;";
                sData += "\n                if (src.get_" + (*itr).sColumnName + "().is_not_a_date()) bind[iCount].is_null = &isnull;";
            }
            break;

        case _time:
            {
                sData += "\n                ConvertDateTime2DB(src.get_" + (*itr).sColumnName + "(), &data_" + (*itr).sColumnName + ");";
                sData += "\n                bind[iCount].buffer_type = MYSQL_TYPE_TIME;";
                sData += "\n                bind[iCount].buffer = (void*)&data_" + (*itr).sColumnName + ";";
                //sData += "\n                bind[i].buffer_length = 0;";
                sData += "\n                if (src.get_" + (*itr).sColumnName + "().is_not_a_date_time()) bind[iCount].is_null = &isnull;";
            }
            break;
        case _blob:
        case _mediumblob:
        case _longblob:
            {
                if ("leafpositions" == (*itr).sColumnName){//mlcshape
                    sData += "\n                std::vector<db_Point2d> leafpositions = src.get_leafpositions();";
                    sData += "\n                const size_t leafSize = leafpositions.size();";
                    sData += "\n                if (leafSize > 1 && leafSize%2 == 0){";
                    sData += "\n                    //const size_t total_length = leafSize*2*FLT_MAX_LENGHT + leafSize*2;//for \"\\\\\"";
                    sData += "\n                    data_leafpositions += std::to_string((long double)(leafpositions[0].x));";
                    sData += "\n                    for (int x(1); x<leafSize; ++x){//300A,00BC 101,102,,,1N,201,202,,,2N";
                    sData += "\n                        data_leafpositions += \"\\\\\" + std::to_string((long double)(leafpositions[x].x));";
                    sData += "\n                    }";
                    sData += "\n                    for (int y(0); y<leafSize; ++y){";
                    sData += "\n                        data_leafpositions += \"\\\\\" + std::to_string((long double)(leafpositions[y].y));";
                    sData += "\n                    }";
                    sData += "\n                }";

                    sData += "\n                bind[iCount].buffer_type= MYSQL_TYPE_BLOB;";
                    sData += "\n                bind[iCount].buffer= (void *)data_leafpositions.c_str();";
                    sData += "\n                bind[iCount].buffer_length= (unsigned long)data_leafpositions.size();";
                }
                else if (TYPE_CONTOUR == table.enTableType && "points"==(*itr).sColumnName){
                    sData += "\n                const std::vector<db_Point3f> vPoints = src.get_contour_points();";
                    sData += "\n                const int data_accuracy = src.get_accuracy();";
                    sData += "\n                const float fAcc = std::pow(10.0f,data_accuracy);";
                    sData += "\n                if (!vPoints.empty()){";
                    sData += "\n                    int iCount(0);";
                    sData += "\n                    for (auto itr=vPoints.cbegin(); itr!=vPoints.cend(); ++itr,++iCount){";
                    sData += "\n                        data_points += (0==iCount? \"\" :\"\\\\\") + std::to_string(long long((*itr).x * fAcc));//float to int";
                    sData += "\n                        data_points += \"\\\\\" + std::to_string(long long((*itr).y * fAcc));";
                    sData += "\n                        data_points += \"\\\\\" + std::to_string(long long((*itr).z * fAcc));";
                    sData += "\n                    }";
                    sData += "\n                }";
                    sData += "\n                bind[iCount].buffer_type = MYSQL_TYPE_BLOB;";
                    sData += "\n                bind[iCount].buffer = (void*)data_points.c_str();";
                    sData += "\n                bind[iCount].buffer_length = (unsigned long)data_points.size();";
                }
                else if (TYPE_RTIMAGE == table.enTableType && "pixeldata"==(*itr).sColumnName){
                    sData += "\n                unsigned long ulSize;";
                    sData += "\n                data_pixeldata = src.get_pixel_data_buffer(&ulSize);";
                    sData += "\n                if(nullptr == data_pixeldata || ulSize < 1){";
                    sData += "\n                    bind[iCount].is_null = &isnull;";
                    sData += "\n                    break;";
                    sData += "\n                }";
                    sData += "\n                bind[iCount].buffer_type= MYSQL_TYPE_MEDIUM_BLOB;";
                    sData += "\n                bind[iCount].buffer= (void *)data_pixeldata;";
                    sData += "\n                bind[iCount].buffer_length= (unsigned long)(ulSize*sizeof(char));";
                }
                else if (TYPE_CT2DENSITY == table.enTableType && "ct2densityvalue"==(*itr).sColumnName){
                    sData += "\n                if(src.get_ct2densitymap().empty()) {";
                    sData += "\n                    bind[iCount].is_null = &isnull;";
                    sData += "\n                    break;";
                    sData += "\n                }";
                    sData += "\n                ";
                    sData += "\n                std::string sData(\"\");";
                    sData += "\n                int iCount(0);";
                    sData += "\n                const std::map<int, float> densitymap = src.get_ct2densitymap();";
                    sData += "\n                for (auto itr=densitymap.cbegin(); itr!=densitymap.cend(); ++itr, ++iCount){";
                    sData += "\n                    sData += std::to_string((long long )((*itr).first)) + \",\" + std::to_string((long double)((*itr).second));";
                    sData += "\n                    if (iCount != densitymap.size()-1){";
                    sData += "\n                        sData += \"|\";";
                    sData += "\n                    }";
                    sData += "\n                }";
                    sData += "\n                data_ct2densityvalue = sData;";
                    sData += "\n                bind[iCount].buffer_type= MYSQL_TYPE_BLOB;";
                    sData += "\n                bind[iCount].buffer= (void *)data_ct2densityvalue.c_str();";
                    sData += "\n                bind[iCount].buffer_length= (unsigned long)data_ct2densityvalue.size();";
                }
                else if (TYPE_BLOCK == table.enTableType && "points"==(*itr).sColumnName){
                    sData += "\n                const std::vector<db_Point2f> vPt = src.get_points();";
                    sData += "\n                if(vPt.empty()) bind[iCount].is_null = &isnull;";
                    sData += "\n                const size_t iSize = vPt.size();";
                    sData += "\n                data_points = \"\";";
                    sData += "\n                if (iSize > 0){";
                    sData += "\n                    for(int i=0; i<iSize; ++i){";
                    sData += "\n                        if (!data_points.empty()) data_points += \"|\";";
                    sData += "\n                        data_points += std::to_string((long double)(vPt[i].x)) +\",\";";
                    sData += "\n                        data_points += std::to_string((long double)(vPt[i].y));";
                    sData += "\n                    }";
                    sData += "\n                }";
                    sData += "\n                bind[iCount].buffer_type= MYSQL_TYPE_BLOB;";
                    sData += "\n                bind[iCount].buffer= (void *)data_points.c_str();";
                    sData += "\n                bind[iCount].buffer_length= (unsigned long)data_points.size();";
                }
                else if (TYPE_COMMISSIONEDUNIT == table.enTableType && "discretedoserate"==(*itr).sColumnName){
                    sData += "\n                std::vector<float> discretedoserate = src.get_discretedoserate();";
                    sData += "\n                if(discretedoserate.empty()) bind[iCount].is_null = &isnull;";
                    sData += "\n                const size_t vSize = discretedoserate.size();";
                    sData += "\n                if (vSize > 0){";
                    sData += "\n                    data_discretedoserate += std::to_string((long double)(discretedoserate[0]));";
                    sData += "\n                    for (int x(1); x<vSize; ++x){";
                    sData += "\n                        data_discretedoserate += \",\" + std::to_string((long double)(discretedoserate[x]));";
                    sData += "\n                    }";
                    sData += "\n                }";
                    sData += "\n                bind[iCount].buffer_type= MYSQL_TYPE_BLOB;";
                    sData += "\n                bind[iCount].buffer= (void *)data_discretedoserate.c_str();";
                    sData += "\n                bind[iCount].buffer_length= (unsigned long)data_discretedoserate.size();";
                }
                else if(TYPE_DOSEGRID == table.enTableType && "dosegridvalue" == (*itr).sColumnName){
                    sData += "\n                const int iLen = src.get_xcount()* src.get_ycount()* src.get_zcount();";
                    sData += "\n                const double dAccuracy = src.get_accuracy();";
                    sData += "\n                const float* pBuffer = src.get_dosegrid_buffer();";
                    sData += "\n                if (iLen < 1 || pBuffer == nullptr){";
                    sData += "\n                    bind[iCount].is_null = &isnull;";
                    sData += "\n                    break;";
                    sData += "\n                }";
                    sData += "\n                data_dosegridvalue = new uint32_t[iLen];";
                    sData += "\n                const int iMutiple = static_cast<int>(1/dAccuracy);";
                    sData += "\n                for (int x = 0;x < iLen; ++x)";
                    sData += "\n                    data_dosegridvalue[x] = static_cast<uint32_t>(pBuffer[x] * iMutiple);";
                    sData += "\n                bind[iCount].buffer_type= MYSQL_TYPE_MEDIUM_BLOB;";
                    sData += "\n                bind[iCount].buffer= (void *)data_dosegridvalue;";
                    sData += "\n                bind[iCount].buffer_length= (unsigned long)(iLen*sizeof(uint32_t));";
                }
                else if ("leafboundaries" == (*itr).sColumnName && TYPE_MACHINE == table.enTableType){
                    sData += "\n                std::vector<double> leafboundaries = src.get_leafboundaries();";
                    sData += "\n                if(leafboundaries.empty()) bind[iCount].is_null = &isnull;";
                    sData += "\n                const size_t leafSize = leafboundaries.size();";
                    sData += "\n                if (leafSize > 0){";
                    sData += "\n                    data_leafboundaries += std::to_string((long double)(leafboundaries[0]));";
                    sData += "\n                    for (int y(1); y<leafSize; ++y){";
                    sData += "\n                        data_leafboundaries += \"\\\\\" + std::to_string((long double)(leafboundaries[y]));";
                    sData += "\n                    }";
                    sData += "\n                }";
                    sData += "\n                bind[iCount].buffer_type= MYSQL_TYPE_BLOB;";
                    sData += "\n                bind[iCount].buffer= (void *)data_leafboundaries.c_str();";
                    sData += "\n                bind[iCount].buffer_length= (unsigned long)data_leafboundaries.size();";
                }
                //else if (TYPE_LONGBLOB == table.enTableType && "data"==(*itr).sColumnName){
                //    sData += "\n                data_data = src.get_data(&ulSize);";
                //    sData += "\n                if (ulSize < 1 || nullptr == data_data){";
                //    sData += "\n                    bind[iCount].is_null = &isnull;";
                //    sData += "\n                    break;";
                //    sData += "\n                }";
                //    sData += "\n                bind[iCount].buffer_type= MYSQL_TYPE_LONG_BLOB;";
                //    sData += "\n                bind[iCount].buffer= (void *)data_data;";
                //    sData += "\n                bind[iCount].buffer_length= (unsigned long)(ulSize * sizeof(char));";
                //}
                else
                {
                    sData += "\n                data_" + (*itr).sColumnName + " = src.get_" + (*itr).sColumnName + "();";
                    sData += "\n                if(data_" + (*itr).sColumnName + ".empty()) bind[iCount].is_null = &isnull;";
                    sData += "\n                bind[iCount].buffer_type= MYSQL_TYPE_BLOB;";
                    sData += "\n                bind[iCount].buffer= (void *)data_" + (*itr).sColumnName + ".c_str();";
                    sData += "\n                bind[iCount].buffer_length= (unsigned long)data_" + (*itr).sColumnName + ".size();";
                }
            }
            break;

        case _varchar:
        case _char:
        case _unknown:
        default:
            if ((shortName=="series") &&("patientage"==(*itr).sColumnName)){
                sData += "\n                const std::string& value = src.get_patientage();";
                sData += "\n                data_patientage = value.substr(0,4);";
                sData += "\n                bind[iCount].buffer_type = MYSQL_TYPE_VAR_STRING;";
                sData += "\n                bind[iCount].buffer = (void*)data_patientage.c_str();";
                sData += "\n                bind[iCount].buffer_length = (unsigned long)data_patientage.size();";
            }
            else if(TYPE_DOSEGRID == table.enTableType && "grid_to_pat_t" == (*itr).sColumnName) {
                sData += "\n                const float* matrix = src.get_grid_to_pat_t(); ";
                sData += "\n                if (nullptr != matrix){";
                sData += "\n                    for (int j(0); j<4; ++j){";
                sData += "\n                        for (int i(0); i<4; ++i){";
                sData += "\n                            data_grid_to_pat_t += std::to_string(long double(matrix[i + j*4])) + ((3==j && 3==i)? \"\":\",\");";
                sData += "\n                        }";
                sData += "\n                    }";
                sData += "\n                }";
                sData += "\n                bind[iCount].buffer_type = MYSQL_TYPE_VAR_STRING;";
                sData += "\n                bind[iCount].buffer = (void*)data_grid_to_pat_t.c_str();";
                sData += "\n                bind[iCount].buffer_length = (unsigned long)data_grid_to_pat_t.size();";
            }
            else if(TYPE_VOI == table.enTableType && "pat2volumematrix" == (*itr).sColumnName) {
                sData += "\n                const float* matrix = src.get_pat2volumematrix(); ";
                sData += "\n                if (nullptr != matrix){";
                sData += "\n                    for (int j(0); j<4; ++j){";
                sData += "\n                        for (int i(0); i<4; ++i){";
                sData += "\n                            data_pat2volumematrix += std::to_string(long double(matrix[i + j*4])) + ((3==j && 3==i)? \"\":\",\");";
                sData += "\n                        }";
                sData += "\n                    }";
                sData += "\n                }";
                sData += "\n                bind[iCount].buffer_type = MYSQL_TYPE_VAR_STRING;";
                sData += "\n                bind[iCount].buffer = (void*)data_pat2volumematrix.c_str();";
                sData += "\n                bind[iCount].buffer_length = (unsigned long)data_pat2volumematrix.size();";
            }
            else if(TYPE_VOI == table.enTableType && "interpolate" == (*itr).sColumnName) {
                sData += "\n                const std::vector<bool>& vValue = src.get_interpolate();";
                sData += "\n                int iCount(0);";
                sData += "\n                for (auto itr=vValue.cbegin(); itr!=vValue.cend(); ++itr, ++iCount){";
                sData += "\n                    data_interpolate += ((*itr)? \"1\":\"0\"); ";
                sData += "\n                    data_interpolate += (iCount!=vValue.size()-1 ? \",\":\"\");";
                sData += "\n                }";
                sData += "\n                bind[iCount].buffer_type = MYSQL_TYPE_VAR_STRING;";
                sData += "\n                bind[iCount].buffer = (void*)data_interpolate.c_str();";
                sData += "\n                bind[iCount].buffer_length = (unsigned long)data_interpolate.size();";
            }
            else if(TYPE_BEAMSEGMENT == table.enTableType && "t_beam_to_pat" == (*itr).sColumnName) {
                sData += "\n                const float* matrix = src.get_t_beam_to_pat(); ";
                sData += "\n                if (nullptr != matrix){";
                sData += "\n                    for (int j(0); j<4; ++j){";
                sData += "\n                        for (int i(0); i<4; ++i){";
                sData += "\n                            data_t_beam_to_pat += std::to_string(long double(matrix[i + j*4])) + ((3==j && 3==i)? \"\":\",\");";
                sData += "\n                        }";
                sData += "\n                    }";
                sData += "\n                }";
                sData += "\n                bind[iCount].buffer_type = MYSQL_TYPE_VAR_STRING;";
                sData += "\n                bind[iCount].buffer = (void*)data_t_beam_to_pat.c_str();";
                sData += "\n                bind[iCount].buffer_length = (unsigned long)data_t_beam_to_pat.size();";
            }
            else if(TYPE_IMAGETRANSFORMATION == table.enTableType && "registrationmatrix" == (*itr).sColumnName) {
                sData += "\n                const float* matrix = src.get_registrationmatrix(); ";
                sData += "\n                if (nullptr != matrix){";
                sData += "\n                    for (int j(0); j<4; ++j){";
                sData += "\n                        for (int i(0); i<4; ++i){";
                sData += "\n                            data_registrationmatrix += std::to_string(long double(matrix[i + j*4])) + ((3==j && 3==i)? \"\":\",\");";
                sData += "\n                        }";
                sData += "\n                    }";
                sData += "\n                }";
                sData += "\n                bind[iCount].buffer_type = MYSQL_TYPE_VAR_STRING;";
                sData += "\n                bind[iCount].buffer = (void*)data_registrationmatrix.c_str();";
                sData += "\n                bind[iCount].buffer_length = (unsigned long)data_registrationmatrix.size();";
            }
            
            else
            {
                sData += "\n                data_" + (*itr).sColumnName + " = src.get_" + (*itr).sColumnName + "();";
                sData += "\n                bind[iCount].buffer_type = MYSQL_TYPE_VAR_STRING;";
                sData += "\n                bind[iCount].buffer = (void*)data_" + (*itr).sColumnName + ".c_str();";
                sData += "\n                bind[iCount].buffer_length = (unsigned long)data_" + (*itr).sColumnName + ".size();";
            }
            break;
        }

        sData += "\n            }";
        sData += "\n            break;";
    }

    sData += "\n        default:";
    sData += "\n            break;";
    sData += "\n        }";   

    sData += "\n"; 
    sData += "\n        ++iCount;";  
    sData += "\n    }";

    sData += "\n    if (mysql_stmt_bind_param(stmt, bind))";
    sData += "\n    {";
    sData += "\n        TPS_LOG_DEV_ERROR<<\" mysql_stmt_bind_param(), failed \"<< mysql_stmt_error(stmt);";
    sData += "\n        DEL_ARRAY(bind);";
    if (TYPE_DOSEGRID == table.enTableType)
    {
        sData += "\n        DEL_ARRAY(data_dosegridvalue);";
    }
    sData += "\n        return false;";
    sData += "\n    }";
    sData += "\n    if (mysql_stmt_execute(stmt))";
    sData += "\n    {";
    sData += "\n        TPS_LOG_DEV_ERROR<<\" mysql_stmt_execute(), failed \"<< mysql_stmt_error(stmt);";
    sData += "\n        TPS_PRINTF_DEBUG(\"mysql_stmt_execute(), failed %s\\n\", mysql_stmt_error(stmt));";
    sData += "\n        DEL_ARRAY(bind);";
    if (TYPE_DOSEGRID == table.enTableType )
    {
        sData += "\n        DEL_ARRAY(data_dosegridvalue);";
    }
    sData += "\n        return false;";
    sData += "\n    }";
    sData += "\n    if (mysql_stmt_close(stmt))";
    sData += "\n    {";
    sData += "\n        TPS_LOG_DEV_ERROR<<\" failed while closing the statement \"<< mysql_stmt_error(stmt);";
    sData += "\n        DEL_ARRAY(bind);";
    if (TYPE_DOSEGRID == table.enTableType )
    {
        sData += "\n        DEL_ARRAY(data_dosegridvalue);";
    }
    sData += "\n        return false;";
    sData += "\n    }";
    sData += "\n    DEL_ARRAY(bind);";
    if (TYPE_DOSEGRID == table.enTableType )
    {
        sData += "\n    DEL_ARRAY(data_dosegridvalue);";
    }
	sData += "\n    RtDatabaseObject* rtDatabaseObject =const_cast<" + sClassName + "*>(&src);";
	sData += "\n    rtDatabaseObject->setdirty_all_fields(false);";
    sData += "\n    return true;";
    sData += "\n}";//InsertTableFromObject end


    //////////////////////////////////////////////////////////////////////////
    //FetchDataFromDB begin

    sData += "\n";
    sData += "\nbool RtDatabaseHelper::FetchDataFromDB(const std::string& sSql, std::vector<" + sClassName + "*>& vList)";
    sData += "\n{";
    sData += "\n    MYSQL_STMT* stmt = mysql_stmt_init(m_pMySql);";
    sData += "\n    if (!stmt)";
    sData += "\n    {";
    sData += "\n        TPS_LOG_DEV_ERROR<<\"mysql_stmt_init(), out of memory\";";
    sData += "\n        return false;";
    sData += "\n    }";

    sData += "\n";
    sData += "\n    MYSQL_RES  *prepare_meta_result = nullptr;";
    sData += "\n    //tms" + shortName +"    " + strNum + " parameters";
    sData += "\n    const int field_num = " + sTableNameUpper +"_FIELD_MAX;";
    sData += "\n    unsigned long MAX_LENGTH[field_num] = {0};";
    int iCount(0);
    for (auto itr=table.vColumnItems.cbegin(); itr!=table.vColumnItems.cend(); ++itr,++iCount)
    {
        sData += "\n    MAX_LENGTH[" + std::to_string((long long)iCount) + "] = " + std::to_string((long long)(*itr).iMaxLength) + ";";
    }

    sData += "\n";
    iCount = 0;
    int iCountDefault = 0;
    for (auto itr=table.vColumnItems.cbegin(); itr!=table.vColumnItems.cend(); ++itr,++iCount){
        const std::string sType = (*itr).sCppDataType;
        switch((*itr).enDataType){
        case _int:
        case _bigint:
            {  
                sData += "\n    " + sType +"\tdata_" + (*itr).sColumnName + " = 0;";
            }
            break;
        case _float:
            {  
                sData += "\n    " + sType +"\tdata_" + (*itr).sColumnName + " = 0.f;";
            }
            break;
        case _double:
            {  
                sData += "\n    " + sType +"\tdata_" + (*itr).sColumnName + " = 0.;";
            }
            break;
        case _bool:
        case _tinyint:
            {
                sData += "\n    " + sType +"\tdata_" + (*itr).sColumnName + " = false;";
            }
            break;

        case _blob:
        case _mediumblob:
        case _longblob:
            {
                sData += "\n    //" + sType +"\tdata_" + (*itr).sColumnName + ";";
            }
            break;

        case _datetime:
        case _timestamp:
        case _date:
        case _time:
            {
                sData += "\n    MYSQL_TIME\tdata_" + (*itr).sColumnName + ";";
            }
            break;

        case _varchar:
        case _char:
        case _unknown:
        default:  
            ++iCountDefault;
            sData += "\n    //" + sType +"\tdata_" + (*itr).sColumnName;
            break;
        }
    }


    sData += "\n";
    sData += "\n    if (mysql_stmt_prepare(stmt, sSql.c_str(), (unsigned long)strlen(sSql.c_str())))";
    sData += "\n    {";
    sData += "\n        TPS_LOG_DEV_ERROR<<\" mysql_stmt_prepare(), SELECT failed.\"<<mysql_stmt_error(stmt);";
    sData += "\n        return false;";
    sData += "\n    }";

    sData += "\n";
    sData += "\n    //Fetch result set meta information";
    sData += "\n    prepare_meta_result = mysql_stmt_result_metadata(stmt);";
    sData += "\n    if (!prepare_meta_result) ";
    sData += "\n    {";
    sData += "\n        TPS_LOG_DEV_ERROR<<\" mysql_stmt_result_metadata(), returned no meta information \"<<mysql_stmt_error(stmt);";
    sData += "\n        return false;";
    sData += "\n    }";

    sData += "\n";
    sData += "\n    // Get total columns in the query and validate column count";
    sData += "\n    const unsigned int column_count= mysql_num_fields(prepare_meta_result);";
    sData += "\n    if (column_count != field_num)";
    sData += "\n    {";
    sData += "\n        TPS_LOG_DEV_ERROR<<\"invalid column count returned by MySQL\";";
    sData += "\n        return false;";
    sData += "\n    }";

    sData += "\n";
    sData += "\n    MYSQL_BIND* bind = new MYSQL_BIND[column_count];";
    sData += "\n    memset(bind, 0, sizeof(MYSQL_BIND)*column_count);";
    sData += "\n    unsigned long* length = new unsigned long[column_count];";
    sData += "\n    memset(length, 0, sizeof(unsigned long) * column_count);";
    sData += "\n    my_bool* is_null = new my_bool[column_count];";
    sData += "\n    memset(is_null, 0, sizeof(my_bool) * column_count);";
    sData += "\n    //Execute the SELECT query";
    sData += "\n    char* data_string[field_num];";
    sData += "\n    for (int i=0; i<field_num; ++i)";
    sData += "\n    {";
    sData += "\n        const unsigned long ulSize = MAX_LENGTH[i] + 1;// + 1 for string's \\0 ";
    sData += "\n        data_string[i] = new char[ulSize];";
    sData += "\n        memset(data_string[i], 0, ulSize*sizeof(char));";
    sData += "\n    }";
    sData += "\n    if (mysql_stmt_execute(stmt))";
    sData += "\n    {";
    sData += "\n        TPS_LOG_DEV_ERROR<<\" mysql_stmt_execute(), failed\"<<mysql_stmt_error(stmt);";
    sData += "\n        for (int i=0; i<field_num; ++i) DEL_ARRAY(data_string[i]);";
    sData += "\n        DEL_ARRAY(bind);";
    sData += "\n        DEL_ARRAY(length);";
    sData += "\n        DEL_ARRAY(is_null);";
    sData += "\n        return false;";
    sData += "\n    }";

    sData += "\n";
    sData += "\n    for (unsigned int i(0); i<column_count; ++i)";
    sData += "\n    {";
    sData += "\n        bind[i].is_null= &is_null[i];";
    sData += "\n        bind[i].length= &length[i];";
    if (iCountDefault != table.vColumnItems.size()){
        sData += "\n        switch (i)";
        sData += "\n        {";
    }

    iCount = 0;
    for (auto itr=table.vColumnItems.cbegin(); itr!=table.vColumnItems.cend(); ++itr, ++iCount){
        const std::string sDataType = std::string(Enum2StringDataType[(*itr).enDataType]);
        switch((*itr).enDataType){
        case _int:
            {
                sData += "\n        case " + std::to_string((long long)iCount) + ": \t//" + (*itr).sColumnName + " "+ sDataType;
                sData += "\n            bind[i].buffer_type= MYSQL_TYPE_LONG;";
                sData += "\n            bind[i].buffer= (char *)&data_" + (*itr).sColumnName +";";
                sData += "\n            break;";
            }
            break;
        case _bigint:
            {
                sData += "\n        case " + std::to_string((long long)iCount) + ": \t//" + (*itr).sColumnName + " "+ sDataType;
                sData += "\n            bind[i].buffer_type= MYSQL_TYPE_LONGLONG;";
                sData += "\n            bind[i].buffer= (char *)&data_" + (*itr).sColumnName +";";
                sData += "\n            break;";
            }
            break;
        case _float:
            {
                sData += "\n        case " + std::to_string((long long)iCount) + ": \t//" + (*itr).sColumnName + " "+ sDataType;
                sData += "\n            bind[i].buffer_type= MYSQL_TYPE_FLOAT;";
                sData += "\n            bind[i].buffer= (char *)&data_" + (*itr).sColumnName +";";
                sData += "\n            break;";
            }   
            break;
        case _double:
            {
                sData += "\n        case " + std::to_string((long long)iCount) + ": \t//" + (*itr).sColumnName + " "+ sDataType;
                sData += "\n            bind[i].buffer_type= MYSQL_TYPE_DOUBLE;";
                sData += "\n            bind[i].buffer= (char *)&data_" + (*itr).sColumnName +";";
                sData += "\n            break;";
            }   
            break;
        case _bool:
        case _tinyint:
            {
                sData += "\n        case " + std::to_string((long long)iCount) + ": \t//" + (*itr).sColumnName + " "+ sDataType;
                sData += "\n            bind[i].buffer_type= MYSQL_TYPE_TINY;";
                sData += "\n            bind[i].buffer= (char *)&data_" + (*itr).sColumnName +";";
                sData += "\n            break;";
            }
            break;
        case _blob:
            {
                sData += "\n        case " + std::to_string((long long)iCount) + ": \t//" + (*itr).sColumnName + " "+ sDataType;
                sData += "\n            bind[i].buffer_type= MYSQL_TYPE_BLOB;";
                //sData += "\n            bind[i].buffer= (char *)&data_" + (*itr).sColumnName +";";
                sData += "\n            break;";
            }
            break;
        case _mediumblob:
            {
                sData += "\n        case " + std::to_string((long long)iCount) + ": \t//" + (*itr).sColumnName + " "+ sDataType;
                sData += "\n            bind[i].buffer_type= MYSQL_TYPE_MEDIUM_BLOB;";
                sData += "\n            break;";
            }
            break;
        case _longblob:
            {
                sData += "\n        case " + std::to_string((long long)iCount) + ": \t//" + (*itr).sColumnName + " "+ sDataType;
                sData += "\n            bind[i].buffer_type= MYSQL_TYPE_LONG_BLOB;";
                sData += "\n            break;";
            }
            break;
        case _datetime:
            {
                sData += "\n        case " + std::to_string((long long)iCount) + ": \t//" + (*itr).sColumnName + " "+ sDataType;
                sData += "\n            bind[i].buffer_type= MYSQL_TYPE_DATETIME;";
                sData += "\n            bind[i].buffer= (char *)&data_" + (*itr).sColumnName +";";
                sData += "\n            break;";
            }
            break;
        case _timestamp:
            {
                sData += "\n        case " + std::to_string((long long)iCount) + ": \t//" + (*itr).sColumnName + " "+ sDataType;
                sData += "\n            bind[i].buffer_type= MYSQL_TYPE_TIMESTAMP;";
                sData += "\n            bind[i].buffer= (char *)&data_" + (*itr).sColumnName +";";
                sData += "\n            break;";
            }
            break;
        case _date:
            {
                sData += "\n        case " + std::to_string((long long)iCount) + ": \t//" + (*itr).sColumnName + " "+ sDataType;
                sData += "\n            bind[i].buffer_type= MYSQL_TYPE_DATE;";
                sData += "\n            bind[i].buffer= (char *)&data_" + (*itr).sColumnName +";";
                sData += "\n            break;";
            }
            break;
        case _time:
            {
                sData += "\n        case " + std::to_string((long long)iCount) + ": \t//" + (*itr).sColumnName + " "+ sDataType;
                sData += "\n            bind[i].buffer_type= MYSQL_TYPE_TIME;";
                sData += "\n            bind[i].buffer= (char *)&data_" + (*itr).sColumnName +";";
                sData += "\n            break;";
            }
            break;

        case _varchar:
        case _char:
        case _unknown:
        default:
            break;
        }
    }

    if (iCountDefault != table.vColumnItems.size()){
        sData += "\n        default:";
        sData += "\n            bind[i].buffer_type= MYSQL_TYPE_STRING;";
        sData += "\n            bind[i].buffer= (char *)data_string[i];";
        sData += "\n            bind[i].buffer_length= MAX_LENGTH[i];";
        sData += "\n            break;";
        sData += "\n        }";
    }
    else{
        sData += "\n        bind[i].buffer_type= MYSQL_TYPE_STRING;";
        sData += "\n        bind[i].buffer= (char *)data_string[i];";
        sData += "\n        bind[i].buffer_length= MAX_LENGTH[i];";
    }

    sData += "\n    }";

    sData += "\n";
    sData += "\n    // Bind the result buffers";
    sData += "\n    if (mysql_stmt_bind_result(stmt, bind))";
    sData += "\n    {";
    sData += "\n        TPS_LOG_DEV_ERROR<<\" mysql_stmt_bind_result() failed \"<<mysql_stmt_error(stmt);";
    sData += "\n        for (int i=0; i<field_num; ++i) DEL_ARRAY(data_string[i]);";
    sData += "\n        DEL_ARRAY(bind);";
    sData += "\n        DEL_ARRAY(length);";
    sData += "\n        DEL_ARRAY(is_null);";
    sData += "\n        return false;"; 
    sData += "\n    }";

    sData += "\n";
    sData += "\n    //Now buffer all results to client";
    sData += "\n    if (mysql_stmt_store_result(stmt))";
    sData += "\n    {";
    sData += "\n        TPS_LOG_DEV_ERROR<<\" mysql_stmt_store_result() failed \"<<mysql_stmt_error(stmt);";
    sData += "\n        for (int i=0; i<field_num; ++i) DEL_ARRAY(data_string[i]);";
    sData += "\n        DEL_ARRAY(bind);";
    sData += "\n        DEL_ARRAY(length);";
    sData += "\n        DEL_ARRAY(is_null);";
    sData += "\n        return false;"; 
    sData += "\n    }";

    sData += "\n";
    sData += "\n    // Fetch all rows ";
    sData += "\n    int ret(1);";
    sData += "\n    " + sClassName + "* pDis = nullptr;";
    sData += "\n    while(true)";
    sData += "\n    {";
    sData += "\n        ret = mysql_stmt_fetch(stmt);";
    sData += "\n        if (ret!=0 && ret!=MYSQL_DATA_TRUNCATED) break;";
    sData += "\n        pDis = new " +sClassName + "();";

    if (table.enTableType == TYPE_VOI){
        sData += "\n        const float* fColor = pDis->get_color();";
        sData += "\n        float fColorNew[4] = {0};";
        sData += "\n        memcpy(fColorNew, fColor, sizeof(float)*4);";
    }

    sData += "\n        for (unsigned int i(0); i<column_count; ++i)";
    sData += "\n        {";
    sData += "\n            if (is_null[i]) continue;";
    sData += "\n            switch (i)";
    sData += "\n            {";

    iCount = 0;
    for (auto itr=table.vColumnItems.cbegin(); itr!=table.vColumnItems.cend(); ++itr, ++iCount){
        const std::string sDataType = std::string(Enum2StringDataType[(*itr).enDataType]);
        if(table.enTableType == TYPE_VOI){
            if ((*itr).sColumnName == "red"){
                sData += "\n            case " + std::to_string((long long)iCount) + ": \t//" + (*itr).sColumnName + " "+ sDataType;
                sData += "\n                {";
                sData += "\n                    fColorNew[0] = data_red;";
                sData += "\n                    pDis->set_color(fColorNew);";
                sData += "\n                }";
                sData += "\n                break;";
                continue;
            }
            else if((*itr).sColumnName == "green"){
                sData += "\n            case " + std::to_string((long long)iCount) + ": \t//" + (*itr).sColumnName + " "+ sDataType;
                sData += "\n                {";
                sData += "\n                    fColorNew[1] = data_green;";
                sData += "\n                    pDis->set_color(fColorNew);";
                sData += "\n                }";
                sData += "\n                break;";
                continue;
            }
            else if((*itr).sColumnName == "blue"){
                sData += "\n            case " + std::to_string((long long)iCount) + ": \t//" + (*itr).sColumnName + " "+ sDataType;
                sData += "\n                {";
                sData += "\n                    fColorNew[2] = data_blue;";
                sData += "\n                    pDis->set_color(fColorNew);";
                sData += "\n                }";
                sData += "\n                break;";
                continue;
            }
            else if((*itr).sColumnName == "alpha"){
                sData += "\n            case " + std::to_string((long long)iCount) + ": \t//" + (*itr).sColumnName + " "+ sDataType;
                sData += "\n                {";
                sData += "\n                    fColorNew[3] = data_alpha;";
                sData += "\n                    pDis->set_color(fColorNew);";
                sData += "\n                }";
                sData += "\n                break;";
                continue;
            }
        }

        switch((*itr).enDataType){
        case _int:
        case _bigint: 
        case _float:
        case _double:
        case _bool:
        case _tinyint:
            {
                sData += "\n            case " + std::to_string((long long)iCount) + ": \t//" + (*itr).sColumnName + " "+ sDataType;
                sData += "\n                pDis->set_" + (*itr).sColumnName +"(data_" + (*itr).sColumnName + ");";
                sData += "\n                break;";
            }
            break;
        case _blob:
            {
                sData += "\n            case " + std::to_string((long long)iCount) + ": \t//" + (*itr).sColumnName + " "+ sDataType;
                if ("leafpositions" == (*itr).sColumnName){
                    sData += "\n                {";
                    sData += "\n                    const int total_length = length[i];";
                    sData += "\n                    if (total_length < 2){";
                    sData += "\n                        break;";
                    sData += "\n                    }";
                    sData += "\n                    char *buf=new char[total_length + 1];";
                    sData += "\n                    memset(buf,0,sizeof(char) * total_length + 1);";
                    sData += "\n                    bind[i].buffer = buf;";
                    sData += "\n                    bind[i].buffer_length = total_length;";
                    sData += "\n                    ret = mysql_stmt_fetch_column(stmt, &bind[i], i, 0);";
                    sData += "\n                    if (0 == ret){";
                    sData += "\n                        std::vector<std::string> subValue;";
                    sData += "\n                        (void)boost::split(subValue, buf, boost::is_any_of(\"\\\\\"));";
                    sData += "\n                        std::vector<db_Point2d> vLeafPos;";
                    sData += "\n                        const size_t fSize = subValue.size()/2;//300A,00BC 101,102,,,1N,201,202,,,2N";
                    sData += "\n                        for (size_t x=0; x<fSize; ++x){";
                    sData += "\n                            double fValuex = atof(subValue[x].c_str());";
                    sData += "\n                            double fValuey = atof(subValue[fSize + x].c_str());";
                    sData += "\n                            db_Point2d pt(fValuex,fValuey);";
                    sData += "\n                            vLeafPos.push_back(pt);";
                    sData += "\n                        }";
                    sData += "\n                        pDis->set_leafpositions(vLeafPos);";
                    sData += "\n                    }";
                    sData += "\n                    DEL_ARRAY(buf);";
                    sData += "\n                }";
                }
                else if (TYPE_COMMISSIONEDUNIT == table.enTableType && "discretedoserate" == (*itr).sColumnName)
                {
                    sData += "\n                {";
                    sData += "\n                    const int total_length = length[i];";
                    sData += "\n                    if (total_length < 1) break;";
                    sData += "\n                    char *buf = new char[total_length + 1];";
                    sData += "\n                    memset(buf, 0, sizeof(char) * total_length + 1);";
                    sData += "\n                    bind[i].buffer = buf;";
                    sData += "\n                    bind[i].buffer_length = total_length;";
                    sData += "\n                    ret = mysql_stmt_fetch_column(stmt, &bind[i], i, 0);";
                    sData += "\n                    if (0 == ret)";
                    sData += "\n                    {";
                    sData += "\n                        std::vector<std::string> vecDoserateString;";
                    sData += "\n                        (void)boost::split(vecDoserateString, buf, boost::is_any_of(\",\"));";
                    sData += "\n                        std::vector<float> vecDoserateFloat;";
                    sData += "\n                        for (auto itStr=vecDoserateString.begin(); itStr!=vecDoserateString.end(); ++itStr)";
                    sData += "\n                        {";
                    sData += "\n                            float bound = (*itStr).empty() ? 0.f : (float)atof((*itStr).c_str());";
                    sData += "\n                            vecDoserateFloat.push_back(bound);";
                    sData += "\n                        }";
                    sData += "\n                        pDis->set_discretedoserate(vecDoserateFloat);";
                    sData += "\n                    }";
                    sData += "\n                    DEL_ARRAY(buf);";
                    sData += "\n                }";
                }
                else if (TYPE_MACHINE == table.enTableType && "leafboundaries" == (*itr).sColumnName){
                    sData += "\n                {";
                    sData += "\n                    const int total_length = length[i];";
                    sData += "\n                    std::vector<double> data_leafboundaries;";
                    sData += "\n                    if (total_length < 1) break;";
                    sData += "\n                    char *buf=new char[total_length + 1];";
                    sData += "\n                    memset(buf,0,sizeof(char) * total_length + 1);";
                    sData += "\n                    bind[i].buffer = buf;";
                    sData += "\n                    bind[i].buffer_length = total_length;";
                    sData += "\n                    ret = mysql_stmt_fetch_column(stmt, &bind[i], i, 0);";
                    sData += "\n                    if (0 == ret){";
                    sData += "\n                        std::vector<std::string> subValue;";
                    sData += "\n                        (void)boost::split(subValue, buf, boost::is_any_of(\"\\\\\\\\\"));";
                    sData += "\n                        const size_t fSize = subValue.size();";
                    sData += "\n                        for (size_t x=0; x<fSize; ++x){";
                    sData += "\n                            double fValue = (double)atof(subValue[x].c_str());";
                    sData += "\n                            data_leafboundaries.push_back(fValue);";
                    sData += "\n                        }";
                    sData += "\n                        pDis->set_leafboundaries(data_leafboundaries);";
                    sData += "\n                    }";
                    sData += "\n                    DEL_PTR(buf);";
                    sData += "\n                }";
                }
                else if (TYPE_CT2DENSITY == table.enTableType && "ct2densityvalue" == (*itr).sColumnName){
                    sData += "\n                {";
                    sData += "\n                    const int total_length = length[i];";
                    sData += "\n                    if (total_length < 2) break;";
                    sData += "\n                    char *buf = new char[total_length + 1];";
                    sData += "\n                    memset(buf,0,sizeof(char) * total_length + 1);";
                    sData += "\n                    bind[i].buffer = buf;";
                    sData += "\n                    bind[i].buffer_length = total_length;";
                    sData += "\n                    ret = mysql_stmt_fetch_column(stmt, &bind[i], i, 0);";
                    sData += "\n                    if (0 == ret) SetupCt2DensityMap_i(buf,pDis);";
                    sData += "\n                    DEL_ARRAY(buf);";
                    sData += "\n                }";
                }
                else if (TYPE_BLOCK == table.enTableType && "points" == (*itr).sColumnName){
                    sData += "\n                {";
                    sData += "\n                    const int total_length = length[i];";
                    sData += "\n                    if (total_length < 2) break;";
                    sData += "\n                    char *buf=new char[total_length + 1];";
                    sData += "\n                    memset(buf,0,sizeof(char) * total_length + 1);";
                    sData += "\n                    bind[i].buffer = buf;";
                    sData += "\n                    bind[i].buffer_length = total_length;";
                    sData += "\n                    ret = mysql_stmt_fetch_column(stmt, &bind[i], i, 0);";
                    sData += "\n                    if (0 == ret){";
                    sData += "\n                        std::vector<std::string> subValue;";
                    sData += "\n                        (void)boost::split(subValue, buf, boost::is_any_of(\"|\"));";
                    sData += "\n                        std::vector<db_Point2f> vPoints;";
                    sData += "\n                        const size_t fSize = subValue.size();";
                    sData += "\n                        for (size_t x=0; x<fSize; ++x){";
                    sData += "\n                            std::vector<std::string> strValue;";
                    sData += "\n                            (void)boost::split(strValue,subValue[x], boost::is_any_of(\",\"));";
                    sData += "\n                            float fValuex = static_cast<float>(atof(strValue[0].c_str()));";
                    sData += "\n                            float fValuey = static_cast<float>(atof(strValue[1].c_str()));";
                    sData += "\n                            db_Point2f pt(fValuex,fValuey);";
                    sData += "\n                            vPoints.push_back(pt);";
                    sData += "\n                        }";
                    sData += "\n                        pDis->set_points(vPoints);";
                    sData += "\n                    }";
                    sData += "\n                    DEL_ARRAY(buf);";
                    sData += "\n                }";
                }
                else if (TYPE_KERNELDATA== table.enTableType && "kerneldata"==(*itr).sColumnName)
                {
                    sData += "\n                {";
                    sData += "\n                    const int total_length = length[i];";
                    sData += "\n                    if (total_length < 1)";
                    sData += "\n                    {";
                    sData += "\n                        break;";
                    sData += "\n                    }";
                    sData += "\n                    char *buf = new char[total_length + 1];";
                    sData += "\n                    memset(buf,0,sizeof(char) * total_length + 1);";
                    sData += "\n                    bind[i].buffer = buf;";
                    sData += "\n                    bind[i].buffer_length = total_length;";
                    sData += "\n                    ret = mysql_stmt_fetch_column(stmt, &bind[i], i, 0);";
                    sData += "\n                    if (0 == ret){";
                    sData += "\n                        std::vector<std::string> subValue;";
                    sData += "\n                        (void)boost::split(subValue, buf, boost::is_any_of(\",\"));";
                    sData += "\n                        std::vector<float> vData;";
                    sData += "\n                        const size_t fSize = subValue.size();";
                    sData += "\n                        for (size_t x=0; x<fSize; ++x){";
                    sData += "\n                            vData.push_back((float)atof(subValue[x].c_str()));";
                    sData += "\n                        }";
                    sData += "\n                        pDis->set_kerneldata(vData);";
                    sData += "\n                    }";
                    sData += "\n                    DEL_ARRAY(buf);";
                    sData += "\n                }";
                }
                else
                {
                    sData += "\n                {";
                    sData += "\n                    const int total_length = length[i];";
                    sData += "\n                    if (total_length < 1) break;";
                    sData += "\n                    char *buf = new char[total_length + 1];";
                    sData += "\n                    memset(buf, 0, sizeof(char) * total_length + 1);";
                    sData += "\n                    bind[i].buffer = buf;";
                    sData += "\n                    bind[i].buffer_length = total_length;";
                    sData += "\n                    ret = mysql_stmt_fetch_column(stmt, &bind[i], i, 0);";
                    sData += "\n                    if (0 == ret) pDis->set_" + (*itr).sColumnName +"(buf);";
                    sData += "\n                    DEL_ARRAY(buf);";
                    sData += "\n                }";
                }
                sData += "\n                break;";
            }
            break;

        case _mediumblob:
            {
                sData += "\n            case " + std::to_string((long long)iCount) 
                    + ": \t//" + (*itr).sColumnName + " "+ std::string(Enum2StringDataType[(*itr).enDataType]);
                if (TYPE_RTIMAGE == table.enTableType && "pixeldata"==(*itr).sColumnName){
                    sData += "\n                {";
                    sData += "\n                    const int total_length = length[i];";
                    sData += "\n                    if (total_length < 1) break;";
                    sData += "\n                    char *buf = new char[total_length + 1];";
                    sData += "\n                    memset(buf, 0, sizeof(char) * total_length + 1);";
                    sData += "\n                    bind[i].buffer = buf;";
                    sData += "\n                    bind[i].buffer_length = total_length;";
                    sData += "\n                    ret = mysql_stmt_fetch_column(stmt, &bind[i], i, 0);";
                    sData += "\n                    if (0 == ret) pDis->set_pixel_data_buffer(buf, total_length);";
                    sData += "\n                    DEL_ARRAY(buf);";
                    sData += "\n                }";
                }

                else if (TYPE_CONTOUR == table.enTableType && "points" == (*itr).sColumnName){
                    sData += "\n                {";
                    sData += "\n                    const int total_length = length[i];";
                    sData += "\n                    if (total_length < 2){";
                    sData += "\n                        break;";
                    sData += "\n                    }";
                    sData += "\n                    char *buf = new char[total_length + 1];";
                    sData += "\n                    memset(buf,0,sizeof(char) * total_length + 1);";
                    sData += "\n                    bind[i].buffer = buf;";
                    sData += "\n                    bind[i].buffer_length = total_length;";
                    sData += "\n                    ret = mysql_stmt_fetch_column(stmt, &bind[i], i, 0);";
                    sData += "\n                    if (0 == ret){";
                    sData += "\n                        std::vector<std::string> subValue;";
                    sData += "\n                        (void)boost::split(subValue, buf, boost::is_any_of(\"\\\\\"));";
                    sData += "\n                        std::vector<db_Point3f> vPos;";
                    sData += "\n                        const size_t fSize = subValue.size();";
                    sData += "\n                        vPos.reserve(fSize / 3);";
                    sData += "\n                        db_Point3f pt;";
                    sData += "\n                        for (size_t x=0; x<fSize;){";
                    sData += "\n                            pt.x = (float)atof(subValue[x].c_str())/std::pow(10.0f,data_accuracy);";
                    sData += "\n                            pt.y = (float)atof(subValue[x+1].c_str())/std::pow(10.0f,data_accuracy);";
                    sData += "\n                            pt.z = (float)atof(subValue[x+2].c_str())/std::pow(10.0f,data_accuracy);";
                    sData += "\n                            vPos.push_back(pt);";
                    sData += "\n                            x += 3;";
                    sData += "\n                        }";
                    sData += "\n                        pDis->set_contour_points(vPos);";
                    sData += "\n                    }";
                    sData += "\n                    DEL_ARRAY(buf);";
                    sData += "\n                }";
                }
                else if ("dosegridvalue" == (*itr).sColumnName){
                    sData += "\n                {";
                    sData += "\n                    const int total_length = length[i];";
                    sData += "\n                    const int iLen = pDis->get_xcount() * pDis->get_ycount() * pDis->get_zcount();";
                    sData += "\n                    if (total_length < 1 || iLen < 1) break;";
                    sData += "\n                    char *buf = new char[total_length + 1];";
                    sData += "\n                    memset(buf,0,sizeof(char) * total_length + 1);";
                    sData += "\n                    bind[i].buffer = buf;";
                    sData += "\n                    bind[i].buffer_length = total_length;";
                    sData += "\n                    ret = mysql_stmt_fetch_column(stmt, &bind[i], i, 0);";
                    sData += "\n                    if (0 == ret){";
                    sData += "\n                        const double dAccuracy = data_accuracy;";
                    sData += "\n                        const int iBitsAllocated = data_bitsallocated;";
                    sData += "\n                        float* pBuffer = pDis->create_dosegrid_buffer();";
                    sData += "\n                        if (16 == iBitsAllocated){";
                    sData += "\n                            uint32_t* pTmp = reinterpret_cast<uint32_t*>(buf);";
                    sData += "\n                            size_t iElemSize = total_length/sizeof(uint32_t);";
                    sData += "\n                            if (iElemSize>0 && iElemSize<= iLen){";
                    sData += "\n                                float fValue = 0.0f;";
                    sData += "\n                                for (int i = 0;i< iElemSize;++i, ++pTmp){";
                    sData += "\n                                    fValue = static_cast<float>(*pTmp * dAccuracy);";
                    sData += "\n                                    pBuffer[i] = fValue;";
                    sData += "\n                                }";
                    sData += "\n                            }";
                    sData += "\n                        }";
                    sData += "\n                        else if(32 == iBitsAllocated){";
                    sData += "\n                            int* pTmp = reinterpret_cast<int*>(buf);";
                    sData += "\n                            size_t iElemSize = total_length/sizeof(int);";
                    sData += "\n                            if (iElemSize>0 && iElemSize<= iLen){";
                    sData += "\n                                float fValue = 0.0f;";
                    sData += "\n                                for (int i = 0;i< iElemSize;++i,++pTmp){";
                    sData += "\n                                    fValue = static_cast<float>(*pTmp * dAccuracy);";
                    sData += "\n                                    pBuffer[i] = fValue;";
                    sData += "\n                                }";
                    sData += "\n                            }";
                    sData += "\n                        }";
                    sData += "\n                        // pDis->set_dosegridvalue(pBuffer);";
                    sData += "\n                    }";
                    sData += "\n                    DEL_ARRAY(buf);";
                    sData += "\n                }";
                }
                else{
                    sData += "\n                {";
                    sData += "\n                    const int total_length = length[i];";
                    sData += "\n                    if (total_length < 1) break;";
                    sData += "\n                    char *buf = new char[total_length + 1];";
                    sData += "\n                    memset(buf, 0, sizeof(char) * total_length + 1);";
                    sData += "\n                    bind[i].buffer = buf;";
                    sData += "\n                    bind[i].buffer_length = total_length;";
                    sData += "\n                    ret = mysql_stmt_fetch_column(stmt, &bind[i], i, 0);";
                    sData += "\n                    if (0 == ret) pDis->set_" + (*itr).sColumnName +"(buf);";
                    sData += "\n                    DEL_ARRAY(buf);";
                    sData += "\n                }";
                }
                sData += "\n                break;";
            }
            break;
        case _longblob:
            {
                sData += "\n            case " + std::to_string((long long)iCount) 
                    + ": \t//" + (*itr).sColumnName + " "+ std::string(Enum2StringDataType[(*itr).enDataType]);
                if (TYPE_DOSEGRID == table.enTableType && "dosegridvalue"==(*itr).sColumnName)
                {
                    sData += "\n                {";
                    sData += "\n                    const int total_length = length[i];";
                    sData += "\n                    const int iLen = pDis->get_xcount() * pDis->get_ycount() * pDis->get_zcount();";
                    sData += "\n                    if (total_length < 1 || iLen < 1) break;";
                    sData += "\n                    char *buf = new char[total_length + 1];";
                    sData += "\n                    memset(buf,0,sizeof(char) * total_length + 1);";
                    sData += "\n                    bind[i].buffer = buf;";
                    sData += "\n                    bind[i].buffer_length = total_length;";
                    sData += "\n                    ret = mysql_stmt_fetch_column(stmt, &bind[i], i, 0);";
                    sData += "\n                    if (0 == ret){";
                    sData += "\n                        const double dAccuracy = data_accuracy;";
                    sData += "\n                        const int iBitsAllocated = data_bitsallocated;";
                    sData += "\n                        float* pBuffer = pDis->create_dosegrid_buffer();";
                    sData += "\n                        if (16 == iBitsAllocated){";
                    sData += "\n                            uint32_t* pTmp = reinterpret_cast<uint32_t*>(buf);";
                    sData += "\n                            size_t iElemSize = total_length/sizeof(uint32_t);";
                    sData += "\n                            if (iElemSize>0 && iElemSize<= iLen){";
                    sData += "\n                                float fValue = 0.0f;";
                    sData += "\n                                for (int i = 0;i< iElemSize;++i, ++pTmp){";
                    sData += "\n                                    fValue = static_cast<float>(*pTmp * dAccuracy);";
                    sData += "\n                                    pBuffer[i] = fValue;";
                    sData += "\n                                }";
                    sData += "\n                            }";
                    sData += "\n                        }";
                    sData += "\n                        else if(32 == iBitsAllocated){";
                    sData += "\n                            int* pTmp = reinterpret_cast<int*>(buf);";
                    sData += "\n                            size_t iElemSize = total_length/sizeof(int);";
                    sData += "\n                            if (iElemSize>0 && iElemSize<= iLen){";
                    sData += "\n                                float fValue = 0.0f;";
                    sData += "\n                                for (int i = 0;i< iElemSize;++i,++pTmp){";
                    sData += "\n                                    fValue = static_cast<float>(*pTmp * dAccuracy);";
                    sData += "\n                                    pBuffer[i] = fValue;";
                    sData += "\n                                }";
                    sData += "\n                            }";
                    sData += "\n                        }";
                    sData += "\n                    }";
                    sData += "\n                    DEL_ARRAY(buf);";
                    sData += "\n                }";
                    sData += "\n                break;";
                }
                else
                {
                    sData += "\n                {";
                    sData += "\n                    const unsigned long total_length = length[i];";
                    sData += "\n                    if (total_length < 1) break;";
                    sData += "\n                    char *buf = new char[total_length + 1];";
                    sData += "\n                    memset(buf, 0, sizeof(char) * total_length + 1);";
                    sData += "\n                    bind[i].buffer = buf;";
                    sData += "\n                    bind[i].buffer_length = total_length;";
                    sData += "\n                    ret = mysql_stmt_fetch_column(stmt, &bind[i], i, 0);";
                    sData += "\n                    if (0 == ret){";
                    sData += "\n                        pDis->set_data(buf, total_length);";
                    sData += "\n                    }";
                    sData += "\n                    else{";
                    sData += "\n                        DEL_ARRAY(buf);";
                    sData += "\n                    }";
                    sData += "\n                }";
                    sData += "\n                break;";
                }
            }
            break;
        case _date:
            {
                sData += "\n            case " + std::to_string((long long)iCount) + ": \t//" + (*itr).sColumnName + " "+ sDataType;
                sData += "\n                {";
                sData += "\n                    DATE_BOOST " + (*itr).sColumnName + ";";
                sData += "\n                    ConvertDateTime2Boost(data_" + (*itr).sColumnName +", &" + (*itr).sColumnName + ");";
                sData += "\n                    pDis->set_" + (*itr).sColumnName +"(" + (*itr).sColumnName + ");";
                sData += "\n                }";
                sData += "\n                break;";
                break;
            }
            break;
        case _datetime:
        case _timestamp:
            {
                sData += "\n            case " + std::to_string((long long)iCount) + ": \t//" + (*itr).sColumnName + " "+ sDataType;
                sData += "\n                {";
                sData += "\n                    DATETIME_BOOST " + (*itr).sColumnName + ";";
                sData += "\n                    ConvertDateTime2Boost(data_" + (*itr).sColumnName +", &" + (*itr).sColumnName + ");";
                sData += "\n                    pDis->set_" + (*itr).sColumnName +"(" + (*itr).sColumnName + ");";
                sData += "\n                }";
                sData += "\n                break;";
                break;
            }
            break;
        case _time:
            {
                sData += "\n            case " + std::to_string((long long)iCount) + ": \t//" + (*itr).sColumnName + " "+ sDataType;
                sData += "\n                {";
                sData += "\n                    TIME_BOOST " + (*itr).sColumnName + ";";
                sData += "\n                    ConvertDateTime2Boost(data_" + (*itr).sColumnName +", &" + (*itr).sColumnName + ");";
                sData += "\n                    pDis->set_" + (*itr).sColumnName +"(" + (*itr).sColumnName + ");";
                sData += "\n                }";
                sData += "\n                break;";
                break;
            }
            break;

        case _varchar:
        case _char:
        case _unknown:
        default:
            sData += "\n            case " + std::to_string((long long)iCount) + ": \t//" + (*itr).sColumnName + " "+ sDataType;

            if(TYPE_DOSEGRID == table.enTableType && "grid_to_pat_t" == (*itr).sColumnName) {
                sData += "\n                {";
                sData += "\n                    std::vector<std::string> subValue;";
                sData += "\n                    std::string value_matrix = std::string((char *)bind[i].buffer);";
                sData += "\n                    (void)boost::split(subValue, value_matrix, boost::is_any_of(\",\"));";
                sData += "\n                    if (subValue.size()!=16) {";
                sData += "\n                        break;";
                sData += "\n                    }";
                sData += "\n                    float matrix[16];";
                sData += "\n                    for (int j(0); j<4; ++j){";
                sData += "\n                        for (int i(0); i<4; ++i){";
                sData += "\n                            matrix[i + j*4] = subValue[i + j*4].empty()? 0.0f : (float)atof(subValue[i + j*4].c_str());";
                sData += "\n                        }";
                sData += "\n                    }";
                sData += "\n                    pDis->set_grid_to_pat_t(matrix);";
                sData += "\n                }";
                sData += "\n                break;";
            }
            else if(TYPE_BEAMSEGMENT == table.enTableType && "t_beam_to_pat" == (*itr).sColumnName) {
                sData += "\n                {";
                sData += "\n                    std::vector<std::string> subValue;";
                sData += "\n                    std::string value_matrix = std::string((char *)bind[i].buffer);";
                sData += "\n                    (void)boost::split(subValue, value_matrix, boost::is_any_of(\",\"));";
                sData += "\n                    if (subValue.size()!=16) {";
                sData += "\n                        break;";
                sData += "\n                    }";
                sData += "\n                    float matrix[16];";
                sData += "\n                    for (int j(0); j<4; ++j){";
                sData += "\n                        for (int i(0); i<4; ++i){";
                sData += "\n                            matrix[i + j*4] = subValue[i + j*4].empty()? 0.0f : (float)atof(subValue[i + j*4].c_str());";
                sData += "\n                        }";
                sData += "\n                    }";
                sData += "\n                    pDis->set_t_beam_to_pat(matrix);";
                sData += "\n                }";
                sData += "\n                break;";
            }
            else if(TYPE_IMAGETRANSFORMATION == table.enTableType && "registrationmatrix" == (*itr).sColumnName) {
                sData += "\n                {";
                sData += "\n                    std::vector<std::string> subValue;";
                sData += "\n                    std::string value_matrix = std::string((char *)bind[i].buffer);";
                sData += "\n                    (void)boost::split(subValue, value_matrix, boost::is_any_of(\",\"));";
                sData += "\n                    if (subValue.size()!=16) {";
                sData += "\n                        break;";
                sData += "\n                    }";
                sData += "\n                    float matrix[16];";
                sData += "\n                    for (int j(0); j<4; ++j){";
                sData += "\n                        for (int i(0); i<4; ++i){";
                sData += "\n                            matrix[i + j*4] = subValue[i + j*4].empty()? 0.0f : (float)atof(subValue[i + j*4].c_str());";
                sData += "\n                        }";
                sData += "\n                    }";
                sData += "\n                    pDis->set_registrationmatrix(matrix);";
                sData += "\n                }";
                sData += "\n                break;";
            }
            else if(TYPE_VOI== table.enTableType && "pat2volumematrix" == (*itr).sColumnName) {
                sData += "\n                {";
                sData += "\n                    std::vector<std::string> subValue;";
                sData += "\n                    std::string sData = std::string((char *)bind[i].buffer);";
                sData += "\n                    (void)boost::split(subValue, sData, boost::is_any_of(\",\"));";
                sData += "\n                    if (subValue.size()!=16) break;";
                sData += "\n                    float matrix[16];";
                sData += "\n                    for (int j(0); j<4; ++j){";
                sData += "\n                        for (int i(0); i<4; ++i){";
                sData += "\n                            matrix[i + j*4] = subValue[i + j*4].empty()? 0.0f : (float)atof(subValue[i + j*4].c_str());";
                sData += "\n                        }";
                sData += "\n                    }";
                sData += "\n                    pDis->set_pat2volumematrix(matrix);";
                sData += "\n                }";
                sData += "\n                break;";
            }
            else if(TYPE_VOI== table.enTableType && "interpolate" == (*itr).sColumnName) {
                sData += "\n                {";
                sData += "\n                    std::string sData = std::string((char *)bind[i].buffer);";
                sData += "\n                    if (sData.empty()) break;";
                sData += "\n                    std::vector<std::string> subValue;";
                sData += "\n                    (void)boost::split(subValue, sData, boost::is_any_of(\",\"));";
                sData += "\n                    std::vector<std::string>::iterator itsubVal = subValue.begin();";
                sData += "\n                    std::vector<bool> vecValue;";
                sData += "\n                    for (; itsubVal!=subValue.end(); itsubVal++) {";
                sData += "\n                        vecValue.push_back((*itsubVal).empty()? false : boost::lexical_cast<bool>((*itsubVal)));";
                sData += "\n                    }";
                sData += "\n                    pDis->set_interpolate(vecValue);";
                sData += "\n                }";
                sData += "\n                break;";
            }
            else{
                
                sData += "\n                pDis->set_" + (*itr).sColumnName +"((char *)bind[i].buffer);";
                sData += "\n                break;";
            }

            break;
        }
    }

    sData += "\n            default:";
    sData += "\n                break;";
    sData += "\n            }";
    sData += "\n        }";
    sData += "\n        pDis->setdirty_all_fields(false);";
    sData += "\n        vList.push_back(pDis);";
    sData += "\n    }";
    sData += "\n    for (int i=0; i<field_num; ++i) DEL_ARRAY(data_string[i]);";
    sData += "\n    DEL_ARRAY(bind);";
    sData += "\n    DEL_ARRAY(length);";
    sData += "\n    DEL_ARRAY(is_null);";
    sData += "\n";
    sData += "\n    // Free the prepared result metadata"; 
    sData += "\n    mysql_free_result(prepare_meta_result);";
    sData += "\n";
    sData += "\n    // Close the statement";
    sData += "\n    if (mysql_stmt_close(stmt))";
    sData += "\n    {";
    sData += "\n        TPS_LOG_DEV_ERROR<<\" failed while closing the statement \" << mysql_stmt_error(stmt);";
    sData += "\n        return false;";
    sData += "\n    }";
    sData += "\n";
    sData += "\n    return true;";
    sData += "\n}";

    //FetchDataFromDB end


    //////////////////////////////////////////////////////////////////////////
    //calc hash
    if (TYPE_MACHINE == table.enTableType)
    {
        rt_tps_database_object_helper_cpp_hash(table, sData);
    }

    //////////////////////////////////////////////////////////////////////////
    sData += "\n";
    sData += "\nRT_TPS_DATABASE_END_NAMESPACE";

    //file.write(sData.c_str() ,sData.length());
    //file.close();
    WriteFileInUTF8(sFileNameOutput, sData);
}

//////////////////////////////////////////////////////////////////////////
void CodeGeneraterMysql::rt_tps_database_object_helper_cpp_hash(TableItem& table, std::string& sData)
{
    std::string shortName = table.sTableName.substr(3,table.sTableName.length());
    const std::string sFileName="rt_tps_database_object_helper_" + shortName + ".cpp";
    const std::string sFileNameOutput="output//" + sFileName;


    std::string sTableNameUpper = shortName;
    transform(shortName.begin(), shortName.end(), sTableNameUpper.begin(), toupper);
    std::string sFirstUpper = shortName.substr(0,1);
    transform(sFirstUpper.begin(), sFirstUpper.end(), sFirstUpper.begin(), toupper);
    std::string sTableNameWithFirst = sFirstUpper + shortName.substr(1,shortName.length() - 1);

    const std::string sRtClassName = "Rt" + sTableNameWithFirst;
    //////////////////////////////////////////////////////////////////////////
    //void RtDatabaseHelper::CalcHash(const RtMachine& src, std::string* pHash)
    sData += "\n";
    sData += "\nvoid RtDatabaseHelper::CalcHash(const " + sRtClassName + "& src, std::string* pHash)";
    sData += "\n{";

    sData += "\n    if(nullptr == pHash) return;";
    sData += "\n";
    sData += "\n    std::string columndata(\"\");";
    sData += "\n    std::string itemdata(\"\");";
    sData += "\n    std::string columnname(\"\");";
    sData += "\n    std::string columnvalue(\"\");";

    sData += "\n";
    sData += "\n    for (unsigned int i(0); i < MACHINE_FIELD_MAX; ++i)";
    sData += "\n    {";
    sData += "\n        //columnname = std::string(ENUM2STRING_MACHINE_FIELD[i]);";
    sData += "\n        switch (i)";
    sData += "\n        {";
    for (auto itr=table.vColumnItems.cbegin(); itr!=table.vColumnItems.cend(); ++itr)
    {
        std::stringstream ssIndex;
        std::string sIndex;
        ssIndex<<( (*itr).iPosition -1 );//start from 0
        ssIndex>>sIndex;

        sData += "\n        case " + sIndex + ": \t//" + (*itr).sColumnName + " "+ std::string(Enum2StringDataType[(*itr).enDataType]);
        sData += "\n            {";

        if ((*itr).sColumnName == "crc"
            || itr->enDataType == _longblob
            || itr->enDataType == _mediumblob
            || itr->enDataType == _blob
            || itr->enDataType == _datetime
            || itr->enDataType == _date
            || itr->enDataType == _time
            || itr->enDataType == _timestamp)
        {
            sData += "\n            }";
            sData += "\n            break;";
           continue;
        }

        sData += "\n                if(!src.has_field(i))";
        sData += "\n                {";
        sData += "\n                    columnvalue = \"\";";
        sData += "\n                }";
        sData += "\n                else";
        sData += "\n                {";

        std::string sNameToString("");
        switch(itr->enDataType)
        {
        case _int:
        case _tinyint:
        case _bigint:
            {
                sNameToString = "std::to_string((long long)src.get_" +itr->sColumnName +"())";
            }
            break;
        case _float:
        case _double:
            {
                sNameToString = "ToString(src.get_" +itr->sColumnName +"())";
            }
            break;
        case _bool:
            {
                sNameToString = "std::to_string(src.get_" +itr->sColumnName +"())";
            }
            break;
        case _datetime:
        case _timestamp:
        case _date:
        case _time:
            {
                //sNameToString = "std::to_string(ConvertDateTime(src.get_" +itr->sColumnName +"()))";
            }
            break;
        case _blob:
        case _mediumblob:
        case _longblob:
            {

            }
            break;

        case _char:
        case _varchar:
        case _unknown:
        default:
            {
                sNameToString = "src.get_" + itr->sColumnName +"()"; 
                
            }
            break;
        }

        sData += "\n                    columnvalue = " + sNameToString +";";

        if (DataType(_unknown) == itr->enDataType
            || DataType(_char) == itr->enDataType
            || DataType(_varchar) == itr->enDataType)
        {
              sData += "\n                    RemoveSlash(columnvalue);";
        }
        sData += "\n                }";

        //sData += "\n                columndata = columnname + \"=\" + columnvalue + \"\\\\\";";
        sData += "\n                columndata = columnvalue + \"|\";";
        sData += "\n                itemdata += columndata;";
        sData += "\n            }";
        sData += "\n            break;";
        /*if(table.enTableType == TYPE_VOI)
        {
            if ((*itr).sColumnName == "red")
            {
                sData += "\n                data_red = src.get_color()[0];";
                sData += "\n                bind[iCount].buffer_type = MYSQL_TYPE_FLOAT;";
                sData += "\n                bind[iCount].buffer = (void*)&data_red;";
                sData += "\n                bind[iCount].buffer_length = sizeof(float);";
                sData += "\n            }";
                sData += "\n            break;";
                continue;
            }
            else if((*itr).sColumnName == "green"){
                sData += "\n                data_green = src.get_color()[1];";
                sData += "\n                bind[iCount].buffer_type = MYSQL_TYPE_FLOAT;";
                sData += "\n                bind[iCount].buffer = (void*)&data_green;";
                sData += "\n                bind[iCount].buffer_length = sizeof(float);";
                sData += "\n            }";
                sData += "\n            break;";
                continue;
            }
        }*/
    }

    sData += "\n            default:";
    sData += "\n                break;";
    sData += "\n        }";
    sData += "\n       }";

    sData += "\n    md5_helper_t hhelper;";
    sData += "\n    std::string hash = hhelper.hexdigesttext(itemdata);";
    sData += "\n    errorinfo_t lasterror = hhelper.lasterror();";
    sData += "\n    if(0 == lasterror.errorCode ) ";
    sData += "\n    {";
    sData += "\n        *pHash = hash;";
    sData += "\n    }";
    sData += "\n    else";
    sData += "\n    {";
    sData += "\n        *pHash = \"\";";
    sData += "\n    }";
    sData += "\n}";
}

void CodeGeneraterMysql::WriteFiles(TableItem& table)
{
    rt_tps_database_interface_object_h(table);
    rt_tps_database_interface_object_cpp(table);

    //rt_tps_database_object_helper_h(table);
    rt_tps_database_object_helper_cpp(table);
}

void CodeGeneraterMysql::DoWork()
{
    //////////////////////////////////////////////////////////////////////////
    
    ShowTables();

    //re connect to DB
    if (nullptr != m_pMySql)
    {
        mysql_close(m_pMySql);
        m_pMySql = nullptr;
    }
    //mysql 
    m_sDatabaseName=INFORMATION_SCHEMA;
    //test connect to DB
    if (!create_database (m_sUser, m_sPassword,m_sDatabaseName,m_sHostName,m_uiPort,m_pMySql)){
        printf("RtDatabaseWrapper has not been initialized probably caused by wrong config or mysql dead");
        return;
    } 

    for (auto itr=vTables.begin(); itr!=vTables.end(); ++itr)
    {
        //if ((*itr).sTableName != "tmsmachine"){//for test only
        //    continue;
        //}

        ShowColumns(*itr);

        //convert item data type
        ConvertDataType2Cpp(*itr);

        WriteFiles(*itr);
    }

    WriteCommonInfo();
    rt_tps_database_object_helper_h();
    //write files;

    //////////////////////////////////////////////////////////////////////////
    //test blob

    //delete old data
    
    //char* insertSQL="delete from tmstestblob";
    //this->MysqlRealQuery(insertSQL);
    //const int iCount = 10000;
    //for (int i=0; i<iCount; ++i){
    //    TestInsertTable(i);
    //}

    //char* insertSqlFile="delete from tmstestfile";
    //this->MysqlRealQuery(insertSqlFile);
    //for (int i=0; i<iCount; ++i){
    //    TestInsertTableWithFile(i);
    //}

    //TestQueryTable();
    //TestQueryTableWithFile();
}

void CodeGeneraterMysql::TearDown()
{ 
    if (nullptr != m_pMySql){
        mysql_close(m_pMySql);
        m_pMySql = nullptr;
    }
}

std::string CodeGeneraterMysql::GetCurrentData(void) const
{
    time_t t_gm;
    time(&t_gm);

    // 标准时间转换到本地时间数字
    tm tm_local;
    localtime_s(&tm_local, &t_gm);
    //printf("本地时间:%04d-%02d-%02d:%02d.%02d.%02d\n", tm_local.tm_year+1900, tm_local.tm_mon+1, tm_local.tm_mday, tm_local.tm_hour, tm_local.tm_min, tm_local.tm_sec);

    // 转换时间格式
    tm tm_utc;
    gmtime_s(&tm_utc, &t_gm);

    std::string strDate("");

    std::stringstream ssM,ssD,ssY;
    std::string strNum;

    ssM<<tm_utc.tm_mon+1;
    ssM>>strNum;
    strDate = strNum+"/";

    ssD<<tm_utc.tm_mday;
    ssD>>strNum;
    strDate += strNum+"/";

    ssY<<tm_utc.tm_year+1900;
    ssY>>strNum;
    strDate += strNum+"";

    return strDate;
}


//////////////////////////////////////////////////////////////////////////
void CodeGeneraterMysql::TestInsertTable(const int iIndex)
{
    MYSQL_STMT    *stmt;    
    MYSQL_BIND    bind[3];   
    memset(bind,0,sizeof(bind));
    stmt = mysql_stmt_init(m_pMySql);  
    char* insertSQL="INSERT INTO tmstestblob VALUES(?, ?, ?)";
    if (mysql_stmt_prepare(stmt, insertSQL, (unsigned long)strlen(insertSQL)))    
    {    
        printf("mysql_stmt_prepare(), INSERT failed,%s\r\n",mysql_error(m_pMySql)); 
        return;    
    }    

    // int to string
    std::stringstream ss;
    std::string sUid;
    ss<<iIndex;
    ss>>sUid;

    bind[0].buffer_type= MYSQL_TYPE_VAR_STRING;    
    bind[0].buffer= (void*)sUid.c_str();    
    bind[0].buffer_length= (unsigned long)strlen(sUid.c_str()); 

    int iCount = iIndex;
    char buff[4];
    memcpy(buff,&iCount,4);

    bind[1].buffer_type= MYSQL_TYPE_LONG;    
    bind[1].buffer= buff;    
    bind[1].buffer_length= sizeof(int);

    //create buffer
    const int iSize = 1000;
    char* pBuffer = new char[iSize*4];
    memset(pBuffer, 0, iSize*4*sizeof(char));
    float fValue = 1.0;
    for (int i=0; i<iSize; ++i){
        sprintf_s(pBuffer + i*4, sizeof(float), "%f", fValue);
        fValue += 1.0;
    }

    bind[2].buffer_type= MYSQL_TYPE_BLOB;    
    bind[2].buffer= pBuffer;    
    bind[2].buffer_length= iSize*4*sizeof(char);


    if (mysql_stmt_bind_param(stmt, bind))    
    {    
        printf("mysql_stmt_bind_param() failed %s\r\n", mysql_stmt_error(stmt));    
        return;
    }    

    if (mysql_stmt_execute(stmt))
    {    
        printf( " mysql_stmt_execute(), failed %s\r\n", mysql_stmt_error(stmt));    
        return ;   
    }    
    mysql_stmt_close(stmt); 

    for (int x=0; x<10; ++x){
        char str[4]= {};
        memcpy(str, pBuffer + x*4, 4*sizeof(char));
        float fValue=(float)atof(str);
        printf("data[%d] \t%f\n", x, fValue);
    }
}

void CodeGeneraterMysql::TestInsertTableWithFile(const int iIndex)
{
    MYSQL_STMT    *stmt;    
    MYSQL_BIND    bind[3];   
    memset(bind,0,sizeof(bind));
    stmt = mysql_stmt_init(m_pMySql);  
    char* insertSQL="INSERT INTO tmstestfile VALUES(?, ?, ?)";
    if (mysql_stmt_prepare(stmt, insertSQL, (unsigned long)strlen(insertSQL)))    
    {    
        printf("mysql_stmt_prepare(), INSERT failed,%s\r\n",mysql_error(m_pMySql)); 
        return;    
    }    

    // int to string
    std::stringstream ss;
    std::string sUid;
    ss<<iIndex;
    ss>>sUid;

    bind[0].buffer_type= MYSQL_TYPE_VAR_STRING;    
    bind[0].buffer= (void*)sUid.c_str();    
    bind[0].buffer_length= (unsigned long)strlen(sUid.c_str()); 

    int iCount = iIndex;
    char buff[4];
    memcpy(buff,&iCount,4);

    bind[1].buffer_type= MYSQL_TYPE_LONG;    
    bind[1].buffer= buff;    
    bind[1].buffer_length= sizeof(int);

    const std::string sFilePath = "D:/temp/" + sUid;
    bind[2].buffer_type= MYSQL_TYPE_VAR_STRING;    
    bind[2].buffer= (void*)sFilePath.c_str();      
    bind[2].buffer_length= (unsigned long)strlen(sFilePath.c_str()); 

    if (mysql_stmt_bind_param(stmt, bind))    
    {    
        printf("mysql_stmt_bind_param() failed %s\r\n", mysql_stmt_error(stmt));    
        return;
    }    

    if (mysql_stmt_execute(stmt))
    {    
        printf( " mysql_stmt_execute(), failed %s\r\n", mysql_stmt_error(stmt));    
        return ;   
    }    

    //write file 
    {
        //create buffer
        const int iSize = 1000;
        const int iFloatSize = iSize * 4;
        char* pBuffer = new char[iFloatSize + 50];
        memset(pBuffer, 0, (iFloatSize +50)*sizeof(char));
        float fValue = 1.0;
        for (int i=0; i<iSize; ++i){
            sprintf_s(pBuffer + i*4, sizeof(float),"%f", fValue);
            fValue += 1.0;
        }
        
        std::ofstream out(sFilePath, std::ios::out|std::ios::binary);  
        out.write(pBuffer, iFloatSize);
        out.close(); 

        delete[] pBuffer;
    }

    mysql_stmt_close(stmt); 

    //for (int x=0; x<10; ++x){
    //    char str[4]= {};
    //    memcpy(str, pBuffer + x*4, 4*sizeof(char));
    //    float fValue=atof(str);
    //    //printf("data[%d] \t%f\n", x, fValue);
    //}


}

void CodeGeneraterMysql::TestQueryTable()
{
    TEST_PERFORMANCE_INIT;
    TEST_PERFORMANCE_BEGIN;

#define STRING_SIZE 64

#define SELECT_SAMPLE "SELECT uid, count,data FROM tmstestblob"    //uid, count, data

    MYSQL_STMT    *stmt;
    MYSQL_BIND    bind[3];
    memset(bind,0,sizeof(bind));
    //printf("%d\n",sizeof(bind));
    MYSQL_RES     *prepare_meta_result;

    unsigned long length[3];
    int           param_count, column_count, row_count;
//    short         small_data;
    int           int_data;
    char          str_data[STRING_SIZE];
    my_bool       is_null[3];

    /* Prepare a SELECT query to fetch data from test_table */
    stmt = mysql_stmt_init(m_pMySql);
    if (!stmt)
    {
        fprintf(stderr, " mysql_stmt_init(), out of memory/n");
        exit(0);
    }
    if (mysql_stmt_prepare(stmt, SELECT_SAMPLE, (unsigned long)strlen(SELECT_SAMPLE)))
    {
        fprintf(stderr, " mysql_stmt_prepare(), SELECT failed/n");
        fprintf(stderr, " %s/n", mysql_stmt_error(stmt));
        exit(0);
    }
    //fprintf(stdout, " prepare, SELECT successful/n");

    /* Get the parameter count from the statement */
    param_count= mysql_stmt_param_count(stmt);
    //fprintf(stdout, " total parameters in SELECT: %d/n", param_count);

    if (param_count != 0) /* validate parameter count */
    {
        fprintf(stderr, " invalid parameter count returned by MySQL/n");
        exit(0);
    }

    /* Fetch result set meta information */
    prepare_meta_result = mysql_stmt_result_metadata(stmt);
    if (!prepare_meta_result)
    {
        fprintf(stderr,
            " mysql_stmt_result_metadata(), returned no meta information/n");
        fprintf(stderr, " %s/n", mysql_stmt_error(stmt));
        exit(0);
    }

    /* Get total columns in the query */
    column_count= mysql_num_fields(prepare_meta_result);
   // fprintf(stdout, " total columns in SELECT statement: %d/n", column_count);

    //if (column_count != 3) /* validate column count */
    //{
    //    fprintf(stderr, " invalid column count returned by MySQL/n");
    //    exit(0);
    //}

    /* Execute the SELECT query */
    if (mysql_stmt_execute(stmt))
    {
        fprintf(stderr, " mysql_stmt_execute(), failed/n");
        fprintf(stderr, " %s/n", mysql_stmt_error(stmt));
        exit(0);
    }

    /* Bind the result buffers for all 4 columns before fetching them */
    memset(bind, 0, sizeof(MYSQL_BIND)*column_count);

    /* STRING COLUMN */
    bind[0].buffer_type= MYSQL_TYPE_STRING;
    bind[0].buffer= (char *)str_data;
    bind[0].buffer_length= STRING_SIZE;
    bind[0].is_null= &is_null[0];
    bind[0].length= &length[0];

    /* INTEGER COLUMN */
    bind[1].buffer_type= MYSQL_TYPE_LONG;//MYSQL_TYPE_LONG;
    bind[1].buffer= (char *)&int_data;
    bind[1].is_null= &is_null[1];
    bind[1].length= &length[1];

    /* BLOB COLUMN */
    unsigned int total_length = 0;//1000*4;
    //char *buf=new char[total_length];
    //memset(buf,0,sizeof(char)*total_length);

    bind[2].buffer_type= MYSQL_TYPE_BLOB;
    //bind[2].buffer= (char *)buf;
    bind[2].is_null= &is_null[2];
    bind[2].length = &length[2];

    /* Bind the result buffers */
    if (mysql_stmt_bind_result(stmt, bind))
    {
        fprintf(stderr, " mysql_stmt_bind_result() failed\n");
        fprintf(stderr, " %s/n", mysql_stmt_error(stmt));
        exit(0);
    }

    /* Now buffer all results to client */
    if (mysql_stmt_store_result(stmt))
    {
        fprintf(stderr, " mysql_stmt_store_result() failed/n");
        fprintf(stderr, " %s/n", mysql_stmt_error(stmt));
        exit(0);
    }

    /* Fetch all rows */
    row_count= 0;
    //fprintf(stdout, "Fetching results .../n");
    //while (!mysql_stmt_fetch(stmt)){
    for (;;)
    {
        int ret = mysql_stmt_fetch(stmt);
        if (ret!=0 && ret!=MYSQL_DATA_TRUNCATED) break;
    
        row_count++;
        //fprintf(stdout, "  row %d\n", row_count);

        /* column 0 */
        //fprintf(stdout, "   column2 (string)   : ");
        if (is_null[0])
            printf(" NULL\n");
        else
           // printf(" %s(%ld)\n", str_data, length[0]);

        /* column 1 */
        //fprintf(stdout, "   column1 (integer)  : ");
        if (is_null[1])
            printf(" NULL\n");
        else
           // printf(" %d(%ld)\n", int_data, length[1]);

        /* column 2 */
       // printf("   column3 (blob) : ");
        if (is_null[2])
        {
            printf( " NULL\n");
        }
        else
        {
            int start = 0;
            total_length = length[2];
            if (total_length < 1){
                continue;
            }
            char *buf=new char[total_length + 50];
            memset(buf,0,sizeof(char) * total_length +50);

            //printf("total_length=%lu\n", total_length);

            while (start<(int)total_length)
            {
                bind[2].buffer = (buf+start);
                bind[2].buffer_length = 4;  //每次读这么长
                ret = mysql_stmt_fetch_column(stmt, &bind[2], 2, start);
                if (ret!=0)   continue;;
                start += bind[2].buffer_length;
            }
            //////////////////////////////////////////////////////////////////////////
            //printf("data buffer length: %ld\n", length[2]);
            int fSize = total_length/4;
            for (int x=0; x<fSize; ++x){
                char str[4]= {};
                memcpy(str, buf + x*4, 4*sizeof(char));
                float fValue=(float)atof(str);
                fValue;
                //printf("data[%d] \t%f\n", x, fValue);
            }
            delete[] buf;
            buf = nullptr;
        }
    }

    /* Validate rows fetched */
   // fprintf(stdout, " total rows fetched: %d/n", row_count);
    //if (row_count != 2)
    //{
    //    fprintf(stderr, " MySQL failed to return all rows/n");
    //    exit(0);
    //}

    /* Free the prepared result metadata */
    mysql_free_result(prepare_meta_result);


    /* Close the statement */
    if (mysql_stmt_close(stmt))
    {
        fprintf(stderr, " failed while closing the statement/n");
        fprintf(stderr, " %s/n", mysql_stmt_error(stmt));
        exit(0);
    }

    int iCount = 10000;
    TEST_PERFORMANCE_END_COUNT("TestQueryTable using blob", iCount);
}

void CodeGeneraterMysql::TestQueryTableWithFile()
{
    TEST_PERFORMANCE_INIT;
    TEST_PERFORMANCE_BEGIN;

#define STRING_SIZE 64    
#define FILEPATH_SIZE 1024

#define SELECT_SAMPLE1 "SELECT uid, count,filepath FROM tmstestfile"    //uid, count, data

    MYSQL_STMT    *stmt;
    MYSQL_BIND    bind[3];
    memset(bind,0,sizeof(bind));
    //printf("%d\n",sizeof(bind));
    MYSQL_RES     *prepare_meta_result;

    unsigned long length[3];
    int           param_count, column_count, row_count;
//    short         small_data;
    int           int_data;
    char          str_data[STRING_SIZE];
    char          data_filepath[FILEPATH_SIZE];
    my_bool       is_null[3];

    /* Prepare a SELECT query to fetch data from test_table */
    stmt = mysql_stmt_init(m_pMySql);
    if (!stmt)
    {
        fprintf(stderr, " mysql_stmt_init(), out of memory/n");
        exit(0);
    }
    if (mysql_stmt_prepare(stmt, SELECT_SAMPLE, (unsigned long)strlen(SELECT_SAMPLE1)))
    {
        fprintf(stderr, " mysql_stmt_prepare(), SELECT failed/n");
        fprintf(stderr, " %s/n", mysql_stmt_error(stmt));
        exit(0);
    }
    //fprintf(stdout, " prepare, SELECT successful/n");

    /* Get the parameter count from the statement */
    param_count= mysql_stmt_param_count(stmt);
    //fprintf(stdout, " total parameters in SELECT: %d/n", param_count);

    if (param_count != 0) /* validate parameter count */
    {
        fprintf(stderr, " invalid parameter count returned by MySQL/n");
        exit(0);
    }

    /* Fetch result set meta information */
    prepare_meta_result = mysql_stmt_result_metadata(stmt);
    if (!prepare_meta_result)
    {
        fprintf(stderr,
            " mysql_stmt_result_metadata(), returned no meta information/n");
        fprintf(stderr, " %s/n", mysql_stmt_error(stmt));
        exit(0);
    }

    /* Get total columns in the query */
    column_count= mysql_num_fields(prepare_meta_result);
    //fprintf(stdout, " total columns in SELECT statement: %d/n", column_count);

    //if (column_count != 3) /* validate column count */
    //{
    //    fprintf(stderr, " invalid column count returned by MySQL/n");
    //    exit(0);
    //}

    /* Execute the SELECT query */
    if (mysql_stmt_execute(stmt))
    {
        fprintf(stderr, " mysql_stmt_execute(), failed/n");
        fprintf(stderr, " %s/n", mysql_stmt_error(stmt));
        exit(0);
    }

    /* Bind the result buffers for all 4 columns before fetching them */
    memset(bind, 0, sizeof(MYSQL_BIND)*column_count);

    /* STRING COLUMN */
    bind[0].buffer_type= MYSQL_TYPE_STRING;
    bind[0].buffer= (char *)str_data;
    bind[0].buffer_length= STRING_SIZE;
    bind[0].is_null= &is_null[0];
    bind[0].length= &length[0];

    /* INTEGER COLUMN */
    bind[1].buffer_type= MYSQL_TYPE_LONG;//MYSQL_TYPE_LONG;
    bind[1].buffer= (char *)&int_data;
    //bind[1].buffer_length= STRING_SIZE;
    bind[1].is_null= &is_null[1];
    bind[1].length= &length[1];

    /* BLOB COLUMN */
//    unsigned int total_length = 0;//1000*4;
    //char *buf=new char[total_length];
    //memset(buf,0,sizeof(char)*total_length);

    bind[2].buffer_type= MYSQL_TYPE_STRING;
    bind[2].buffer= (char *)data_filepath;
    bind[2].buffer_length= FILEPATH_SIZE;
    bind[2].is_null= &is_null[2];
    bind[2].length= &length[2];

    /* Bind the result buffers */
    if (mysql_stmt_bind_result(stmt, bind))
    {
        fprintf(stderr, " mysql_stmt_bind_result() failed\n");
        fprintf(stderr, " %s/n", mysql_stmt_error(stmt));
        exit(0);
    }

    /* Now buffer all results to client */
    if (mysql_stmt_store_result(stmt))
    {
        fprintf(stderr, " mysql_stmt_store_result() failed/n");
        fprintf(stderr, " %s/n", mysql_stmt_error(stmt));
        exit(0);
    }

    /* Fetch all rows */
    row_count= 0;
    //fprintf(stdout, "Fetching results .../n");
    //while (!mysql_stmt_fetch(stmt)){
    for (;;)
    {
        int ret = mysql_stmt_fetch(stmt);
        if (ret!=0 && ret!=MYSQL_DATA_TRUNCATED) break;

        row_count++;
        //fprintf(stdout, "  row %d\n", row_count);

        /* column 0 */
        //fprintf(stdout, "   column2 (string)   : ");
        if (is_null[0])
            printf(" NULL\n");
        else
            // printf(" %s(%ld)\n", str_data, length[0]);

            /* column 1 */
            //fprintf(stdout, "   column1 (integer)  : ");
            if (is_null[1])
                printf(" NULL\n");
            else
                // printf(" %d(%ld)\n", int_data, length[1]);

                /* column 2 */
                // printf("   column3 (blob) : ");
                if (is_null[2])
                {
                    printf( " NULL\n");
                }
                else
                {
                    //read file
                    {
                        char * buffer;
                        long size;  
                        std::ifstream in (data_filepath, std::ios::in|std::ios::binary|std::ios::ate);  
                        size = (long)in.tellg();  
                        in.seekg (0, std::ios::beg);  
                        buffer = new char [size];  
                        in.read (buffer, size);  
                        in.close();  

                        int fSize = size/4;
                        for (int x(0); x<fSize; ++x){
                            char str[4]= {};
                            memcpy(str, buffer + x*4, 4*sizeof(char));
                            float fValue=(float)atof(str);
                            fValue;
                        }
                        delete[] buffer;
                        buffer = nullptr;
                    }
                }
    }

    /* Validate rows fetched */
    //fprintf(stdout, " total rows fetched: %d/n", row_count);
    //if (row_count != 2)
    //{
    //    fprintf(stderr, " MySQL failed to return all rows/n");
    //    exit(0);
    //}

    /* Free the prepared result metadata */
    mysql_free_result(prepare_meta_result);

    /* Close the statement */
    if (mysql_stmt_close(stmt))
    {
        fprintf(stderr, " failed while closing the statement/n");
        fprintf(stderr, " %s/n", mysql_stmt_error(stmt));
        exit(0);
    }

    
    int iCount = 10000;

    TEST_PERFORMANCE_END_COUNT("TestQueryTable using file", iCount);
}
/*
void CodeGeneraterMysql::TestQueryTable()
{

    MYSQL_STMT* stmt = mysql_stmt_init(m_pMySql);
    if(NULL==stmt) return;

    const char* sql = "SELECT * FROM tmstestblob";

    int sql_len = strlen(sql);

    int ret = mysql_stmt_prepare(stmt, sql, sql_len);
    if(0!=ret) return; 
    // ERR_LOG("param count:%d", (int)mysql_stmt_param_count(stmt));

    MYSQL_BIND result[3];   
    memset(result,0,sizeof(result));

    unsigned long total_length = 0;

    result[0].buffer_type= MYSQL_TYPE_VAR_STRING;    
    //result[0].buffer= "1.1";    
    result[0].buffer_length= total_length; 

    //int iCount = 1;
    //char buff[4];
    //memcpy(buff,&iCount,4);

    //result[1].buffer_type= MYSQL_TYPE_SHORT;    
    //result[1].buffer= buff;    
    //result[1].buffer_length= sizeof(int);

    //result[2].buffer_type= MYSQL_TYPE_BLOB;    
    //result[2].buffer= pBuffer;    
    //result[2].buffer_length= iSize*4*sizeof(char);


    ret = mysql_stmt_bind_result(stmt, &result[0]);
    if(0!=ret) return;

    //     result.buffer_type = MYSQL_TYPE_BLOB; 

    ret = mysql_stmt_execute(stmt);
    if(0!=ret) return;

    ret = mysql_stmt_store_result(stmt);

    if(0!=ret) return;

    //while (mysql_stmt_fetch(stmt)!=0)

    for (;;)
    {

        ret = mysql_stmt_fetch(stmt);

        if (ret!=0 && ret!=MYSQL_DATA_TRUNCATED) break;

        int start = 0;

        char *buf=new char[total_length];
        memset(buf,0,sizeof(buf));

        printf("total_length=%lu\n", total_length);

        while (start<(int)total_length)

        {

            result[0].buffer = (buf+start);

            result[0].buffer_length = 3;  //每次读这么长

            ret = mysql_stmt_fetch_column(stmt, &result[0], 0, start);

            if (ret!=0)
            {
                return;
            }

            start += result[0].buffer_length;

        }
        //CFile f;
        //if(f.Open("e:\\aaa.7z", CFile::modeCreate|CFile::modeWrite | CFile::typeBinary))
        //{
        //    f.WriteHuge(buf,total_length);
        //    f.Close();
        //}
    }

    mysql_stmt_close(stmt);
}

*/

std::string CodeGeneraterMysql::GetDatabaseVersion() const
{ 
    std::string DBVersion("");
    std::string sSQL = GET_DBVERSION;

    if(!MysqlRealQuery(sSQL)){
        printf("%s\n", sSQL);
        return DBVersion;
    }

    MYSQL_RES *myquery = mysql_store_result(m_pMySql);
    if (nullptr == myquery){
        return DBVersion;
    }

    MYSQL_ROW row = nullptr;
    const unsigned int num_fields = mysql_num_fields(myquery);

    if (num_fields >0 ){
        if(nullptr != (row = mysql_fetch_row(myquery))){
            DBVersion = row[0] ? row[0] : "";
        }
    }
    //mysql_free_result
    if (nullptr != myquery){
        mysql_free_result(myquery);
        myquery = nullptr;
    }

    return DBVersion;
}

/* old version not supported blob
void CodeGeneraterMysql::rt_tps_database_object_helper_cpp(TableItem& table)
{
    //write rt_tps_database_object_helper_xxx.cpp
    std::ofstream file;
    std::string shortName = table.sTableName.substr(3,table.sTableName.length());
    const std::string sFileName="rt_tps_database_object_helper_" + shortName + ".cpp";
    const std::string sFileNameOutput="output//" + sFileName;
    file.open(sFileNameOutput);

    std::string sData("");
    sData += "//////////////////////////////////////////////////////////////////////////";
    sData += "\n/// \\defgroup Radio Therapy Business Unit";
    sData += "\n///  Copyright, (c) Shanghai United Imaging Healthcare Inc., 2016";
    sData += "\n///  All rights reserved.";
    sData += "\n///";
    sData += "\n///  \\author  ZHOU qiangqiang  mailto:qiangqiang.zhou@united-imaging.com";
    sData += "\n///";
    sData += "\n///  \\file      " + sFileName;
    sData += "\n///  \\brief     This file was generated by CodeGenerater.exe ";
    sData += "\n///             From database version: " + m_DatabaseVersion;
    sData += "\n///";
    sData += "\n///  \\version 1.0";
    sData += "\n///  \\date    " + m_sCurrentDate;
    sData += "\n///  \\{";
    sData += "\n//////////////////////////////////////////////////////////////////////////";

    std::string sTableNameUpper = shortName;
    transform(shortName.begin(), shortName.end(), sTableNameUpper.begin(), toupper);
    std::string sFirstUpper = shortName.substr(0,1);
    transform(sFirstUpper.begin(), sFirstUpper.end(), sFirstUpper.begin(), toupper);
    std::string sTableNameWithFirst = sFirstUpper + shortName.substr(1,shortName.length() - 1);

    const std::string sClassName = "Rt" + sTableNameWithFirst;

    std::stringstream ss;
    std::string strNum;
    ss<<table.vColumnItems.size();
    ss>>strNum;

    sData += "\n";
    sData += "\n#include \"StdAfx.h\"";
    sData += "\n#include \"rt_tps_database_object_helper_" + shortName + ".h\"";
    sData += "\n#include \"rt_tps_database_wrapper_utils.h\"";
    if (table.bHasDateTime){
        sData += "\n#include \"boost/date_time/posix_time/time_parsers.hpp\"";
    }
    if(shortName=="mlcshape"){
        sData += "\n#include \"boost/algorithm/string/split.hpp\"";
        sData += "\n#include \"boost/algorithm/string/classification.hpp\"";
    }
    sData += "\n#ifdef _WIN32// For windows";
    sData += "\n#  include <winsock2.h>";
    sData += "\n#endif";
    sData += "\n#include \"mysql.h\"";

    sData += "\n";
    sData += "\nRT_TPS_DATABASE_BEGIN_NAMESPACE;";

    sData += "\n";
    sData += "\nvoid " + sClassName + "HelperApi::CopyObjectFromTable(const MYSQL_ROW& row, unsigned int iNumFields, " + sClassName + "* pDis)";
    sData += "\n{";
    sData += "\n    if (nullptr == pDis) return;";
    sData += "\n    std::string sValue(\"\");";

    if(shortName=="beam"){
        sData += "\n        float color[4]; memset(color, 0, sizeof(float)*4);";
    }

    sData += "\n    //" + strNum + " parameters";
    sData += "\n    for(unsigned int i(0); i < iNumFields; ++i){";

    sData += "\n";
    sData += "\n        if (nullptr == row[i]) continue;";
    sData += "\n        sValue = row[i];";

    sData += "\n";
    sData += "\n        switch (i){";

    for (auto itr=table.vColumnItems.cbegin(); itr!=table.vColumnItems.cend(); ++itr){
        std::stringstream ssIndex;
        std::string sIndex;
        ssIndex<<( (*itr).iPosition -1 );//start from 0
        ssIndex>>sIndex;
        sData += "\n        case " + sIndex + ": \t//" + (*itr).sColumnName;
        sData += "\n                \t//" + std::string(Enum2StringDataType[(*itr).enDataType]);

        if(shortName=="beam"){
            if ((*itr).sColumnName == "red"){
                sData += "\n            if(!sValue.empty()) color[0] = sValue.empty()? 0.0f : boost::lexical_cast<float>(sValue);";
                sData += "\n            pDis->set_beamcolor(color);";
                sData += "\n            break;";
                continue;
            }
            else if((*itr).sColumnName == "green"){
                sData += "\n            if(!sValue.empty()) color[1] = sValue.empty()? 0.0f : boost::lexical_cast<float>(sValue);";
                sData += "\n            pDis->set_beamcolor(color);";
                sData += "\n            break;";
                continue;
            }
            else if((*itr).sColumnName == "blue"){
                sData += "\n            if(!sValue.empty()) color[2] = sValue.empty()? 0.0f : boost::lexical_cast<float>(sValue);";
                sData += "\n            pDis->set_beamcolor(color);";
                sData += "\n            break;";
                continue;
            }
            else if((*itr).sColumnName == "alpha"){
                sData += "\n            if(!sValue.empty()) color[3] = sValue.empty()? 0.0f : boost::lexical_cast<float>(sValue);";
                sData += "\n            pDis->set_beamcolor(color);";
                sData += "\n            break;";
                continue;
            }
        }

        switch((*itr).enDataType){

        case _int:
            {  
                sData += "\n            pDis->set_" + (*itr).sColumnName + "(sValue.empty()? 0 : boost::lexical_cast<int>(sValue));";
            }
            break;

        case _bigint:
            {  
                sData += "\n            pDis->set_" + (*itr).sColumnName + "(sValue.empty()? 0 : boost::lexical_cast<long long>(sValue));";
            }
            break;

        case _float:
            {  
                sData += "\n            pDis->set_" + (*itr).sColumnName + "(sValue.empty()? 0.f : boost::lexical_cast<float>(sValue));";
            }
            break;

        case _double:
            {  
                sData += "\n            pDis->set_" + (*itr).sColumnName + "(sValue.empty()? 0 : boost::lexical_cast<double>(sValue));";
            }
            break;

        case _bool:
        case _tinyint:
            {
                sData += "\n            pDis->set_" + (*itr).sColumnName + "(sValue.empty()? false : boost::lexical_cast<bool>(sValue));";
            }
            break;

        case _datetime:
            {
                sData += "\n            if(sValue.length()>18){";
                sData += "\n                pDis->set_" + (*itr).sColumnName + "(boost::posix_time::time_from_string(sValue));";
                sData += "\n            }";
            }
            break;

        case _date:
            {
                sData += "\n            if(sValue.length()>9){";
                sData += "\n                pDis->set_" + (*itr).sColumnName + "(boost::gregorian::from_simple_string(sValue));";
                sData += "\n            }";
            }
            break;

        case _time:
            {
                sData += "\n            if(sValue.length()>7){";
                sData += "\n                pDis->set_" + (*itr).sColumnName + "(boost::posix_time::duration_from_string(sValue));";
                sData += "\n            }";
            }
            break;

        case _varchar:
        case _char:
        case _unknown:
        default:
            sData += "\n            pDis->set_" + (*itr).sColumnName + "(sValue);";
            break;
        }

        sData += "\n            break;";
    }


    sData += "\n        default:";
    sData += "\n            break;";
    sData += "\n        }";   
    sData += "\n    }";
    sData += "\n}";

    //////////////////////////////////////////////////////////////////////////
    //InsertTableFromObject

    sData += "\n";
    sData += "\nvoid " + sClassName +"HelperApi::InsertTableFromObject(const " + sClassName + "* pSrc, std::string& sSql)";
    sData += "\n{";
    sData += "\n    if (nullptr == pSrc) {";
    sData += "\n        TPS_LOG_DEV_ERROR<<\"ERROR_DB_MEMORY_ALLOCATION_FAILURE\";";
    sData += "\n        return;";
    sData += "\n    }";

    sData += "\n";
    sData += "\n    //tms" + shortName +"    " + strNum + " parameters";
    sData += "\n    sSql = INSERT_TABLE_" + sTableNameUpper +";";

    int iCount(0);
    for (auto itr=table.vColumnItems.cbegin(); itr!=table.vColumnItems.cend(); ++itr, ++iCount){
        const std::string sFirst = iCount == 0 ? "\"(" : "\",";
        sData += "\n    sSql += " + sFirst + (*itr).sColumnName + "\";" + "//" + std::string(Enum2StringDataType[(*itr).enDataType]);
    }

    //    sSQL += ")VALUES(";
    sData += "\n    sSql += \")VALUES(\";";
    sData += "\n";

    iCount = 0;
    for (auto itr=table.vColumnItems.cbegin(); itr!=table.vColumnItems.cend(); ++itr, ++iCount){

        //if(shortName=="beam"){
        //    if ((*itr).sColumnName == "red"
        //        ||(*itr).sColumnName == "green"
        //        ||(*itr).sColumnName == "blue"
        //        ||(*itr).sColumnName == "alpha"
        //        ){
        //            continue;
        //    }
        //}
        //if(shortName=="voi"){
        //    if ((*itr).sColumnName == "red"
        //        ||(*itr).sColumnName == "green"
        //        ||(*itr).sColumnName == "blue"
        //        ||(*itr).sColumnName == "alpha"
        //        ||(*itr).sColumnName == "pat2volumematrix"
        //        ||(*itr).sColumnName == "interpolate"
        //        ){
        //            continue;
        //    }
        //}
        //else if (shortName == "beamsegment"){
        //    if ((*itr).sColumnName == "t_beam_to_pat"){
        //        continue;
        //    }
        //}
        //else if (shortName == "dosegrid"){  
        //    if ((*itr).sColumnName == "grid_to_pat_t"){
        //        continue;
        //    }
        //}

        const std::string sFirst = iCount == 0 ? "\"" : "\",";

        std::string sItem="\n    sSql += " + sFirst;
        switch((*itr).enDataType){
        case _int:
        case _bigint:
        case _float:
        case _double:
        case _bool:
        case _tinyint:
            {
                sItem += "'\" + boost::lexical_cast<std::string>(pSrc->get_" + (*itr).sColumnName +"()) + \"'\";";
            }
            break;

        case _datetime:
        case _date:
        case _time:
            {
                sItem += "\" + RtObjectCommonHelperApi::ConvertDateTime(pSrc->get_" + (*itr).sColumnName +"());";
            }
            break;

        case _varchar:
        case _char:
        case _unknown:
        default:

            sItem += "'\" + RtObjectCommonHelperApi::CheckValidateString(pSrc->get_" + (*itr).sColumnName +"()) + \"'\";";
            break;
        }

        sData += sItem;
    }

    sData += "\n    sSql += \") \";";
    sData += "\n}";

    //////////////////////////////////////////////////////////////////////////
    sData += "\n";
    sData += "\nRT_TPS_DATABASE_END_NAMESPACE";

    file.write(sData.c_str() ,sData.length());
    file.close();
}
*/
