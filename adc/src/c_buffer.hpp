
/*
 * =====================================================================================
 *
 *   Filename:  c_buffer.hpp
 *
 *   Description:  this is the circular buffer, the type template dictates the data-type
 *		of samples it is used for, it's very important that size of type 
 *		is a multiple 4096, which is the std. pagesize of a posix system.
 *
 *   Created:  2013-11
 *   Revision:  none
 *   Compiler:  gcc(C11)
 *
 *   Author:  Soeren V. Joergensen, svjo@mmmi.sdu.dk
 *   Organization:  MMMI, University of Sourthern Denmark
 *
 * =====================================================================================
*/

#ifndef C_BUFFER_HPP
#define C_BUFFER_HPP

#include <sys/mman.h>
#include <cstdlib>
#include <unistd.h>
#include <err.h>
#include <stdio.h>
#include <string.h>

#include <string>

template <class Type>

class C_Buffer{

	public:

		C_Buffer(uint32_t arg_byte_size)
		:_write_offset(0),_read_offset(0)
		{
			//path = "dev/zero";
			int file_descriptor;

			char path[] = "/dev/shm/data_buffer-XXXXXX";
			char* address;

			file_descriptor = mkstemp(path);
			
			_path = path;
			
			if(file_descriptor < 0)
				errx(1, "error on generating buffer path");

			//_status = unlink(path);
			//if(_status)
			//	errx(1, "error on unlinking path");		

			//calculate byte_size, must be a multiple of 4096 and divisable by sample_size;
			printf("Wished for byte_size, %i\n", arg_byte_size);
			_byte_size = (arg_byte_size/4096) * 4096;
			//_byte_size = _byte_size/sizeof(Type) * sizeof(Type);
			printf("byte_size %i\n, that will be allocated", _byte_size);

			printf("total amount of samples available %f\n", (double)_byte_size/(double)sizeof(Type));
			_sample_amount = uint64_t(_byte_size/sizeof(Type));
			//optimization: Why calculate the same number twice? --mhs

			_status = ftruncate(file_descriptor, _byte_size);
			if(_status)
				errx(1, "error on truncation of file descriptor, to fit page size");

			_address = (char*)mmap(NULL, _byte_size << 1, PROT_NONE, 
					MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

			if(_address == MAP_FAILED){
				errx(1,"failed to map buffer currectly");
			}

			address = (char*)mmap(_address, _byte_size, PROT_READ | PROT_WRITE, 
					MAP_FIXED |	MAP_SHARED, file_descriptor, 0);

			_start_address = _address;
			_end_address = _address + _byte_size;

			if(address == MAP_FAILED){
				errx(1,"failed to map secondary buffer correctly");
			}

			address = (char*)mmap(_address + _byte_size, _byte_size, 
					PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, file_descriptor,0);

			if(address != _address + _byte_size)
				errx(1, "ring buffer does not eat it's own tail");

			_status = close(file_descriptor);
			if(_status)
				errx(1, "cannot close file!");
		}

		~C_Buffer(){
			_status = unlink(_path.c_str());
			if(_status)
				errx(1, "error on unlinking path");
			
			_status = munmap(_address, _byte_size << 1);
			if(_status)
				errx(1, "unable to delete memory mapping");
		}


		char* get_Sample(uint64_t arg_sample_number){
			//calculate index:
			uint64_t index = arg_sample_number;

			if(arg_sample_number > _sample_amount){				

				index = arg_sample_number % _sample_amount;				
				//printf("index is : %i, %i\n",index);
			}

			return _address + (index * sizeof(Type));
		}

		char* get_Start_Address(){
			return _start_address;			
		}

		char* get_End_Address(){
			return _end_address;
		}

		uint32_t get_Buffer_Size(){
			return _byte_size;
		}



	private:
		uint32_t _byte_size; //number of bytes the buffer fills in virtual memory:
		uint32_t _write_offset; //
		uint32_t _read_offset; //
		//unsigned long _count_bytes;
		char* _address; //
		char* _start_address; //
		char* _end_address;

		std::string  _path;

		uint32_t _status; //debug variable.
		uint32_t _sample_size; //byte size of each sample.
		uint64_t _sample_amount;

};
#endif // C_BUFFER_HPP
