
/*
 * =====================================================================================
 *
 *       Filename:  control.cpp
 *
 *    Description:  Control is the interface into the data grapper it uses POSIX
 *			to control data recording, this piece of software is designed to run in a 
 *			Raspberry Pi, it can sample an analog source at 3[Mhz] into a circular
 * 			buffer located in a temporary file in /dev/shm.
 *			Raw data snapshots can be taken via the snapshot command, these will be saved.
 *			locally in a uid named file.
 *			For 3[Mhz] sampling it's recommended to overclock the Pi to 900[Mhz]. 
 *
 *        Created:  2013-11
 *       Revision:  none
 *       Compiler:  gcc (g++ C11 compatible)
 *
 *         Author:  Soeren V. Joergensen, svjo@mmmi.sdu.dk
 *      Co-author:	Malthe Hoej-Sunesen, only lines with --mhs postfix
 *   Organization:  MMMI, University of Southern Denmark
 *
 * =====================================================================================
 */



#include<sys/types.h>
#include<sys/mman.h>
#include<sys/stat.h>

#include<err.h>
#include<cstdint>
#include<vector>
#include<algorithm>
#include<iterator>
#include<sstream>

//C11 threading:
#include<atomic>
#include<thread>
#include<mutex> //--mhs

#include "recorder.hpp"
#include "c_buffer.hpp"
#include "../build/bat.h"
#include "Net.h"

#define PORT			2468	//--mhs
#define	IP_A			127		//--mhs
#define	IP_B			0		//--mhs
#define	IP_C			0		//--mhs
#define	IP_D			1		//--mhs
#define DGRAM_DELIMITER	" "		//--mhs

using namespace std;

uint32_t _sample_rate = (uint32_t)30e5;

void stop_recording();
void snapshot(uint64_t arg_sample_from, uint64_t arg_sample_to, const char arg_path[]);
void start_recording(char arg_device[], uint32_t arg_sample_rate, 
		char* arg_start_address, char* arg_end_address, int arg_buffer_size);
void write_error(const char arg_msg[], const char arg_path[]);
void analyzer();
void start_continuous_recording(char arg_device[], uint32_t arg_sample_rate,
		char* arg_start_address, int arg_buffer_size);	//--mhs
void stop_recording(char arg_device[], uint32_t arg_sample_rate, 
		char* arg_start_address, int arg_buffer_size);	//--mhs
bool _running = true;
bool _recording = false;
bool _continuous_recording = false;
bool _stop_continuous_recording = false;
mutex send_UDP_mutex;//--mhs
//status integer:
int ret = 0;

thread* _record_thread;
thread* analyzer;

bool Utility::SNAP_READY = false;

struct Sample{
	uint16_t sample[8];
};


C_Buffer<Sample> _c_buffer(190e6);
Recorder _recorder;

