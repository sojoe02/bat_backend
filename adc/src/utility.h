#ifndef UTILITY_H 
#define UTILITY_H

#include <condition_variable>
#include <mutex>
#include <atomic>

class Utility{
	/**
	 * Static class that offers various shared variables.
	 */
	public:	
		static uint64_t WRITTEN_SAMPLE;

		static std::atomic_uint WRITTEN_BLOCK;
		static std::atomic_uint SNAPSHOT_BLOCK_SIZE;
		
		static uint32_t SNAPSHOT_MAX_BYTE_SIZE;
		static uint32_t SNAPSHOT_BYTE_SIZE;

		//the sample that was written to the buffer last:
		static std::atomic_uint_least64_t LAST_SAMPLE;

		//the sample that a snapshot can originate from:
		static std::atomic_uint_least64_t SNAP_SAMPLE;

		//condition variable to enable the recorder to signal when a 
		//snapshot is ready to be written to disk:
		static std::condition_variable CV;
		
		//mutex so only one serial_snapshot can be written at any one time:
		static std::mutex LM;
		
		//signal bool used by the condition variable CV:
		static bool SNAP_READY;
};


#endif //UTILITY_H
