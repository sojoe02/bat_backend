
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

		C_Buffer(unsigned long long arg_byte_size)
		:_write_offset(0),_read_offset(0)
		{
			//path = "dev/zero";
			int file_descriptor;


			char path[] = "/dev/shm/data_buffer-XXXXXX";
			Type* address;

			file_descriptor = mkstemp(path);
			
			_path = path;
			
			if(file_descriptor < 0)
				errx(1, "error on generating buffer path");

			//_status = unlink(path);
			//if(_status)
			//	errx(1, "error on unlinking path");		

			//calculate byte_size, must be a multiple of 4096 and divisable by sample_size;
			printf("Wished for byte_size, %i\n", arg_byte_size);
			_byte_size = arg_byte_size/4096 * 4096;
			_byte_size = _byte_size/sizeof(Type) * sizeof(Type);
			printf("byte_size %i\n, that will be allocated", _byte_size);

			printf("total amount of samples available %f\n", (double)_byte_size/(double)sizeof(Type));
			_sample_amount = _byte_size/sizeof(Type);

			_status = ftruncate(file_descriptor, _byte_size);
			if(_status)
				errx(1, "error on truncation of file descriptor, to fit page size");

			_address = (Type*)mmap(NULL, _byte_size << 1, PROT_NONE, 
					MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

			if(_address == MAP_FAILED){
				errx(1,"failed to map buffer currectly");
			}

			address = (Type*)mmap(_address, _byte_size, PROT_READ | PROT_WRITE, 
					MAP_FIXED |	MAP_SHARED, file_descriptor, 0);

			if(address == MAP_FAILED){
				errx(1,"failed to map secondary buffer correctly");
			}

			address = (Type*)mmap(_address + _byte_size, _byte_size, 
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


		void write_Samples(Type arg_sample){
			//memcpy(_address ,arg_samples, arg_num_bytes);

			if(_write_offset == _byte_size/sizeof(Type))
				_write_offset = 0;

			*(_address + _write_offset) = arg_sample;
			//printf("%d\n",(int)*(_address + _write_offset));
			_write_offset++;
		}

		void print_Buffer(){

			//for(int i = 0; i<_byte_size/sizeof(Type); i++){
				//printf("sample %d, %d \n", i,(Type)_address[i]);
			//}

		}

		Type* get_Sample(unsigned long long sample_number){
			//calculate index:
			unsigned long long index = sample_number;
			if(sample_number > _sample_amount){				

				index = sample_number % _sample_amount;
				
				printf("index is : %i, %i\n",index);
			}

			return (_address + index);
		}



	private:
		int _byte_size; //number of bytes the buffer fills in virtual memory:
		unsigned long long _read_offset; //
		int _write_offset; //
		//unsigned long _count_bytes;
		Type* _address; //
		std::string  _path;

		int _status; //debug variable.
		int _sample_size; //byte size of each sample.
		unsigned long long _sample_amount;

};
#endif // C_BUFFER_HPP