#ifndef DATACHANNEL_H
#define DATACHANNEL_H

#include <cstring>
#include <pthread.h>

namespace Communication{

enum RET_CODE{
SUCCESS=0,
NO_FREE_SPACE,
NO_FRAME_AVAILABLE,
FAIL
};

typedef struct {
int frameCount ;
void *data ;
}QueueNode;

typedef struct {
 void *data;
 size_t size;
 }Frame;

class DataChannel{

public:
	DataChannel(const size_t &frameSize, const size_t capacity);
	~DataChannel();

	RET_CODE SendFrame(const Frame* ptrFrame);
	RET_CODE ReceiveFrame(Frame* outFrame,size_t &outSize);
	RET_CODE SendLastFrame();

private:
	size_t m_frameSize ;
	size_t m_capacity ;
	int front ;
	int rear ;
	char *m_shmPtr ;
	QueueNode *m_pQueue ;
	pthread_mutex_t m_lock ;
	int m_lastFrame ; 
};

}

#endif //DATACHANNEL_H