int main(int argc, char *argv[]){

	//open the cmd pipe:
	FILE* f_pipe;
	char f_buffer[1024];
	char filename[] = "/home/pi/grapper.cmd";
	char device[] = "/dev/comedi0";

	string cmd_exit = "exit";
	string cmd_start_rec = "start_rec";
	string cmd_stop_rec = "stop_rec";
	string cmd_take_snapshot = "snapshot";
	string cmd_set_sr = "set_sr";
	string cmd_start_cont_rec = "start_cont_rec";
	string cmd_stop_cont_rec = "stop_cont_rec";
	string input;

	printf("CMDs available are \n");
	printf("'exit'\texits the grapper\n");
	printf("'start_rec'\tstarts the recorder module\n");
	printf("'stop_rec'\tstops the recorder module\n");
	printf("'start_cont_rec'\tstarts a continuous recording\n");
	printf("'stop_cont_rec'\tstops a continuous recording\n");

	umask(0);

	printf("Creating Pipe... Waiting for receiver process...\n\n");
	//TRY TO CREATE A NAMED PIPE
	if (mkfifo(filename,0666)<0){
		perror("FIFO (named pipe) could not be created, it exists already?");
		//exit(-1);
	}

	f_pipe = fopen(filename, "r");
	
	while(1){


		fgets(f_buffer, 1024, f_pipe);
		input = f_buffer;
		printf("input is %s\n", input.c_str());

		//Split the input string up and put command and it's arguments in a vector:		
		istringstream iss(input);
		vector<string> input_data{istream_iterator<string>{iss},
			istream_iterator<string>{}};


		if(cmd_start_rec.compare(input_data[0]) == 0){
			if(_recording){
				printf("System is recording, you need to stop ('stop_rec') it to restart it\n");
			}else {
				printf("Starting recording at %u[Hz]\n", _sample_rate);

				_record_thread = new thread(start_recording, device, 
						_sample_rate, _c_buffer.get_Start_Address(),
						_c_buffer.get_End_Address(), _c_buffer.get_Buffer_Size());
			}
		}
		
		else if(cmd_set_sr.compare(input_data[0])==0){
			if(!_recording && input_data.size() >= 2){
				_sample_rate = stoi(input_data[1]);
				printf("Sample rate set to %u[hz]",_sample_rate);
			}else{
				printf("Not enough arguments, to adjust samplerate");
				printf("or system is recording!\n");
			}


		}

		else if(cmd_stop_rec.compare(input_data[0]) == 0){
			if(!_recording){
				printf("System is not recording, no need to stop it\n");			
			}else{
				printf("Stopping recording now\n");
				stop_recording();		
				sleep(1);			
			}

		}

		else if(cmd_take_snapshot.compare(input_data[0]) == 0){

			uint64_t sample_from = stoull(input_data[1]);
			uint64_t sample_to = stoull(input_data[2]);
			string path = input_data[3];

			printf("taking snapshot, path is: %s\n"
					,path.c_str());

			thread s_thread(snapshot, sample_from, sample_to, path.c_str());
			s_thread.detach();
		}
		
		else if(cmd_start_cont_rec.compare(input_data[0] == 0)	{
		//place actual recording command in here. DO NOT CALL UDP SENDING FUNCTION!!!
			char buffer[512];
			if(input_data.size() != 5)	{	//number of arguments for function!
				sprintf(buffer, "too few or too many arguments for continuous recording ");
				warnx(buffer);
				write_error("arg", buffer);
			}	else	{
				try{
					uint32_t count = stoull(input_data[3]); //change to correct argument number!!
					string path = input_data[4]				//change to correct argument number!!
				
					printf("starting continuous recording\n");
					
					if(_continuous_recording)	{
						sprintf(buffer,"continuous recording already in progress; aborting");
						warnx("%s", buffer);
						write_error(path.c_str(), buffer);
					}	else if(!_recording)	{
						sprintf(buffer, "system is not recording; aborting");
						warnx("%s", buffer);
						write_error(path.c_str(), buffer);
					}	else {
						printf("starting continuous recording thread, to path: %s\n", path.c_str());
						
						_record_thread = new thread(start_continuous_recording, count, path.c_str())
					}
				}
			}
		}

		else if(cmd_stop_cont_rec.compare(input_data[0] == 0)	{
		//place actual stop recording command in here. DO NOT CALL UDP SENDING FUNCTION!!!
		}
		
		else if(cmd_exit.compare(input_data[0]) == 0){
			if(_recording){
				printf("stopping recording\n");
				stop_recording();
			}
			break;			

		}else
			printf("Command '%s' unknown\n", input.c_str());

	}
	if (unlink(filename)<0){
		perror("Error deleting pipe file.");
		exit(-1);
	}
	fclose(f_pipe);

	printf("exiting \n");
}

void snapshot(uint64_t arg_sample_from, uint64_t arg_sample_to, const char arg_path[]){
	//get rid of ekstra bytes:
	//and ensure that one whole samples are saved:

	uint32_t byte_size = (arg_sample_to - arg_sample_from)*(sizeof(Sample)) ; 


	char *snapshot_space;
	char *buffer_ptr;

	buffer_ptr = _c_buffer.get_Sample(arg_sample_from);	

	//allocate anonymous memmap for the snapshot data:
	snapshot_space = (char*)mmap(NULL,byte_size, 
			PROT_READ | PROT_WRITE, MAP_ANON|MAP_SHARED, -1, 0);

	if(snapshot_space == MAP_FAILED)
		errx(1,"snapshot allocation failed");

	//copy the snapshot data into the empty memmap:
	memcpy(snapshot_space, buffer_ptr, byte_size);

	//save memmap data to disk:
	FILE *s_file;
	s_file = fopen(arg_path, "wb");
	fwrite (snapshot_space, sizeof(char), byte_size, s_file);	
	fclose(s_file);
	//clear mmap:
	munmap(snapshot_space,byte_size);
}

void start_recording(char arg_device[], uint32_t arg_sample_rate, 
		char* arg_start_address, char* arg_end_address, int arg_buffer_size){

	_recording = true;

	_recorder.start_Sampling(arg_device, arg_sample_rate, 
			arg_start_address, arg_end_address, arg_buffer_size);

}
	
void stop_recording(){
	_recorder.stop_Sampling();

	_recording = false;
}

void analyzer()	{
	//need all the char arg_device[] in here somehow...
	uint32_t sample_rate = 375000;
	sampler = new thread(sampler, this, device1, sample_rate);
}

void continuous_recording(uint32_t arg_count, const char arg_path[])	{
	printf("starting continuous recorder thread\n");
	std::unique_lock<std::mutex> lk(Utility::LM);
	
	char path[1024];
	char* buffer_ptr;
	uint32_t byte_size = Utility::SNAPSHOT_BYTE_SIZE;
	_continuous_recording = true;
	uint32_t i = 1;
	
	FILE s_file;
	sprintf(path, "%s.%llX", arg_path, (uint64_t)Utility::SNAP_SAMPLE);
	s_file.open(path, "a");
	while(_continuous_recording && arg_count >= i)	{
		printf("waiting for condition variable to be set\n");
		
		Utility::CV.wait(lk,[]{return Utility::SNAP_READY;});
		Utility::SNAP_READY = false;
		
		
		if(_stop_continuous_recording)	{
			printf("stopping continuous recording\n");
			_stop_continuous_recording = false;
			break;
		}
		//generate write path
		sprintf(path, "%s.%llX.%u", arg_path, (uint64_t)Utility::SNAP_SAMPLE);
		
		printf("writing snapshot to recording\n");
		buffer_ptr = _c_buffer.get_Sample(Utility::SNAP_SAMPLE);
		
		fwrite(buffer_ptr, sizeof(char), byte_size, s_file);
	}
	fclose(s_file);
	lk.unlock();
	_continuous_recording = false;
}

void stop_continuous_recording()	{
	_stop_continuou_recording = true;
	Utility::CV.notify_one();
}

//whole function --mhs
//If used in thread environment, please lock whole process with mutex!
bool send_command_start_continuous_recording(char arg_device[], uint32_t arg_sample_rate, char* arg_start_address)	{
	net::Socket socket; //new socket object
	if(!socket.Open(PORT))	{ //need to open socket to send
		printf("control.cpp: failed to create socket\n");
		return false;
	}
	char sample_rate[16];		//for conversion from uint32_t to string, as per http://cboard.cprogramming.com/c-programming/104485-how-convert-uint32_t-string.html
	snprintf(sample_rate, sizeof(sample_rate), "%lu", (unsigned long)arg_sample_rate);
	string data = string(arg_device) + " " + string(sample_rate) + " " + string(arg_start_address);		//data to be sent
	int data_length = static_cast<int>strlen(data.c_str());		//get length, not size, for sending; if sizeof, then size=4, only 4 bytes sent!
	bool reply = socket.Send(Address(IP_A, IP_B, IP_C, IP_D, PORT),
		data.c_str(), (int)data_length);		//sent or not?
	return reply;		//indicate success
}

//whole function --mhs
bool send_command_stop_continuous_recording(char* arg_start_address, char* arg_end_address)	{
	net::Socket socket; 
	if(!socket.Open(PORT))	{
		printf("control.cpp: failed to create socket\n");
		return false;
	}
	string data = string(arg_device) + " " + string(arg_start_address) + " " + string(arg_end_address);
	int data_length = static_cast<int>strlen(data.c_str());
	bool reply = socket.Send(Address(IP_A, IP_B, IP_C, IP_D, PORT), data.c_str(), (int)data_length);
	return reply;
}

void write_error(const char arg_path[], const char arg_msg[]){
	char path[256];
	//get the time:
	std::time_t result = std::time(NULL);

	sprintf(path, "%s.error.%X", arg_path, (uint32_t)result);
	std::ofstream file(path);
	file << arg_msg;
	file.close();	
}