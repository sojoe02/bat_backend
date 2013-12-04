
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
 *       Compiler:  gcc (g++ C11 comptible)
 *
 *         Author:  Soeren V. Joergensen, svjo@mmmi.sdu.dk
 *   Organization:  MMMI, SDU dk
 *
 * =====================================================================================
 */



#include<sys/types.h>
#include<sys/mman.h>
#include<sys/stat.h>

#include<err.h>
#include<fcntl.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<cstdint>
#include<fstream>
#include<iostream>

//C11 threading:
#include<atomic>
#include<thread>

#include "recorder.hpp"
#include "c_buffer.hpp"
#include "../build/bat.h"

float _sample_rate = 30e5;
int _snapshot_size = (int)10e6;

void stop_recording();
void snapshot(int arg_byte_size, int arg_sample_number);
void start_recording(char arg_device[], float arg_sample_rate, 
		char* arg_start_address, char* arg_end_address, int arg_buffer_size);

bool _running = true;
bool _recording = false;
//status integer:
int ret = 0;

std::thread* _record_thread;

struct Sample{
	uint16_t sample[8];
};


C_Buffer<Sample> _c_buffer(190e6);
Recorder _recorder;

int main(int argc, char *argv[]){

	//open the cmd pipe:
	FILE* f_pipe;
	char f_buffer[96];
	char filename[] = "/home/pi/grapper.cmd";
	char device[] = "/dev/comedi0";

	std::string cmd_exit = "exit";
	std::string cmd_start_rec = "start_rec";
	std::string cmd_stop_rec = "stop_rec";
	std::string cmd_take_snapshot = "take_snapshot";
	std::string input;

	printf("CMDs available are \n");
	printf("'exit'\texits the grapper\n");
	printf("'start_rec'\tstarts the recorder module\n");
	printf("'stop_rec'\tstops the recorder module\n");

	umask(0);

	printf("Creating Pipe... Waiting for receiver process...\n\n");
	//TRY TO CRATE A NAMED PIPE
	if (mkfifo(filename,0666)<0){
		perror("FIFO (named pipe) could not be created, it exists allready?");
		//exit(-1);
	}

	while(1){	
		sleep(1);;
		f_pipe = fopen(filename, "r");
		fgets(f_buffer, 96, f_pipe);
		input = f_buffer;
		//remove end of line:
		input = input.substr(0,input.length()-1);
		printf("input is %s\n", input.c_str());

		if(cmd_start_rec.compare(input) == 0){
			if(_recording){
				printf("System is recording, you need to stop ('stop_rec') it to restart it\n");

			}else{
				printf("Starting recording at %f[Hz]\n", _sample_rate);

				_record_thread = new std::thread(start_recording, device, 
						_sample_rate, _c_buffer.get_Start_Address(),
						_c_buffer.get_End_Address(), _c_buffer.get_Buffer_Size());
			}
		}

		else if(cmd_stop_rec.compare(input) == 0){
			if(!_recording){
				printf("System is not recording, no need to stop it\n");			
			}else{
				printf("Stopping recording now\n");
				stop_recording();			
			}

		}

		else if(cmd_take_snapshot.compare(input.substr(0,13)) == 0){
			input = input.substr(14, input.length()-1);
			int sample_number = atoi(input.c_str());
			if(sample_number >= 1){
				printf("taking snapshot, sample %i\n",sample_number);
				std::thread s_thread(snapshot,_snapshot_size,sample_number);
				s_thread.detach();
			}else printf("cancelling snapshot, sample number: %i\n", sample_number);
		}

		else if(cmd_exit.compare(input) == 0){
			if(_recording){
				printf("stopping recording\n");
				stop_recording();
			}
			break;			

		}else
			printf("Command '%s' unknown\n", input.c_str());

		fclose(f_pipe);
	}
	if (unlink(filename)<0){
		perror("Error deleting pipe file.");
		exit(-1);
	}
	fclose(f_pipe);

	printf("exiting \n");
}

void snapshot(int arg_byte_size, int arg_sample_number){
	//get rid of ekstra bytes:
	//and ensure that one whole samples are saved:
	int byte_size = arg_byte_size/sizeof(Sample) * sizeof(Sample); 

	char *snapshot_space;
	char *buffer_ptr;

	buffer_ptr = _c_buffer.get_Sample(arg_sample_number);

	snapshot_space = (char*)mmap(NULL,byte_size, 
			PROT_READ | PROT_WRITE, MAP_ANON|MAP_SHARED, -1, 0);

	if(snapshot_space == MAP_FAILED)
		errx(1,"snapshot allocation failed");

	memcpy(snapshot_space, buffer_ptr, byte_size);

	//save mmap to disk:
	FILE *s_file;
	char name_buf[1024];
	sprintf(name_buf,"snapshot,%i",arg_sample_number);
	s_file = fopen(name_buf, "wb");
	fwrite (snapshot_space, sizeof(char), byte_size, s_file);	
	fclose(s_file);
	//clear mmap:
	munmap(snapshot_space,byte_size);
}


void start_recording(char arg_device[], float arg_sample_rate, 
		char* arg_start_address, char* arg_end_address, int arg_buffer_size){

	_recording = true;

	_recorder.start_Sampling(arg_device, arg_sample_rate, 
			arg_start_address, arg_end_address, arg_buffer_size);

}

void stop_recording(){
	_recorder.stop_Sampling();

	_recording = false;
}





/*
   FILE *fp;
   char readbuf[1024];
   char _filename[] = "pipe.txt";

   std::string cmd_exit = "exit";
   std::string input;

   umask(0);
   mknod(_filename,S_IFIFO|0666,0);

/*  while(cmd_exit.compare(input)){
fp = fopen(_filename,"r");
fgets(readbuf,1024,fp);
input = readbuf;
input = input.substr(0,input.length()-1);
//printf("Recieved string: %s\n",readbuf);
fclose(fp);
printf("pipe line: %s\n", input.c_str());
}
printf("exit pushed\n");

return 0;


//while(std::getline(_fifo,cmd)){				

//	read(_file_descriptor, _buffer, 1024);
//	cmd = _buffer;
//	printf("Received: %s\n", _buffer);		

//}

//unlink(_fifo);
//printf("size of sample %i, sizeof\n", sizeof(sample1));

printf("this is seomething else, my PID is %i\n",getpid());

//int sample[] = {5,4,3,2};
uint8_t something = 3;

printf("testing values %d, %d \n", sizeof(something), something);

for(int i = 0; i<1e6; i++){
//printf("%d\t", i);
Sample sample;
sample.x = i;
sample.y = i;
//sample.f = i;
buffer.write_Samples(sample);
//if(i % (int)1e7 == 0){
//	printf("sample: %i\n",i);
//}
}


Recorder recorder;


char device[] = "/dev/comedi0";
recorder.start_Sampling(device, 30e5, buffer.get_Start_Address(), 
buffer.get_End_Address(), buffer.get_Buffer_Size());

//buffer.print_Buffer();
//
Sample* test_sample = buffer.get_Sample(34);
printf("testing sample %i,%i\n",test_sample->x, test_sample->y);

Sample* test_sample2 = ++test_sample;
printf("testing sample %i,%i\n",(++test_sample2)->x, (++test_sample2)->y);

}

int cmd_Interpreter(){
//create named pipe.
}

int cleanup(){

}


*/


