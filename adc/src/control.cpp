
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
 *   Organization:  MMMI, University of Southern Denmark
 *		  Version:	0.71
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
#include<ctime>
#include<iostream>
#include<fstream>

//C11 threading:
#include<atomic>
#include<thread>
#include<mutex>
#include<condition_variable>

#include "recorder.hpp"
#include "c_buffer.hpp"
#include "../build/bat.h"

using namespace std;

//set circle buffer size:
uint32_t BUFFER_SIZE = 190e6;

//-----------UTILITY-VARIABLES------------------------//
uint32_t Utility::SNAPSHOT_MAX_BYTE_SIZE = 152e6;
uint32_t Utility::SNAPSHOT_BYTE_SIZE = 152e6;

std::atomic_uint Utility::SNAPSHOT_BLOCK_SIZE;
std::atomic_uint Utility::WRITTEN_BLOCK;

std::atomic_uint_least64_t Utility::SNAP_SAMPLE;
std::atomic_uint_least64_t Utility::LAST_SAMPLE;

std::condition_variable Utility::CV;
std::mutex Utility::LM;

bool Utility::SNAP_READY = false;
//----------------------------------------------------//

//-----------Function-Headers-------------------------//
void stop_recording();
void snapshot(uint64_t arg_sample_from, uint64_t arg_sample_to, const char arg_path[]);
void start_recording(char arg_device[], uint32_t arg_sample_rate, 
		char* arg_start_address, char* arg_end_address, int arg_buffer_size);
void serial_snapshot(uint32_t arg_sample_number, uint32_t arg_sample_length, uint32_t arg_count, const char arg_path[]);
void stop_serial_snapshot();
void write_error(const char arg_msg[], const char arg_path[]);

//-----------Variables:-------------------------------//
uint32_t _sample_rate = (uint32_t)30e5;
bool _recording = false;
std::atomic_bool _serial_snapshotting;
std::atomic_bool _stop_serial_snap;
bool _snap_wait = false;
//status integer:
int ret = 0;

thread* _record_thread;
thread* _serial_thread;

struct Sample{
	uint16_t sample[8];
};


C_Buffer<Sample> _c_buffer(BUFFER_SIZE);
Recorder<Sample> _recorder;

/**
 * Initializes the command structure of the program.
 * It's simple string matching, strings are read from a named pipe
 * in the users homedir e.g., commands can be given via cat > <pipe> in
 * regular bash.
 */ 
