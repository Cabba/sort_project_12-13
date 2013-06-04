#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>
#include "executive.h"
#include "task-example.c"

////////
/// Code
////////
/*
bool sp_task_request()
  {
  //controlla slack time disponibile fino al periodo precedente alla deadline
  //schedulaione prima dei task periodici, lancio thread  
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
	wakeup_time = TIME_UNIT_IN_MILLI * MILLI_TO_NANO * H_PERIOD / NUM_FRAMES;
	printf("** FRAME LENGTH is: %d[ms]\n", (int)(wakeup_time/1E6));
	struct timespec time;

	int frame_counter = 0;

	// Setting the starting time.	
	gettimeofday(&zero,NULL);	
	time.tv_sec = zero.tv_sec;
	time.tv_nsec = zero.tv_usec * 1000; // Conversion between microsecond and nanosecond
	
	while( /*frame_counter < NUM_FRAMES*/ true ){
		// Print info
		print_current_time("FRAME_EXPIRED", zero);
		
		sp_task_request(frame_counter);
	
		pthread_cond_signal( &(frames[frame_counter].queue) );
	
		// Time managing
		pthread_mutex_lock( &mutex );				
		time.tv_sec += ( time.tv_nsec + wakeup_time ) / 1000000000;
		time.tv_nsec = ( time.tv_nsec + wakeup_time ) % 1000000000;
		pthread_cond_timedwait( &executive_data.queue, &mutex, &time );
		pthread_mutex_unlock( &mutex );
		
		// Check deadline		
		deadlinemiss_handler(frame_counter);
		
		frames[frame_counter].state = PENDING;
				
		++frame_counter; 
		frame_counter = frame_counter % NUM_FRAMES; 
	}

	return NULL;
}



/**
 * Start the real-time application.
 */
void start(){
	pthread_mutex_init( &mutex, NULL );
	
	sp_request = NONE;

	task_init();	

	threads_init();
	
	executive_init();
}



/**
 * Try to stop the real-time application.
 */
