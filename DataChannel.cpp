#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <iostream>
#include <errno.h>

#include "DataChannel.h"

#define SHM_FRAMEDATA "/sharedFrameData"

namespace Communication{

DataChannel::DataChannel(const size_t &frameSize, const size_t capacity)
		try : m_frameSize(frameSize),m_capacity(capacity),front(-1),rear(-1),m_shmPtr(NULL),m_pQueue(NULL),m_lastFrame(-1)
{
	int shm_fd = -1 ;
	size_t shmSize = frameSize * capacity ;

	shm_fd = shm_open(SHM_FRAMEDATA,O_CREAT | O_RDWR, 0666);

	if( shm_fd == -1)
	{
		throw std::string("shared memory create failed!!");
	}
	
	if (ftruncate(shm_fd, shmSize) == -1)
	{
		throw std::string("ftruncate failed!!");
	}

	m_shmPtr = static_cast<char*> (mmap(NULL, shmSize, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0));
	if (m_shmPtr == MAP_FAILED)
	{
		throw std::string("shared memory mapping failed!!");
	}

	close(shm_fd);
	//intialize queue.
	m_pQueue = new QueueNode[m_capacity];
	if(m_pQueue)
	{
		for(int i = 0 ; i < m_capacity ; ++i)
		{
			m_pQueue[i].data = m_shmPtr + (i * frameSize) ; 
			m_pQueue[i].frameCount = 0 ;
		}
	}
	//initialize mutex
	if(pthread_mutex_init(&m_lock, NULL) != 0 )
	{
		throw std::string("mutex init failed!!");
	}
}
catch(std::string& errorMsg)
{
	if(m_pQueue)
		delete[] m_pQueue;
	//unlink shared memory.
	shm_unlink(SHM_FRAMEDATA);
	std::cout << errorMsg << std::endl;
}	

DataChannel::~DataChannel()
{
	//delete queue array.
	if(m_pQueue)
		delete[] m_pQueue;
	//unlink shared memory.
	shm_unlink(SHM_FRAMEDATA);

	pthread_mutex_destroy(&m_lock);
	
}

RET_CODE DataChannel::SendFrame(const Frame* ptrFrame)
{
	RET_CODE retVal = FAIL ;

	//if user passed a NULL pointer
	if(	!ptrFrame)
	{
		std::cout << "Invalid pointer" << std::endl;
		return retVal ;
	}

	if( m_frameSize < ptrFrame->size)
	{
		std::cout << "Input frame is bigger than the supported frame size." << std::endl;
		return retVal ;
	}
	int local_front , local_rear;

	//below operation should be performed atomically.
	pthread_mutex_lock(&m_lock);
	local_front = front ;
	local_rear = rear ;
	pthread_mutex_unlock(&m_lock);

	if( local_front == (local_rear + 1) % m_capacity)
	{
		//Producer is notified using this value that consumer is very slow so adding more frames is not possible.
		retVal = NO_FREE_SPACE ;
	}
	else
	{
		//new rear position.
		local_rear = (local_rear + 1) % m_capacity ;
		
		//copy frame data
		memcpy(m_pQueue[local_rear].data, ptrFrame->data, m_frameSize);
				
		pthread_mutex_lock(&m_lock);
		
		m_pQueue[local_rear].frameCount = 1 ;

		if( front == -1)
			front = 0;

		rear = local_rear ; 
		m_lastFrame = rear;
		pthread_mutex_unlock(&m_lock);
		//storing the index of last frame added.
		

		retVal = SUCCESS;
	}
	
	return retVal;
}


RET_CODE DataChannel::ReceiveFrame(Frame* outFrame,size_t &outSize)
{
	RET_CODE retVal = FAIL ;

	//if user passed a NULL pointer
	if(	!outFrame || outFrame->data == NULL)
	{
		std::cout << "Invalid pointer" << std::endl;
		return retVal ;
	}
	int local_front , local_rear;

	//below operation should be performed atomically.
	pthread_mutex_lock(&m_lock);
	local_front = front ;
	local_rear = rear ;
	pthread_mutex_unlock(&m_lock);	

	if( local_front == local_rear && local_rear == -1)
	{
		 retVal = NO_FRAME_AVAILABLE ;
	}
	else
	{
		//copy next frame data.
		if( m_pQueue[local_front].frameCount > 0 )
		{
			memcpy(outFrame->data, m_pQueue[local_front].data, m_frameSize);
			//if all frames have been consumed then reset front and rear.
		
			pthread_mutex_lock(&m_lock);
			//decrease the frame count
			m_pQueue[local_front].frameCount-- ;
		
			//if all copies of this frame are read.
			if( m_pQueue[local_front].frameCount == 0 )
			{
				if( front == rear)
					front = rear = -1 ;
				else
					front = (front + 1) % m_capacity ;
			}
			pthread_mutex_unlock(&m_lock);

			//set the size of the copied frame
			outSize =  m_frameSize;
			retVal = SUCCESS; 
		}
	}
	return retVal;

}

RET_CODE DataChannel::SendLastFrame()
{
	RET_CODE retVal = FAIL ;
	//Case when all frames have been received.
	pthread_mutex_lock(&m_lock);
	// if no frame was sent before this call then it will fail
	if(m_lastFrame < 0 )
	{
		return retVal ;
	}
	if( front == rear && rear == -1)
	{
		front = rear = m_lastFrame ;
		m_pQueue[m_lastFrame].frameCount = 1 ;
	}
	else
	{
		//mark multiple occurence of last frame.		
		m_pQueue[m_lastFrame].frameCount = m_pQueue[m_lastFrame].frameCount + 1 ;
	}
	pthread_mutex_unlock(&m_lock);	
	retVal = SUCCESS;  	
	return retVal;
}


}
