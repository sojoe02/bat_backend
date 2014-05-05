/*
	Simple Network Library from "Networking for Game Programmers"
	http://www.gaffer.org/networking-for-game-programmers
	Author: Glenn Fiedler <glenn.fiedler@gmail.com>
	Edited by Malthe Hoej-Sunesen <mhoej10@student.sdu.dk>; only lines with --mhs
*/

// For "how to use", see bottom.

#ifndef NET_H
#define NET_H

// platform detection

#define PLATFORM_WINDOWS  1
#define PLATFORM_MAC      2
#define PLATFORM_UNIX     3

#if defined(_WIN32)
#define PLATFORM PLATFORM_WINDOWS
#elif defined(__APPLE__)
#define PLATFORM PLATFORM_MAC
#else
#define PLATFORM PLATFORM_UNIX
#endif

#if PLATFORM == PLATFORM_WINDOWS

	#include <winsock2.h>
	#pragma comment( lib, "wsock32.lib" )

#elif PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX

	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <fcntl.h>

#else

	#error unknown platform!

#endif

#define SENDING_PORT	5028 //--mhs
#include <assert.h>
#include <stdint.h>
#include <thread> //--mhs
#include <mutex> //--mhs
#include <stdlib.h> //--mhs - added "just in case"
#include <unistd.h>

using namespace std;

namespace net
{
	// platform independent wait for n seconds

/*#if PLATFORM == PLATFORM_WINDOWS

	void wait( float seconds )
	{
		Sleep( (int) ( seconds * 1000.0f ) );
	}

#else

	#include <unistd.h>
	void wait( float seconds ) { usleep( (int) ( seconds * 1000000.0f ) ); }

#endif*/

	// internet address

	std::mutex socket_mutex;
	class Address
	{
	public:
	
		Address()
		{
			address = 0;
			port = 0;
		}
	
		Address( unsigned char a, unsigned char b, unsigned char c, unsigned char d, unsigned short port )
		{
			this->address = ( a << 24 ) | ( b << 16 ) | ( c << 8 ) | d;
			this->port = port;
		}
	
		Address( unsigned int address, unsigned short port )
		{
			this->address = address;
			this->port = port;
		}
	
		unsigned int GetAddress() const
		{
			return address;
		}
	
		unsigned char GetA() const
		{
			return ( unsigned char ) ( address >> 24 );
		}
	
		unsigned char GetB() const
		{
			return ( unsigned char ) ( address >> 16 );
		}
	
		unsigned char GetC() const
		{
			return ( unsigned char ) ( address >> 8 );
		}
	
		unsigned char GetD() const
		{
			return ( unsigned char ) ( address );
		}
	
		unsigned short GetPort() const
		{ 
			return port;
		}
	
		bool operator == ( const Address & other ) const
		{
			return address == other.address && port == other.port;
		}
	
		bool operator != ( const Address & other ) const
		{
			return ! ( *this == other );
		}
	
	private:
	
		unsigned int address;
		unsigned short port;
	};

	// sockets

	inline bool InitializeSockets()
	{
		#if PLATFORM == PLATFORM_WINDOWS
	    WSADATA WsaData;
		return WSAStartup( MAKEWORD(2,2), &WsaData ) != NO_ERROR;
		#else
		return true;
		#endif
	}

	inline void ShutdownSockets()
	{
		#if PLATFORM == PLATFORM_WINDOWS
		WSACleanup();
		#endif
	}

	class Socket
	{
	public:
	
		Socket()
		{
			socket = 0;
		}
	
		~Socket()
		{
			Close();
		}
	
		bool Open( unsigned short port )
		{
			assert( !IsOpen() );
		
			// create socket

			socket = ::socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );

			if ( socket <= 0 )
			{
				printf( "failed to create socket\n" );
				socket = 0;
				return false;
			}

			// bind to port

			sockaddr_in address;
			address.sin_family = AF_INET;
			address.sin_addr.s_addr = INADDR_ANY;
			address.sin_port = htons( (unsigned short) port );
		
