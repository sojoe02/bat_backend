#include <iostream>
//This is mostly a draft document, ought to be merged into control.cpp, for
//ease of use vis-à-vis pointers
using namespace std;

int main() {
	//get pointer from A/D (recorder.hpp) --mhs
	//operate on data and determine if event --mhs
	//UDP package to Perl script --mhs
	//TODO: Get access to Perl script (not in git branch what I can see) --mhs
	//TODO: How high is the highest possible sound (sint, long?) --mhs
	//discussion: The first 100 samples - ok to be wasted? Want to do 
	//in order to avoid a boolean check every time the sounds are 
	//analyzed. Working on like it is a deal. --mhs

	int32 x[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	int32 store[8][100];
	int32 sum[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	uint32 least_sig_val[8];	//address to the least significant (= earliest)
								//value in the store. --mhs

	for(int i = 0; i < 8; i++) {
		
	}


}
