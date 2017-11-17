
## 20171117

-	dcmtk connect to pacs

echoscu.exe -d 10.6.60.20 3333 -aet zhou -aec UIHPACSSERVER

storescu.exe -d 10.6.60.20 3333 E:\share\dicom-image\1.dcm -aet zhou -aec UIHPACSSERVER

findscu.exe -v -S -aec UIHPACSSERVER -aet zhou 10.6.60.20 3333 -k QueryRetrieveLevel=STUDY -k StudyDate -k StudyDescription -k StudyInstanceUID

movescu.exe -v -d -S -aec UIHPACSSERVER -aet zhou -aem zhou --port 104 -od E:/tmp/ 10.6.60.20 3333 -k QueryRetrieveLevel=STUDY -k StudyInstanceUID=1.2.156.112605.161345103018.20160229064539.2.13428.1

104 is default

123 is OK

firewall should be disabled

DICOM医学图像处理：DCMTK的wiki资料学习之PACS调试

http://m.blog.csdn.net/zssureqh/article/details/39758969


C++ 日志库 boost::log 以及 glog 的对比

https://yq.aliyun.com/articles/2620
