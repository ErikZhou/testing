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

## Radiation Oncology Physics:
A Handbook for Teachers and Students
E.B. Podgorsak
Technical Editor
INTERNATIONAL ATOMIC ENERGY AGENCY
VIENNA, 2005

====TOP TODO====

## 20170728
Dose calculation algorithms in 3DCRT and IMRT
http://www.aapm.org/meetings/amos2/pdf/35-9832-6508-974.pdf

剂量计算
1.解析方法或经验方法，如卷积（Convolution)、叠加 (Superposition)、笔形束(Pencil Beam)等
优点：速度快
缺点：精度差。人体内部射线散射及次级辐射过程的复杂性难以准确计算，或射线传输中电子失衡等物理过程的影响，特别在低密度或高密度的不均匀组织（气腔或致密骨）和靠近不同组织分界面附近表现更为突出，剂量计算误差可达11％～32％

2.  统计模拟方法，如蒙特卡罗(Monte Carlo)
优点：通过随机模拟大量粒子与物质相互作用的物理全过程，可以准确计算射束与介质作用后的能量沉积，反映了最真实的剂量分布。计算精度误差可以小于1%，结果相对更加精确，所以被业界认为是最精确的一种剂量计算方法
缺点：通常速度慢

逆向优化方法
逆向优化方法是根据医生或物理师给定的目标进行逆向求解、迭代优化，最终获得满足给定条件的治疗方案的方法。TPS的逆向优化方法一般包括：

通量图优化（FMO）：针对IMRT，将方案转换为单目标问题，根据目标优化出理想照射条件下的通量分布，优化速度快，需要经过叶片序列优化为多叶准直器形状分布，可能存在转换误差。
 叶片序列优化（Leaf Sequencing）：可实施step and shoot 和sliding window两种不同模式的治疗。

直接子野优化（DAO）：针对IMRT，将方案转换为单目标问题，考虑机器约束，根据目标优化出可供多叶准直器直接执行的治疗方案，误差小，优化速度慢。

子野权重优化（SWO）：针对适形放疗（CRT），将方案转换为单目标问题，用于对三维适形计划进行微调。

多目标优化（MCO）：放疗方案是一个多目标问题，直接求解多目标，获得多个满足条件的方案，医生或物理师根据实际情况进行选取最终方案。

现代放疗技术的主攻方向

影像引导放疗设备（现代加速器标配）
诊断级影像
高精度定位与摆位

快速高效的治疗方案
高剂量率加速器（提高能量、加大馈入功率、FFF模式等）
自动放疗计划：自动/半自动勾画（atlas based，model based等）、自动计划（algorithm based，knowledge based等）
智能逆向优化方法（FMO、DAO、SWO、MCO）

高精度治疗计划与实施
高精度快速剂量计算（Monte Carlo等）
精确的计划实施设备（MLC等）
可靠的剂量验证与质量保证（EPID等）

互联网+放疗
网络化、多中心互联
大数据、高效工作流布局

## 20170727

放射物理学-戴晓波
http://flk.sysucc.org.cn/uploadfiles/about/%E6%94%BE%E5%B0%84%E7%89%A9%E7%90%86%E5%AD%A6-%E6%88%B4%E6%99%93%E6%B3%A2.pdf

## 20170717

https://en.wikipedia.org/wiki/Monitor_unit
1 Gy = 100 cGy
https://www.unitjuggler.com/convert-absorbeddose-from-Gy-to-cGy.html

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
