
/*
 * =====================================================================================
 *
 *       Filename:  control.cpp
 *
 *    Description:  This is the comedi interface, that initializes the USBDUX-FAST unit
 *				it is designed to run on a seperate thread, sampling can be stoppen via
 *				an atomic boolean, allways stop sampling when restarting!
 *
 *        Created:  2013-11
 *       Revision:  none
 *       Compiler:  gcc (g++ C11 comptible)
 *
 *         Author:  Soeren V. Joergensen, svjo@mmmi.sdu.dk
 *   Organization:  MMMI, University of Sourthern Denmark
 *
 * =====================================================================================
 */



#ifndef RECORDER_HPP
#define RECORDER_HPP


#include <stdio.h>
#include <stdlib.h>
#include <comedilib.hpp>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <condition_variable>
#include <atomic>

#include "utility.h"

class Recorder{

	public:

		Recorder():
			_sample_rate(0), _channel_amount(0),_stop(false){
				
			}


		int start_Sampling(char* arg_device, uint32_t arg_sample_rate, 
				char* arg_start_address, char* arg_end_address, int arg_buffer_size){

			_channel_amount = 16;
			_sample_rate = arg_sample_rate;

			unsigned int chanlist[_channel_amount];
			unsigned int convert_arg = 1e9 / _sample_rate;
			int ret;

			comedi_cmd *cmd = (comedi_cmd *) calloc(1, sizeof(comedi_cmd));
			_device = comedi_open(arg_device);


			int buffer = 1048576;
			if(comedi_set_max_buffer_size(_device, 0, buffer) < 0 ){
				printf("Failed to set max buffer size to %ikB\n", buffer);
				return -1;
			} else printf("Maximum buffer size set to %iB\n", comedi_get_max_buffer_size(_device,0));

			if(comedi_set_buffer_size(_device, 0, buffer) < 0){
				printf("Failed to set buffer size to %iBytes\n", buffer);
				return -1;
			} else printf("Buffer size set to %iBytes\n", comedi_get_buffer_size(_device,0));

			if(!_device){
				errx(1, "unable to open device");
				comedi_perror("/dev/comedi0");
				return -1;
			}

			for(int i = 0; i < _channel_amount; i++)	
				chanlist[i] = i;

			comedi_set_global_oor_behavior(COMEDI_OOR_NUMBER);

			if((ret = comedi_get_cmd_generic_timed(_device, 0, cmd, _channel_amount,0)) < 0){
				printf("comedi_get_cmd_generic_timed failed\n");
				return ret;
			}

			cmd->chanlist 	= chanlist;
			cmd->stop_src	= TRIG_NONE;
			cmd->stop_arg	= 0;
			cmd->convert_arg = convert_arg;

			/* call test twice because different things are tested?
			 * if tests are successful run sampling command */
			if((ret = comedi_command_test(_device, cmd)) != 0
					|| (ret = comedi_command_test(_device, cmd)) != 0
					|| (ret = comedi_command(_device, cmd)) < 0){

				fprintf(stderr, "err: %d\n", ret);
				return -1;
			}

			FILE* dux_fp;

			if((dux_fp = fdopen(comedi_fileno(_device), "r")) <= 0)
				comedi_perror("fdopen");

			char* write_address = arg_start_address;

			Utility::WRITTEN_BLOCK = 0;
			Utility::ACTIVE_SAMPLE = 0;
			uint64_t active_sample = 0;
			uint32_t tmp_block = 0;
			int samples_pr_block = 4096/8;

			while((ret = fread(write_address, 1, 4096,dux_fp)) >= 0){

				write_address+=4096;
				//Utility::WRITTEN_BLOCK+=1;
				tmp_block+=1;
				active_sample += samples_pr_block;
				//Utility::ACTIVE_SAMPLE += samples_pr_block;

				if(tmp_block >= Utility::SNAPSHOT_BLOCK_SIZE){
					tmp_block = 0;
					Utility::ACTIVE_SAMPLE = active_sample;
					Utility::CV.notify_one();
				}

				if(write_address == arg_end_address){
					write_address -= arg_buffer_size;
					printf("resetting to beginning of buffer\n");
				}
			}

			if(ret < 0)
				perror("read");

			comedi_cancel(_device, 0);
			return 0;
		}

		void stop_Sampling(){
			_stop = true;
		}


	private:

		uint32_t _sample_rate;
		int _channel_amount;
		std::atomic_bool _stop;
		FILE* _dux_fp;
		comedi_t* _device;
};
#endif // RECORDER_HPP
