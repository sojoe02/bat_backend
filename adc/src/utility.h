#ifndef UTILITY_H
#define UTILITY_H

#include <condition_variable>

class Utility{
	public:	
		static uint64_t WRITTEN_SAMPLE;
		static uint32_t WRITTEN_BLOCK;
		static uint32_t SNAPSHOT_BLOCK_SIZE;
		static std::condition_variable CV;

};


#endif // UTILITY_H
