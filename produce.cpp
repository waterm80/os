#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
using namespace std;



int main(){
    ofstream ofs;
    ofs.open("input.txt");
    int n = 250000;
    int min = 1;
    int max = 500000;
    ofs << n << '\n';
    while(n-- > 0)
        ofs << rand() % (max - min + 1) + min << " ";

    return 0;
}