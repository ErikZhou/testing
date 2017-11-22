
# 171122

std::sort

	//sort by instance number
	
	auto cmp = [](obj img1, obj img2)
	{  
		return img1.get_instancenumber() < img2.get_instancenumber();
	};
	std::sort(vImageObjList.begin(),vImageObjList.end(),cmp);
	
	
find files

http://www.boost.org/doc/libs/1_37_0/libs/filesystem/example/simple_ls.cpp

echo copy files to D:\UIH

xcopy /s /f /q /y include D:\UIH\include\external 

copy bin D:\UIH\bin

copy bin_debug D:\UIH\bin_debug

copy lib D:\UIH\lib\external

copy lib_debug D:\UIH\lib_debug\external

echo done
# 171120

Dcmtk 在 PACS 开发中的应用  

http://okaimee.blog.163.com/blog/static/17466526820130164320429/

		NumberOfRemainingSubOperations	0	unsigned short
		NumberOfCompletedSubOperations	7	unsigned short
		NumberOfFailedSubOperations	0	unsigned short



movescu.exe -v -d -S -aec UIHPACSSERVER -aet qqzhou -aem qqzhou --port 104 -od E:/tmp/ 10.6.60.20 3333 -k QueryRetrieveLevel=STUDY -k StudyInstanceUID=1.2.156.112605.161345103018.20160229064539.2.13428.1

# 171116

DICOM万能编辑工具之Sante DICOM Editor

http://www.cnblogs.com/h2zZhou/p/6291680.html

http://www.santesoft.com/downloads.html


C++ 日志库 boost::log 以及 glog 的对比

https://yq.aliyun.com/articles/2620
