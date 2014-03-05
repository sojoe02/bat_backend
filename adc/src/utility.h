#ifndef UTILITY_H
#define UTILITY_H

#include <condition_variable>
#include <mutex>
#include <atomic>

class Utility{
	public:	
		static uint64_t WRITTEN_SAMPLE;

		static std::atomic_uint WRITTEN_BLOCK;
		static std::atomic_uint SNAPSHOT_BLOCK_SIZE;

		static uint32_t SNAPSHOT_MAX_BYTE_SIZE;
		static uint32_t SNAPSHOT_BYTE_SIZE;

		static std::atomic_uint_least64_t LAST_SAMPLE;
		static std::atomic_uint_least64_t SNAP_SAMPLE;

		static std::condition_variable CV;
		static std::mutex LM;
		static bool SNAP_READY;

};


#endif //UTILITY_H
