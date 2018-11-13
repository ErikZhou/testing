#include <iostream>
#include <fstream>
using namespace std;

int main()
{
  short* fnum = new short[4];
  fnum[0]=95;
  fnum[1]=-34;
  fnum[2]=10;
  fnum[3]=21;
  int i;

  ofstream out("numbers", ios::out | ios::binary);
  if(!out) {
    cout << "Cannot open file.";
    return 1;
   }

  cout<<"sizeof fum ="<<sizeof fnum<<"\n";
  out.write((char *) fnum, sizeof fnum);
  out.close();

  for(i=0; i<4; i++) // clear array
    fnum[i] = 0.0;

  ifstream in("numbers", ios::in | ios::binary);
  in.read((char *) fnum, sizeof fnum);

  // see how many bytes have been read
  cout << in.gcount() << " bytes read\n";

  for(i=0; i<4; i++) // show values read from file
  cout << fnum[i] << " ";

  std::cout<<"\n";
  in.close();

  delete[] fnum;
  return 0;
}
