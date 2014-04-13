#include <iostream>
#include <vector>
//This is mostly a draft document, ought to be merged into control.cpp, for
//ease of use vis-à-vis pointers
using namespace std;

#define NUM_DEVICES 20
#define NUM_MICS	8
#define SUM			100	//amount of samples in sum --mhs
#define THRESHOLD	500	//is it a bat or not? --mhs

int insert(int, int);
void batsound_present(int);
void batsound_gone(int);

int old_value = 0, new_value = 0;
int x[NUM_DEVICES] = {0, 0, 0, 0, 0, 0, 0, 0};
int store[NUM_DEVICES][SUM];	//classic circular array --mhs
int sum[NUM_DEVICES] = {0, 0, 0, 0, 0, 0, 0, 0};
bool recording[NUM_DEVICES] = {false, false, false, false, false, false, false, false};
//vector for storing which devives are recording already --mhs
int least_sig_val[NUM_DEVICES];	//address to the least significant (= earliest)
								//value in the store. --mhs
int main() {
	//get pointer from A/D (recorder.hpp) --mhs
	//operate on data and determine if event --mhs
	//UDP package to Perl script --mhs
	//TODO: Get access to Perl script (not in git branch what I can see) --mhs
	//TODO: How high is the highest possible sound (sint, long?) --mhs
	//TODO: Best possible circular array implementation? --mhs
	//discussion: The first 100 samples - ok to be wasted? Want to do 
	//in order to avoid a boolean check every time the sounds are 
	//analyzed. Working on like it is a deal. --mhs

	

	for(int i = 0; i < NUM_DEVICES; i++) {
		//get value/sound data from pointer, put int array x[] --mhs
		
		//get oldest value from store --mhs
		old_value = store[i][least_sig_value];
		
		//just rebranding newest value --mhs
		new_value = x[i];

		//calculate sum --mhs
		sum[i] = sum[i] - old_value * old_value + new_value * new_value;
		
		//save newest value for later --mhs
		insert(new_value, i);

		//check if the registered sounds are louder than the threshold --mhs
		if(sum[i] > THRESHOLD) { //remember to avoid multiple of the same device --mhs
			batsound_present(i);	//should create new thread to handle this,
									//to allow bats at more than one location
									//--mhs
		}
	}
}

void insert(int num, int index) {
	//avoid overflow in circular buffer --mhs
	if(least_sig_value[index] > (SUM - 2)) {
		least_sig_value[index] = -1;	//intentionally off by one - see just below --mhs
	}

	//move to next (= oldest) sample --mhs
	least_sig_value[index]++;	//no if-statement to avoid additional overhead --mhs

	//store latest sample in circular buffer --mhs
	store[index][least_sig_value[index]] = num;
}

void batsound_present(int index) {
	//UDP to Perl script to start recording --mhs
	//get value/sound data from pointer for -index- device, put in dedicated thread --mhs
	recording[index] = true;
	

}