			if ( bind( socket, (const sockaddr*) &address, sizeof(sockaddr_in) ) < 0 )
			{
				printf( "failed to bind socket\n" );
				Close();
				return false;
			}

			// set non-blocking io

			#if PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX
		
				int nonBlocking = 1;
				if ( fcntl( socket, F_SETFL, O_NONBLOCK, nonBlocking ) == -1 )
				{
					printf( "failed to set non-blocking socket\n" );
					Close();
					return false;
				}
			
			#elif PLATFORM == PLATFORM_WINDOWS
		
				DWORD nonBlocking = 1;
				if ( ioctlsocket( socket, FIONBIO, &nonBlocking ) != 0 )
				{
					printf( "failed to set non-blocking socket\n" );
					Close();
					return false;
				}

			#endif
		
			return true;
		}
	
		void Close()
		{
			if ( socket != 0 )
			{
				#if PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX
				close( socket );
				#elif PLATFORM == PLATFORM_WINDOWS
				closesocket( socket );
				#endif
				socket = 0;
			}
		}
	
		bool IsOpen() const
		{
			return socket != 0;
		}
	
		bool Send( const Address & destination, const void * data, int size )
		{
			assert( data );
			assert( size > 0 );
		
			if ( socket == 0 )
				return false;
		
			sockaddr_in address;
			address.sin_family = AF_INET;
			address.sin_addr.s_addr = htonl( destination.GetAddress() );
			address.sin_port = htons( (unsigned short) destination.GetPort() );

			int sent_bytes = sendto( socket, (const char*)data, size, 0, (sockaddr*)&address, sizeof(sockaddr_in) );

			return sent_bytes == size;
		}
	
		int Receive( Address & sender, void * data, int size )
		{
			assert( data );
			assert( size > 0 );
		
			if ( socket == 0 )
				return false;
			
			#if PLATFORM == PLATFORM_WINDOWS
			typedef int socklen_t;
			#endif
			
			sockaddr_in from;
			socklen_t fromLength = sizeof( from );

			int received_bytes = recvfrom( socket, (char*)data, size, 0, (sockaddr*)&from, &fromLength );

			if ( received_bytes <= 0 )
				return 0;

			unsigned int address = ntohl( from.sin_addr.s_addr );
			unsigned int port = ntohs( from.sin_port );

			sender = Address( address, port );

			return received_bytes;
		}
		
	private:
	
