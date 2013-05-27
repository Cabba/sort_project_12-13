/* traccia dell'executive (pseudocodice) */

#include <stdio.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>
//#include "task.h"

/*
bool sp_task_request()
  {
  // acceptance test etc. 
  }

void frame_handler(...)
  {
  }

void sp_task_handler(...)
  {
  }
*/
// Time declaration
int nanosec = 10E8;

// Threads
pthread_t exec_thread;

// Global mutex
pthread_mutex_t mutex;

// Executive waiting queue
pthread_cond_t exec_wait_queue;

void printCurrentTime(struct timeval origin){
	struct timeval now;
	gettimeofday(&now, NULL);
	printf("*TIMING second: %lu, milliseconds: %lu\n", now.tv_sec - origin.tv_sec, (now.tv_usec - origin.tv_usec)/1000);
}

void* executive(void* param){
	printf("***************** \nThe executive is started!!\n***************** \n");
	struct timespec time;
	struct timeval zero;

	int frame_counter = 0;

	// Setting the starting time.	
	gettimeofday(&zero,NULL);	
	time.tv_sec = zero.tv_sec;
	time.tv_nsec = zero.tv_usec * 1000; // Conversion between microsecond and nanosecond

	while( frame_counter < 10 ){
		//printf( "Frame number: %d\n",frame_counter);
		
		printCurrentTime(zero);
	
		pthread_mutex_lock( &mutex );				
		time.tv_sec += ( time.tv_nsec + nanosec ) / 1000000000;
		time.tv_nsec = ( time.tv_nsec + nanosec ) % 1000000000;
    	
		// The function pthread_cond_timedwait require a mutex section
		pthread_cond_timedwait( &exec_wait_queue, &mutex, &time );
		++frame_counter;
		pthread_mutex_unlock( &mutex );
	}

	return NULL;
}

void start(){
	// Initializing the executive sync varable and start the thread
	pthread_mutex_init( &mutex, NULL );
	pthread_cond_init( &exec_wait_queue, NULL );	

	// Create the executive RT thread
	pthread_attr_t exec_attr;
	struct sched_param exec_param;

	pthread_attr_init( &exec_attr );
	pthread_attr_setinheritsched( &exec_attr, PTHREAD_EXPLICIT_SCHED );
	pthread_attr_setschedpolicy( &exec_attr, SCHED_FIFO );
	exec_param.sched_priority = sched_get_priority_max( SCHED_FIFO ) - 1;
	
	pthread_attr_setschedparam( &exec_attr, &exec_param );

	if( pthread_create( &exec_thread, &exec_attr, executive, NULL ) != 0 )
		perror("Executive creation. Not enought privilege.");
}

void stop(){	
	// Join the executive
	pthread_join( exec_thread, NULL );

	pthread_cond_destroy( &exec_wait_queue );
	pthread_mutex_destroy( &mutex );
}

int main(int argc, char** argv){
	printf("The application will start now...\n");	
	start();
	stop();
	printf("End of the application...\n");	
	return 0;
}