void stop(){	
	// Join the executive
	pthread_join( executive_data.thread , NULL );
	
	int i = 0;
	for( i = 0; i < NUM_FRAMES; ++i ){
		pthread_cond_destroy( &frames[i].queue );
		pthread_attr_destroy( &frames[i].thread_attr );
	}
	
	task_destroy();
	free( frames );

	pthread_cond_destroy( &executive_data.queue );
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
////////////////////
bool sp_task_request( int current_frame ){
	srand(time(NULL));
	if( rand() % 100 / 100 <= 0.6 && sp_request == NONE ){
		sp_request = CREATE;
		printf("**** SPORADIC REQUEST\n");		
	}
	int frame_number = SP_DLINE / (H_PERIOD / NUM_FRAMES);
	int available_execution = 0;
	int count = 0;
	while( count < frame_number ){
		available_execution += SLACK[current_frame + count];
		count++;
	}
	if( available_execution >= SP_WCET ){
		printf("** SPORADIC JOB ACCEPTED!!\n");
		return true;
	}
	return false;
}

void deadlinemiss_handler(int frame_id){
	frame_states_t state = frames[frame_id].state;
	if( state == BUSY ){
		printf("**!!** DEADLINE MISS **!!**\n");
		int it = frames[frame_id].iteration;
		printf("Uncompleted functions are: \n");
		while( SCHEDULE[frame_id][it] >= 0 ){
			printf("\t %d \n",SCHEDULE[frame_id][it] );
			++it;
		}
	} 
	if( state == PENDING ){
		printf("**!!** Thread isn't started **!!**\n");
	}
	if( state == PENDING || state == BUSY ){	
		printf("The program will terminate .. now!\n");	
		pthread_exit(NULL);	
		// TODO: liberare la memoria prima di fare exit(1)
		//exit(1);
	}
}

void print_thread_state(int thread_number){
	frame_states_t state = frames[thread_number].state;
	int function = SCHEDULE[thread_number][ frames[thread_number].iteration ];

	if( state == BUSY )
		printf("Thread %d state: BUSY with function %d -- DEADLINE MISS\n", thread_number, function);
	else if( state == IDLE )
		printf("Thread %d state: IDLE then function %d\n", thread_number, function);
        else
		printf("Thread %d state: PENDING\n", thread_number); 
	
}



void print_current_time(char* string, struct timeval origin){
	struct timeval now;
	gettimeofday(&now, NULL);
	printf("** %s - TIMING second: %ld, milliseconds: %ld\n",string, now.tv_sec - origin.tv_sec, (now.tv_usec - origin.tv_usec)/1000);
}



/**
 * Initialize all the threads as real-time threads.
 */
void threads_init(){
	// Memory allocation for threads
	frames = (frame_data_t*)malloc( sizeof(frame_data_t) * NUM_FRAMES );	
	
	int i = 0;	
	for( i = 0; i < NUM_FRAMES; ++i ){
		// Real-time settings
		pthread_attr_init( &frames[i].thread_attr );
		pthread_attr_setinheritsched( &frames[i].thread_attr, PTHREAD_EXPLICIT_SCHED );
		pthread_attr_setschedpolicy( &frames[i].thread_attr, SCHED_FIFO );
		frames[i].thread_param.sched_priority = sched_get_priority_max( SCHED_FIFO ) - 2;	
		pthread_attr_setschedparam( &frames[i].thread_attr, &frames[i].thread_param );
	
		frames[i].id = i;	
		// Condition variable creation
		pthread_cond_init( &frames[i].queue, NULL );	
	
		if( pthread_create( &frames[i].thread, &frames[i].thread_attr, frame_handler, (void*)&frames[i] ) != 0 )
			perror("Error in threads creations.");
	}
}



/**
 * Initialize the execitive thread as real-time thread.
 */
void executive_init(){
	pthread_cond_init( &executive_data.queue, NULL );	

	// Create the executive RT thread -------------
	pthread_attr_init( &executive_data.thread_attr );
	pthread_attr_setinheritsched( &executive_data.thread_attr, PTHREAD_EXPLICIT_SCHED );
	pthread_attr_setschedpolicy( &executive_data.thread_attr, SCHED_FIFO );
	executive_data.thread_param.sched_priority = sched_get_priority_max( SCHED_FIFO ) - 1;	
	pthread_attr_setschedparam( &executive_data.thread_attr, &executive_data.thread_param );

	if( pthread_create( &executive_data.thread, &executive_data.thread_attr, executive, NULL ) != 0 )
		perror("Executive creation. Not enought privilege.");
}



/**
 * Frame handler function.
 */
void* frame_handler(void *param){
	frame_data_t *frame_data = (frame_data_t*)param;
	printf( "Thread %d is sleeping.\n", frame_data->id );
	
	while( true ){
		pthread_mutex_lock( &mutex );
		pthread_cond_wait( &frame_data->queue, &mutex );
		pthread_mutex_unlock( &mutex );

		// Thread started ...	
		pthread_mutex_lock( &mutex );
		frame_data->state = BUSY;
		frame_data->iteration = 0;
		pthread_mutex_unlock( &mutex );


		// TODO: Aggiungere l'esecuzione ciclica.
		printf("* Thread %d is starting.\n", frame_data->id );	
		if( SCHEDULE[frame_data->id][0] < 0 )
			printf("Warning: the frame %d is empty!!\n", frame_data->id);	

		int i = 0;
		while( SCHEDULE[frame_data->id][i] >= 0 ){
			printf("\t<thread %d> iteration number %d, executing task %d\n", frame_data->id, i, SCHEDULE[frame_data->id][i]);
			frame_data->iteration = i;
			P_TASKS[ SCHEDULE[frame_data->id][i] ]();
			++i;
		}

		pthread_mutex_lock( &mutex );
		frame_data->state = IDLE;
		print_current_time("THREAD SETTED IDLE", zero );
		pthread_mutex_unlock( &mutex );
	}	
	
	return NULL; 
}










