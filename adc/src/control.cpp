#include<sys/types.h>
#include<sys/mman.h>
#include<err.h>
#include<fcntl.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<cstdint>


//C11 includes.

#include "recorder.h"
#include "c_buffer.hpp"
#include "../build/bat.h"

int init_Buffer();
int cmd_Interpreter();
int cleanup();

int main(int argc, char *argv[]){

	printf("size of usigned long %i\n",sizeof(1UL));

	//Sample sample1;
	//sample1._sample_number = 50;
	//sample1._sample_value = 219;

	//printf("size of sample %i, sizeof\n", sizeof(sample1));
		
	C_Buffer<uint32_t> buffer(200e6);
	
	
	printf("this is seomething else, my PID is %i\n",getpid());

	//int sample[] = {5,4,3,2};
	 uint8_t something = 3;

	 printf("testing values %d, %d \n", sizeof(something), something);

	for(int i = 0; i<1e9; i++){
		//printf("%d\t", i);
		buffer.write_Samples(i);
	}

	//buffer.print_Buffer();
	//

	//sleep(3);
	/*  	
			sleep(30);
	//Mapping of the ringbuffer:
	//Open dev/zero to write zeros to the ringbuffer.
	int fd = -1;
	if((fd=open("/dev/zero", O_RDWR,0)) == -1){
	err(1,"open");
	}else printf("/dev/zero opened for reading and writing\n");

	//allocate the memory for the snapshot and databuffer:
	char *snapshot, *databuffer;
	snapshot = (char*)mmap(NULL, 30e6, PROT_READ|PROT_WRITE,MAP_ANON|MAP_SHARED, -1,0);
	databuffer = (char*)mmap(NULL, 128e6, PROT_READ|PROT_WRITE,MAP_SHARED, fd,0);

	if(snapshot == MAP_FAILED || databuffer == MAP_FAILED){
	errx(1,"either mmap");
	}else printf("snapshot and databuffer initialized\n");

	printf("contents of the databuffer are: %s \n", databuffer);

	const char test[] = "something else";
	const char test2[] = "something";



	int i = 0;
	while(i < 1e6){

	strcat(databuffer, test);
	i++;
	}

	strcat(databuffer, test2);



	printf("contents of the databuffer are: %s \n", databuffer);

	unsigned long t2 = 2UL << 12;


	printf(" result is: %ul, bitshifted\n", t2);
	*/

}

int init_Buffer(){

}


void cmd_Interpreter(){
	//create named pipe.
}

void cleanup(){

}



