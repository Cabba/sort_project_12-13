#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>
#include "executive.h"

////////
/// Code
////////
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

////////////////////
/// Public functions
////////////////////
/**
 * The executive function.
 */
void* executive(void* param){
	printf("***************** \nThe executive is started!!\n***************** \n");
	struct timespec time;
	struct timeval zero;

	int frame_counter = 0;

	// Setting the starting time.	
	gettimeofday(&zero,NULL);	
	time.tv_sec = zero.tv_sec;
	time.tv_nsec = zero.tv_usec * 1000; // Conversion between microsecond and nanosecond
	

	while( frame_counter < NUM_FRAMES ){
		// Print info
		print_current_time(zero);
	
		pthread_cond_signal( &threads_cond[frame_counter] );
	
		// Time managing
		pthread_mutex_lock( &mutex );				
		time.tv_sec += ( time.tv_nsec + nanosec ) / 1000000000;
		time.tv_nsec = ( time.tv_nsec + nanosec ) % 1000000000;
		pthread_cond_timedwait( &exec_wait_queue, &mutex, &time );
		++frame_counter;
		pthread_mutex_unlock( &mutex );
	}
	/*
	int i;	
	for(i = 0; i < NUM_FRAMES; ++i ){
		pthread_join( threads[i], NULL);
	}
	printf("* All the threads are join.\n");
	*/

	return NULL;
}

/**
 * Start the real-time application.
 */
void start(){
	pthread_mutex_init( &mutex, NULL );

	threads_init();
	executive_init();
}

/**
 * Try to stop the real-time application.
 */
void stop(){	
	// Join the executive
	pthread_join( exec_thread, NULL );
	
	int i = 0;
	for( i = 0; i < NUM_FRAMES; ++i )
		pthread_cond_destroy( &threads_cond[i] );
	free( threads_cond );	

	pthread_cond_destroy( &exec_wait_queue );
	pthread_mutex_destroy( &mutex );
}


int main(int argc, char** argv){
	printf("\n**************************\nThe application will start now...\n");	
	start();
	stop();
	printf("End of the application...\n***********************\n\n");	
	return 0;
}


/////////////////////
/// Private functions
/////////////////////

void print_current_time(struct timeval origin){
	struct timeval now;
	gettimeofday(&now, NULL);
	printf("*TIMING second: %lu, milliseconds: %lu\n", now.tv_sec - origin.tv_sec, (now.tv_usec - origin.tv_usec)/1000);
}

/**
 * Initialize all the threads as real-time threads.
 */
void threads_init(){
	threads_cond = (pthread_cond_t*) malloc( sizeof(pthread_cond_t) * NUM_FRAMES );
	threads = (pthread_t*) malloc( sizeof(pthread_t) * NUM_FRAMES );
	threads_index = (int*) malloc( sizeof(int) * NUM_FRAMES );

	pthread_attr_t frame_attr;
	struct sched_param frame_param;	
	int i = 0;	
	// Real-time settings
	pthread_attr_init( &frame_attr );
	pthread_attr_setinheritsched( &frame_attr, PTHREAD_EXPLICIT_SCHED );
	pthread_attr_setschedpolicy( &frame_attr, SCHED_FIFO );
	frame_param.sched_priority = sched_get_priority_max( SCHED_FIFO ) - 2;	
	pthread_attr_setschedparam( &frame_attr, &frame_param );

	for( i = 0; i < NUM_FRAMES; ++i ){
		threads_index[i] = i;	
		// Condition variable creation
		pthread_cond_init( &threads_cond[i], NULL );		
		if( pthread_create( &threads[i], &frame_attr, frame_handler, (void*)&threads_index[i] ) != 0 )
			perror("Error in threads creations.");
	}
}

/**
 * Initialize the execitive thread as real-time thread.
 */
void executive_init(){
	pthread_cond_init( &exec_wait_queue, NULL );	

	// Create the executive RT thread -------------
	pthread_attr_t exec_attr;
	struct sched_param exec_param;	
	// Real-time settings
	pthread_attr_init( &exec_attr );
	pthread_attr_setinheritsched( &exec_attr, PTHREAD_EXPLICIT_SCHED );
	pthread_attr_setschedpolicy( &exec_attr, SCHED_FIFO );
	exec_param.sched_priority = sched_get_priority_max( SCHED_FIFO ) - 1;	
	pthread_attr_setschedparam( &exec_attr, &exec_param );

	if( pthread_create( &exec_thread, &exec_attr, executive, NULL ) != 0 )
		perror("Executive creation. Not enought privilege.");
}

/**
 * Frame handler function.
 */
void* frame_handler(void *param){
	int *frame_pos = (int*)param;
	printf( "Thread %d is sleeping.\n", *frame_pos );
	pthread_mutex_lock( &mutex );
	pthread_cond_wait( &threads_cond[*frame_pos], &mutex );
	pthread_mutex_unlock( &mutex );
	printf("Thread %d is awake.\n", *frame_pos);
	return NULL;
}
