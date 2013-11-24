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

#include "recorder.h"
#include "c_buffer.hpp"
#include "../build/bat.h"


//std::string _filename("cmd_pipe");
std::ifstream _fifo;
char _buffer[1024];
int _file_descriptor;


int init_Buffer();
int cmd_Interpreter();
int cleanup();

struct sample{
	short int x;
	short int y;
	//short int f;
};

int main(int argc, char *argv[]){

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
*/

	//while(std::getline(_fifo,cmd)){				

	//	read(_file_descriptor, _buffer, 1024);
	//	cmd = _buffer;
	//	printf("Received: %s\n", _buffer);		

	//}

	//unlink(_fifo);
	//printf("size of sample %i, sizeof\n", sizeof(sample1));
	 	
		C_Buffer<sample> buffer(9000);

 
		printf("this is seomething else, my PID is %i\n",getpid());

	//int sample[] = {5,4,3,2};
	uint8_t something = 3;

	printf("testing values %d, %d \n", sizeof(something), something);

	for(int i = 0; i<1e3; i++){
	//printf("%d\t", i);
		sample sample;
		sample.x = i;
		sample.y = i;
		//sample.f = i;
		buffer.write_Samples(sample);
	} 
	//buffer.print_Buffer();
	//
	sample* test_sample = buffer.get_Sample(23312);
	printf("testing sample %i,%i\n",test_sample->x, test_sample->y);
	
	sample* test_sample2 = ++test_sample;
	printf("testing sample %i,%i\n",(++test_sample2)->x, (++test_sample2)->y);

}

int init_Buffer(){

}


int cmd_Interpreter(){
	//create named pipe.
}

int cleanup(){

}



