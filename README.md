====TOP TODO====

Open-source radiation therapy research toolkit for 3D Slicer

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
