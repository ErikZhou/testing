#include <iostream>
#include <fstream>
#include <string>
using namespace std;

int save_shorts(const unsigned short* array, size_t num_shorts)
{
  int ok = 0;
  FILE* out = fopen("numbers.bin", "wb");

  if (out != NULL)
  {
    ok = fwrite(array, num_shorts * sizeof *array, 1, out) == 1;
    fclose(out);
  }
  return ok;
}

void readFile(const char * fname, unsigned short * array, unsigned length)
{
    std::ifstream fin(fname, std::ios::binary);
    fin.seekg(0, std::ios_base::end);
    unsigned file_len = fin.tellg();
    if (file_len != length * sizeof(unsigned short))
    {
        std::cout << "Error: file length: " << file_len 
                  << "  expected: " << length * sizeof(unsigned short) << std::endl;
        return; 
    }
    
    fin.seekg(0, std::ios_base::beg);
    fin.read( (char *) array, file_len);
    fin.close();
}

int main() 
{
  cout << "Hello world!" << endl;
  int num = 2;
  unsigned short sArr[2];
  sArr[0]=123;
  sArr[1]=456;
  save_shorts(sArr,2);

  unsigned short buf[2] = {0}; 
    readFile("numbers.bin", buf, 2);
  return 0;
}
