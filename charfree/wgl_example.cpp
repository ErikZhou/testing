

#include <windows.h>
#include <GL/GL.h>

//#pragma comment (lib, "opengl32.lib")

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

int WinMain( __in HINSTANCE hInstance, __in_opt HINSTANCE hPrevInstance, __in_opt LPSTR lpCmdLine, __in int nShowCmd )
{
    MSG msg          = {0};
    WNDCLASS wc      = {0}; 
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
    wc.lpszClassName = L"oglversionchecksample";
    wc.style = CS_OWNDC;
    if( !RegisterClass(&wc) )
        return 1;
    CreateWindowW(wc.lpszClassName,L"openglversioncheck",WS_OVERLAPPEDWINDOW|WS_VISIBLE,0,0,640,480,0,0,hInstance,0);

    while( GetMessage( &msg, NULL, 0, 0 ) > 0 )
        DispatchMessage( &msg );

    return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
    case WM_CREATE:
        {
            PIXELFORMATDESCRIPTOR pfd =
            {
                sizeof(PIXELFORMATDESCRIPTOR),
                1,
                PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    //Flags
                PFD_TYPE_RGBA,            //The kind of framebuffer. RGBA or palette.
                32,                        //Colordepth of the framebuffer.
                0, 0, 0, 0, 0, 0,
                0,
                0,
                0,
                0, 0, 0, 0,
                24,                        //Number of bits for the depthbuffer
                8,                        //Number of bits for the stencilbuffer
                0,                        //Number of Aux buffers in the framebuffer.
                PFD_MAIN_PLANE,
                0,
                0, 0, 0
            };

            HDC ourWindowHandleToDeviceContext = GetDC(hWnd);

            int  letWindowsChooseThisPixelFormat;
            letWindowsChooseThisPixelFormat = ChoosePixelFormat(ourWindowHandleToDeviceContext, &pfd); 
            SetPixelFormat(ourWindowHandleToDeviceContext,letWindowsChooseThisPixelFormat, &pfd);

            HGLRC ourOpenGLRenderingContext = wglCreateContext(ourWindowHandleToDeviceContext);
            wglMakeCurrent (ourWindowHandleToDeviceContext, ourOpenGLRenderingContext);

            //MessageBoxA(0,(char*)glGetString(GL_VERSION), "OPENGL VERSION",0);

            std::string str;
            const GLubyte* vendor = glGetString(GL_VENDOR); // Returns the vendor
            const GLubyte* renderer = glGetString(GL_RENDERER); // Returns a hint to the model
            str += std::string((const char*)vendor);
            str += std::string("\n");
            str += std::string((const char*)renderer);

            str += std::string("\n");
            const GLubyte* version = glGetString(GL_VERSION);
            str += std::string((const char*)version);

            GLint totalMemoryKb = 0;
            glGetIntegerv(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &totalMemoryKb);

            std::cout<<"total memory"<<totalMemoryKb / 1024.0 <<"\n";

            GLint currentMemoryKb = 0;
            glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &currentMemoryKb);

            std::cout<<"current memory"<<currentMemoryKb / 1024.0 <<"\n";

            GLint count;
            glGetIntegerv(GL_NUM_EXTENSIONS, &count);

            //for (GLint i = 0; i < count; ++i)
            //{
            //    const char *extension = (const char*)glGetStringi(GL_EXTENSIONS, i);
            //    if (!strcmp(extension, "GL_NVX_gpu_memory_info"))
            //    {
            //        printf("%d: %s\n", i, extension);
            //    }
            //}

            MessageBoxA(0,str.c_str(), "OPENGL VERSION",0);


            wglDeleteContext(ourOpenGLRenderingContext);
            PostQuitMessage(0);
        }
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;

}
//
//
