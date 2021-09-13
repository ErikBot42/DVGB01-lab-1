// Written by Erik Magnusson and Rasmus Angeleni Gjein

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include <semaphore.h>
#include <time.h>
#include <errno.h>
#include <signal.h>


volatile int keepRunning = 1;

// Mutex vars
char *pBuffer;
int iBufferSize;
int iReadIndex=0;
int iWriteIndex=0;

// Semaphores and mutexes
pthread_mutex_t BufferMutex;
pthread_mutex_t IdMutex;
sem_t produceSemaphore; // number of writable spaces
sem_t consumeSemaphore; // number of things to read

void signalHandler(int iSignal) { 
	if (iSignal = SIGINT) {
		keepRunning=0; 
		sem_post(&consumeSemaphore); // To unlock consumers so that they can exit
	}
}

int iConsumerTimeInterVal = 0;

void print_buffer(char * arr, int size)
{
	for (int i=0;i<size;i++)
	{
		printf("%c ", arr[i]);
	}
	printf("\n");
}

void waitSec (int seconds)
{
	sleep(seconds);
	//clock_t now = clock();
	//clock_t next = now + CLOCKS_PER_SEC*seconds;
	//while (clock()<next);
}

void * consumer(void * arg)
{
	static int id;
	pthread_mutex_lock(&BufferMutex);
	int myId = id++;
	pthread_mutex_unlock(&BufferMutex);

	printf("created\n");
	do 
	{
		sem_wait(&consumeSemaphore);
		if (!keepRunning) break;

		pthread_mutex_lock(&BufferMutex);

		printf("reading value (id: %d): %c\n", myId, pBuffer[iReadIndex++]);
		iReadIndex%=iBufferSize;

		pthread_mutex_unlock(&BufferMutex);
		sem_post(&produceSemaphore);
		waitSec(iConsumerTimeInterVal);
	} while(keepRunning);
	printf("Thread %d is exiting.\n", myId);
	pthread_exit(0);
	return arg;
}

void produce () // fully thread safe, even if it will only run from one thread.
{
	sem_wait(&produceSemaphore);
	pthread_mutex_lock(&BufferMutex);

	static int i = 0;
	char value = 'A' + i++;
	i%=26;
	printf("writing value: %c\n", value);
	pBuffer[iWriteIndex++] = value;
	iWriteIndex%=iBufferSize;

	pthread_mutex_unlock(&BufferMutex);
	sem_post(&consumeSemaphore);
}

int main() 
{
	//printf("my id is: %d", pthread_getthreadid_np());
	signal(SIGINT, &signalHandler);
	pthread_mutex_init(&BufferMutex, NULL);
	pthread_mutex_init(&IdMutex, NULL);
	sem_init(&produceSemaphore, 0, 0);
	sem_init(&consumeSemaphore, 0, 0);

	int n;
	printf("Number of consumers: ");
	scanf("%d", &n);
	pthread_t consumers[n];

	printf("Buffer size: ");
	scanf("%d", &iBufferSize);

	int iTimeInterval;
	printf("Time interval (producer): ");
	scanf("%d", &iTimeInterval);

	printf("Time interval (consumer): ");
	scanf("%d", &iConsumerTimeInterVal);

	char buffer[iBufferSize];
	pBuffer = buffer;

	for (int i = 0; i<iBufferSize; i++)
	{
		sem_post(&produceSemaphore);
	}
	
	for (int i = 0; i<n; i++) 
	{
		pthread_t pthreadData;
		pthread_create(&pthreadData, NULL, consumer, buffer);
		consumers[i] = pthreadData;
	}

	while(keepRunning)
	{
		waitSec(iTimeInterval);
		produce();
	}
	
	printf("killing other threads\n");

	for (int i = 0; i<n; i++) 
	{
		pthread_kill(consumers[i], SIGINT);
		pthread_join(consumers[i], NULL);
	}
	return 0;
}


