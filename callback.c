


#pragma once

#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(WIN32)
#define DLLEXP __declspec(dllexport) 
#else
#define DLLEXP
#endif




/* input: process between 0.0 and 1.0
   output: status 1 means canceled by the user
   return: 0 - success, others - error
*/
typedef int (*callback_process)(double /*process*/, int* /*status*/);

__declspec(dllexport) int callalg(int n, callback_process callback);

__declspec(dllexport) int test(int n);



#include <stdio.h>
#include "ai_alg.h"

int callalg(int n, callback_process callback)
{
   // printf("n=%d\n",n);
    double process = 0.5;

    int i=0;
    callback(process, &i);
    return 0;
}

int test(int n)
{
    return n*n;
}



#include <stdio.h>
//#include "ai_alg.h"
#include <Windows.h>

typedef int (*callback_process)(double,int*);
typedef int (*my_callalg)(int, callback_process);

typedef int (*my_test)(int);
//////////////////////////////////////////////////////////////////////////

int set_process(double process, int* status)
{
   printf("process: %f \n", process);
 //  *cancel = true;
   *status = 1;
   return 0;
}

int main()  
{ 
    char* sDllFileName="test.dll";
    void* m_DllInstance = LoadLibraryA(sDllFileName);  
    char* test = "test";
    my_test pf = (my_test)GetProcAddress((HINSTANCE)m_DllInstance,test);
    int res = pf(3);

    my_callalg pf1 = (my_callalg)GetProcAddress((HINSTANCE)m_DllInstance,"callalg");


    pf1(3,set_process);

    return 0;  
}  






