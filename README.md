====TOP TODO====

Open-source radiation therapy research toolkit for 3D Slicer

## 172.93.42.13

http://slicerrt.github.io/

https://classroom.udacity.com/nanodegrees/nd009/parts/0091345400/modules/009134540075460/lessons/5402929278/concepts/54776097980923#
申城「三日谈」：言汇百家，思通以达（SDCC 2017上海站PPT集锦）

http://geek.csdn.net/news/detail/193172

https://webassembly.org/roadmap/

WebAssembly Consensus
WebAssembly CG members representing four browsers, Chrome, Edge, Firefox, and WebKit, have reached consensus that the design of the initial (MVP) WebAssembly API and binary format is complete to the extent that no further design work is possible without implementation experience and significant usage. This marks the end of the Browser Preview and signals that browsers can begin shipping WebAssembly on-by-default. From this point forward, future features will be designed to ensure backwards compatibility.
01：一文入门谷歌深度学习框架Tensorflow
http://blog.csdn.net/sinat_33761963/article/details/56286408

【一步步学OpenGL 1】-《打开一个窗口》
http://blog.csdn.net/cordova/article/details/52485909
http://ogldev.atspace.co.uk/

http://www.boost.org/doc/libs/1_51_0/doc/html/boost_asio/examples.html
====TOP TODO====


##	20170713
-	1.0 CalculateBeamMU
-	ComposeDoseGrids

-	added new interfaces to main
http://www.uthgsbsmedphys.org/RadOncRes/11a-MonitorUnitCalculations-1.pdf
Commissioning a Proton Therapy Machine and TPS
http://chapter.aapm.org/GLC/media/2012/Park.pdf
##	20170712
-	dose rescale and TPS performance meeting
-	new interfaces

    int CalculateBeamMU ComposeDoseGrids
-	branch implementation
##	20170711	week started
-	PB algorithm
-	inverse plan of TPS
-	prepare for ODE planning
-	matlab TPS
-	PB algorithm
-	Dose Manager
-	Clear Dose Grid from 204ms to 74ms

-	rescale issue
-	Dose Manager
##	20170710
-	Dose Manager

-	Dose Manager
-	Clear Dose Grid from 204ms to 74ms

Beam and dose modelling in TPS
http://amos3.aapm.org/abstracts/pdf/77-22581-312436-91356.pdf

##	20170707
-	Dose Manager

-	Dose Manager
##	20170706
-	PB algorithm
-	prepare for ODE planning

-	Dose Manager
*	ALG 内部重构
*	ALG + TPS接口修改
*
E:\share\Software\cuda_5.5.20_winvista_win7_win8_general_64>
setup.exe -log:"C:\path\that\exists" -loglevel:6

## 20170706
# 麦克风没声音
http://product.pconline.com.cn/itbk/software/win7/1405/4715616.html

## 20170626
放射治疗计划设计进入人工智能时代
http://www.medsoso.cn/article/402.html
https://www.varian.com/sites/default/files/resource_attachments/CN_ARIA_Brochure.pdf

TCP Servers:
Offloading TCP Processing in Internet Servers.
Design, Implementation, and Performance
http://www.cs.ucr.edu/~bhuyan/CS260/LECTURE8.pdf

template <class Class, typename ReturnType, typename Parameter>
        class SingularCallBack
        {
        public:

            typedef ReturnType (Class::*Method)(Parameter);

            SingularCallBack(Class* class_instance, Method method)
                : class_instance_(class_instance),
                method_(method)
            {}

            ReturnType operator()(Parameter parameter)
            {
                return (class_instance_->*method_)(parameter);
            }

            ReturnType execute(Parameter parameter)
            {
                return operator()(parameter);
            }

        private:

            Class* class_instance_;
            Method method_;
        };

## 20170623
#TCP&UDP压力测试工具
https://my.oschina.net/ikende/blog/204874

## 20170619

#前沿技术资料百度云链接(资料共享)
http://blog.csdn.net/fjssharpsword/article/details/53155634?locationNum=14&fps=1

## 20170614

# boost::split performance issue
# call back
http://partow.net/programming/templatecallback/

http://www.tedfelix.com/software/c++-callbacks.html

https://stackoverflow.com/questions/2298242/callback-functions-in-c

## 20170609
# if defined(_WIN32)
#define DECLARE_APPSERVER(CLASS_NAME) \
    extern "C" \
{ \
    __declspec(dllexport) tps::server::IRtAppServer* getAppServerInstance(); \
};

#define IMPLEMENT_APPSERVER(CLASS_NAME) \
    __declspec(dllexport) tps::server::IRtAppServer* getAppServerInstance() \
{\
    tps::server::IRtAppServer* pApp = static_cast<tps::server::IRtAppServer*>(new CLASS_NAME##()); \
    return pApp; \
}

# endif

## 20170608
https://www.petri.com/disable_uac_in_windows_vista

## 20170605

# This file requires _WIN32_WINNT to be #defined at least to 0×0403
http://www.xuebuyuan.com/2177729.html


## 20161129

Interprocess Communications
https://msdn.microsoft.com/en-us/library/windows/desktop/aa365574(v=vs.85).aspx

Sharing memory between processes
http://www.boost.org/doc/libs/1_54_0/doc/html/interprocess/sharedmemorybetweenprocesses.html

What is shared memory?

Shared memory is the fastest interprocess communication mechanism. 

A Simple Wrapper for Sharing Data Structures Between Processes
https://www.codeproject.com/articles/1362/a-simple-wrapper-for-sharing-data-structures-betwe

WebSockets
Methods for Real-Time Data Streaming
https://os.alfajango.com/websockets-slides/#/

## links[
https://prx.im/
## links]
