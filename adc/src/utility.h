#ifndef UTILITY_H
#define UTILITY_H

#include <condition_variable>
#include <mutex>

class Utility{
	public:	
		static uint64_t WRITTEN_SAMPLE;
		static uint32_t WRITTEN_BLOCK;
		static uint32_t SNAPSHOT_BLOCK_SIZE;
		static uint64_t ACTIVE_SAMPLE;
		static std::condition_variable CV;
		static std::mutex LM;

};


#endif // UTILITY_H