		int socket;
	};
	
	class BatUDP // Entire class by --mhs
	{
	public:
		BatUDP()	{
			_callInitializeSockets(SENDING_PORT);
			_addr = Address(192,168,1,139, SENDING_PORT);
		}
		
		BatUDP(int aaa, int bbb, int ccc, int ddd, short port)	{
			_callInitializeSockets(SENDING_PORT);
			_addr = Address(aaa, bbb, ccc, ddd, port);
		}
		
		~BatUDP()	{
			ShutdownSockets();
		}
		
		void start_recording(char* arg_device, uint32_t arg_sample_rate, 
				char* arg_start_address) { 
			//	printf("Attempting to launch thread for sending <start> message\n");
			//	std::thread BatUDP_t1(&BatUDP::_start_recording_thread, this, arg_device, arg_sample_rate, arg_start_address);
			//	BatUDP_t1.detach();
			printf("Checkpoint 2a\n");
			char sample_rate[16];
			printf("Checkpoint 2\n");
			snprintf(sample_rate, sizeof(sample_rate), "%lu", (unsigned long)arg_sample_rate);
			string tmp = "csnap\t" + string(arg_device) + "\t" + string(sample_rate) + "\t" + string(arg_start_address);
			printf("Checkpoint 3\n");
			const char* data[] = tmp.c_str();
			printf("Checkpoint 4\n");
			socket_mutex.lock();
			//printf("Attempting to send <start>\n");
			printf("Checkpoint 5\n");
			socket.Send(_addr, data, sizeof(data));
			printf("<start> should have been sent by now\n");
			socket_mutex.unlock();
		}
		
		void stop_recording(char* arg_device, uint32_t arg_sample_rate, 
				char* arg_start_address, char* arg_end_address) {
			//	printf("Attempting to launch thread for sending <stop> message\n");
				std::thread BatUDP_t2(&BatUDP::_stop_recording_thread, this, arg_device, arg_sample_rate, arg_start_address,
					arg_end_address);
				BatUDP_t2.detach();
		}
		
	private:
		
		Address _addr;
		Socket socket;
		
		void _callInitializeSockets(short _port) {
			printf("Creating outgoing socket on port %i\n", _port);
			if(!InitializeSockets())	{
				printf("Failed to initialize sockets. Program cannot continue.\n");
				exit(1);
			}
		}
			
		/*void _start_recording_thread(char* arg_device, uint32_t arg_sample_rate, 
			char* arg_start_address) {
			//printf("Thread for sending <start> created.\n");
			char sample_rate[16];
			snprintf(sample_rate, sizeof(sample_rate), "%lu", (unsigned long)arg_sample_rate);
			string tmp = "csnap\t" + string(arg_device) + "\t" + string(sample_rate) + "\t" + string(arg_start_address);
			const char data[] = tmp.c_str();
			socket_mutex.lock();
			//printf("Attempting to send <start>\n");
			socket.Send(_addr, data, sizeof(data));
			printf("<start> should have been sent by now\n");
			socket_mutex.unlock();
			//TODO: Check in Perl (array-cmd.sdx) how the command is received and how much I can add to it.
			//TODO: See if this is good enough. Whitespace delimiter is as per Perl script: 
				//$msg =~ s/\s#\s+/ #/g;	# Attach prefix # to following argument, if separated by whitespace
				//my @msg = split /\s+/, $msg;
		}*/
		
		void _stop_recording_thread(char* arg_device, uint32_t arg_sample_rate, 
			char* arg_start_address, char* arg_end_address)	{
			//printf("Thread for sending <stop> created.\n");
			char sample_rate[16];
			snprintf(sample_rate, sizeof(sample_rate), "%lu", (unsigned long)arg_sample_rate);
			string tmp = "cstop\t" + string(arg_device) + "\t" + string(sample_rate) + "\t" + string(arg_start_address) + "\t" + string(arg_end_address);
			const char data[] = tmp.c_str();
			socket_mutex.lock();
			//printf("Attempting to send <stop>\n");
			socket.Send(_addr, data, sizeof(data));
			printf("<stop> should have been sent by now\n");
			socket_mutex.unlock();
			//TODO: See if this is good enough.
		}
				
	};
}
#endif

/*
How to use this file:

	Sending and Receiving Packets Example (Simple version!)
	From "Networking for Game Programmers" - http://www.gaffer.org/networking-for-game-programmers
	Author: Glenn Fiedler <glenn.fiedler@gmail.com>


#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "Net.h"

using namespace std;
using namespace net;

int main( int argc, char * argv[] )
{
	// initialize socket layer

	if ( !InitializeSockets() )
	{
		printf( "failed to initialize sockets\n" );
		return 1;
	}
	// create socket
	

	int port = 30000;

	printf( "creating socket on port %d\n", port );

	Socket socket;

	if ( !socket.Open( port ) )
	{
		printf( "failed to create socket!\n" );
		return 1;
	}

	// send and receive packets to ourself until the user ctrl-breaks...

	while ( true )
	{
		const char data[] = "hello world!";

		socket.Send( Address(127,0,0,1,port), data, sizeof(data) );
			
		while ( true )
		{
			Address sender;
			unsigned char buffer[256];
			int bytes_read = socket.Receive( sender, buffer, sizeof( buffer ) );
			if ( !bytes_read )
				break;
			printf( "received packet from %d.%d.%d.%d:%d (%d bytes)\n", 
				sender.GetA(), sender.GetB(), sender.GetC(), sender.GetD(), 
				sender.GetPort(), bytes_read );
		}
		
		wait( 0.25f );
	}
	
	// shutdown socket layer
	
	ShutdownSockets();

	return 0;
}
*/