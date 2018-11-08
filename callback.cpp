
//http://www.cplusplus.com/forum/beginner/196638/
#include <iostream>
#include <string>
#include <functional>

class A
{
public:
  A() = default;
  ~A() = default;

    void BindCallback(std::function<int(void*,int, int)> fn) 
    {
        privateCallback = std::bind(fn, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    }

    void MakeCallback(int i, int j) 
    {
        if (privateCallback)
           printf("Output from callback: %d\n", privateCallback((void*)m_pCaller, i, j));
    }
    void SetCaller(void* pCaller)
    {
        m_pCaller = pCaller;
    }

private:
    std::function<int(void*,int, int)> privateCallback;
    void*   m_pCaller;
};

int f(void* p, int a, int b) {
  return (a + b);
}

int main()
{
  A obj;

  obj.BindCallback(f);

  obj.MakeCallback(3, 4);
  obj.MakeCallback(7, 8);

  return 0;
}
