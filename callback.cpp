
//http://www.cplusplus.com/forum/beginner/196638/
#include <iostream>
#include <string>
#include <functional>

class A
{
public:
  A() = default;
  ~A() = default;

  void BindCallback(std::function<int(int, int)> fn) {
    privateCallback = std::bind(fn, std::placeholders::_1, std::placeholders::_2);
  }

  void MakeCallback(int i, int j) {
    if (privateCallback)
      std::cout << "Output from callback: " << privateCallback(i, j) << std::endl;
  }

private:
  std::function<int(int, int)> privateCallback; //Could do a vector of them, or whatever
};

int f(int a, int b) {
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