int main(int argc, char *argv[]){

	_serial_snapshotting = false;
	_stop_serial_snap = false;

	Utility::SNAPSHOT_BLOCK_SIZE = Utility::SNAPSHOT_BYTE_SIZE/4096;

	//open the cmd pipe:
	FILE* f_pipe;
	char f_buffer[1024];
	char filename[] = "/home/pi/grapper.cmd";
	char device[] = "/dev/comedi0";

	printf("---------------------------------------------------\n");
	//-----------INFORMATION------------------------------//
	printf("size of channel sample will be %i bytes'\n", sizeof(Sample));	
	printf("sample_rate is %i[Hz]\n", _sample_rate);

	//-----------COMMANDS---------------------------------//
	string cmd_exit = "exit";
	string cmd_start_rec = "start_rec";
	string cmd_stop_rec = "stop_rec";
	string cmd_take_snapshot = "snapshot";
	string cmd_set_sr = "set_sr";
	string cmd_take_snapshot_series ="serial_snapshot";
	string cmd_stop_snapshot_series ="serial_stop";
	string cmd_take_snapshot_series_simple = "simple_serial";
	string input;

	printf("---------------------------------------------------\n");
	printf("CMDs available are \n");
	printf("---------------------------------------------------\n");
	printf("'exit'\t\t (exits the grapper)\n");
	printf("'start_rec <sample_rate>'\t (starts the recorder module)\n");
	printf("'stop_rec'\t\t (stops the recorder module)\n");
	printf("'snapshot <sample_from,64uint> <sample_to,64uint> <path,string>'\n");
	printf("'serial_snapshot <byte_size,int> <path,string>'\n");
	printf("'serial_stop \n'");
	printf("---------------------------------------------------\n");
	//-----------------------------------------------------//

	//setting file permission via umask:
	umask(0);
	//remove the pipe if it allready exists to get rid of potential
	//'pipe garbage'
	remove (filename);

	printf("Creating Pipe; %s, Waiting for receiver process...\n\n", filename);
	//TRY TO CRATE A NAMED PIPE
	if (mkfifo(filename,0666)<0){
		perror("FIFO (named pipe) could not be created");
		//exit(-1);
	}
	printf("---------------------------------------------------\n");

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

			}else{
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
				_record_thread->join();	
				//sleep(1);			
			}
		}

		else if(cmd_take_snapshot.compare(input_data[0]) == 0){
			uint64_t sample_from = stoull(input_data[1]);
			uint64_t sample_to = stoull(input_data[2]);
			string path = input_data[3];

			printf("taking snapshot, path is: %s\n",path.c_str());

			thread s_thread(snapshot, sample_from, sample_to, path.c_str());
			s_thread.detach();
		}

		else if(cmd_take_snapshot_series.compare(input_data[0]) == 0){

			uint64_t sample_number = stoull(input_data[1]);
			uint32_t sample_length = stoull(input_data[2]);
			uint32_t count = stoull(input_data[3]);
			string path = input_data[4];			

			printf("taking snapshots in sequence\n");

			if (sample_length * sizeof(Sample) > Utility::SNAPSHOT_MAX_BYTE_SIZE){
				printf("snapshot size too large, maximum is %i ; aborting\n", Utility::SNAPSHOT_MAX_BYTE_SIZE);

			} else if(_serial_snapshotting){
				printf("the serial snapshotter is already running; arborting\n");
			}
			else if(!_recording)	{
				printf("system is not recording; aborting\n");
			} 
			else if(sample_number < (Utility::LAST_SAMPLE - sample_length)){
				printf("the sample you want is obsolete\n");
			}
			else{
				Utility::SNAPSHOT_BLOCK_SIZE = sample_length/4096;
				Utility::SNAPSHOT_BYTE_SIZE = sample_length * sizeof(Sample);

				printf("starting snapshot series thread, to path: %s\n", path.c_str());		
				_serial_thread = new thread(serial_snapshot, sample_number , sample_length, count, path.c_str());
			}
		}
		else if(cmd_take_snapshot_series_simple.compare(input_data[0]) == 0){

			uint32_t sample_length = 120e6/sizeof(Sample);
			Utility::SNAPSHOT_BLOCK_SIZE = (sample_length*sizeof(Sample))/4096;
			Utility::SNAPSHOT_BYTE_SIZE = sample_length*sizeof(Sample);
			std::string path = "simple_shot";
			uint64_t sample_number = Utility::LAST_SAMPLE;

			printf("starting simple snapshot series thread0");

			_serial_thread = new thread(serial_snapshot, sample_number, sample_length, 2, path.c_str());

		}

		else if(cmd_stop_snapshot_series.compare(input_data[0]) == 0){
			_serial_snapshotting = false;
			stop_serial_snapshot();

			if(_serial_thread != NULL && _serial_thread->joinable())
				_serial_thread->join();	

		}

		else if(cmd_exit.compare(input_data[0]) == 0){
			if(_recording){
				printf("stopping recording\n");
				stop_recording();
				_serial_snapshotting = false;
				sleep(1);
				if(_record_thread != NULL && _record_thread->joinable())
					_record_thread->join();
				if(_serial_thread != NULL && _serial_thread->joinable())
					_serial_thread->join();
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

/**
 * Takes a series of sequencial snapshots from the circular buffer, between 
 * snapshots it waits for a signal from the Recorder that signals 
 * whenever a data for a snapshot is ready.
 * @param arg_sample_number sample startpoint for the snapshot series.
 * @param arg_path the path the snapshots will be written to.
 * @param arg_count amount of snapshots that will be written.
 * @param arg_sample_length number of samples each snapshot will contain.
 */
void serial_snapshot(uint32_t arg_sample_number, uint32_t arg_sample_length, uint32_t arg_count, const char arg_path[]){

	printf("starting simple snapshotter\n");
	std::unique_lock<std::mutex> lk(Utility::LM);

	char path[1024];
	char* buffer_ptr;
	uint32_t byte_size = Utility::SNAPSHOT_BYTE_SIZE;
	uint32_t samples_pr_snapshot = Utility::SNAPSHOT_BYTE_SIZE / sizeof(Sample);

	_serial_snapshotting = true;

	uint32_t i = 1;
	//starting the snapshot loop, which is controlled by the recorder thread.
	while( _serial_snapshotting == true && arg_count >= i ){

		printf("waiting for condition variable to be set\n");

		Utility::CV.wait(lk,[]{return Utility::SNAP_READY;});
		Utility::SNAP_READY = false;
		
		if(_stop_serial_snap){
			break;
			printf("stopping serial snapshotting\n");
			_stop_serial_snap = false;
		}

		//generate the path:
		sprintf(path, "%s.%llX.%u", arg_path, (uint64_t)Utility::SNAP_SAMPLE, i);

		printf("Writing snapshot %s\n", path);
		//write the snapshot:
		buffer_ptr = _c_buffer.get_Sample( Utility::SNAP_SAMPLE -  samples_pr_snapshot);

		FILE *s_file;
		s_file = fopen(path,"wb");
		fwrite(buffer_ptr, sizeof(char), byte_size, s_file);
		fclose(s_file);
		printf("Snapshot written\n");	
		i++;

	}
	lk.unlock();
	_serial_snapshotting == false;
	

}

/**
 * Stop the serial snapshotting thread in a safely.
 */
void stop_serial_snapshot(){
	_stop_serial_snap = true;
	Utility::CV.notify_one();
}

/**
 * Takes a single snapshot, unlike the serial snapshotter, this does not need an overhead
 * as it will copy the buffer area neeed to another location in memory
 * and then writing those to a binary file. 
 * @param arg_sample_from start sample of the snapshot.
 * @param arg_sample_to end sample of the snapshot.
 * @param arg_path[] path of the snapshot.
 */
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

/**
 * Start the the ADC sampling thread, unless the external trigger command
 * is set it will begin sampling as soon as the usbdux device has been
 * initialized.
 * The final three arguments are very important to get right
 * as the recorder thread will use them to determine where to write it's samples and
 * when it's reached end of the buffer. It doesn't care about the c_buffer class
 * at all.
 * @param arg_device name of the comedi device e.g., 'comedi0'
 * @param arg_sample_rate the rate the usb_dux should sample with in hertz (max=3[Mhz]})
 * @param arg_start_address the start address of the circle buffer sample should be loading into
 * @param arg_end_address the end address of the cicle buffer.
 * @param arg_buffer_size byte size of the buffer.
 */
void start_recording(char arg_device[], uint32_t arg_sample_rate, 
		char* arg_start_address, char* arg_end_address, int arg_buffer_size){

	_recording = true;

	_recorder.start_Sampling(arg_device, arg_sample_rate, 
			arg_start_address, arg_end_address, arg_buffer_size);

}

/**
 * Stops the recording, this function needs to be called when exiting this program
 * unless you want kernel problems.
 */
void stop_recording(){
	_recorder.stop_Sampling();

	_recording = false;
}


/**
 * Writes an error text erro file to disk, filename is <path>.<data(HEX)>.
 * @param arg_msg the message that the error file should contain.
 * @param arg_path the path the file will be written to.
 */
void write_error(const char arg_msg[], const char arg_path[]){
	char path[256];
	//get the time:
	std::time_t result = std::time(NULL);

	sprintf(path, "%s.error.%X", arg_path, (uint32_t)result);
	std::ofstream file(path);
	file << arg_msg;
	file.close();	
}
