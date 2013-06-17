#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>
#include <stdbool.h>

// Application specific library
#include "executive.h"
#include "task-example.c"
#include "task.h"

#ifndef TIME_UNIT_IN_MILLI
	#define TIME_UNIT_IN_MILLI 10
#endif // TIME_UNIT_IN_MILLI

#ifndef MILLI_TO_NANO
	#define MILLI_TO_NANO 1E6
#endif // MILLI_TO_NANO

#ifndef SP_REQUEST_PROBABILITY
	#define SP_REQUEST_PROBABILITY 0.1
#endif // SP_REQUEST_PROBABILITY

///////////////////
/// Data structures
///////////////////
// Frame states
typedef enum{ BUSY, IDLE, PENDING } frame_states_t;

// Generical thread data
typedef struct thread_data_t_{
	pthread_t 			thread;
	pthread_cond_t 		queue;
	pthread_attr_t 		attr;
	struct sched_param 	param; 
} thread_data_t; 	

// Frame data
typedef struct frame_data_t_ {
	int 				id;
	int 				iteration;
	frame_states_t 		state; 
	thread_data_t 		thread_data;
} frame_data_t;

// Sporadic task data
typedef struct sp_data_t_{
	thread_data_t 		thread_data;
	frame_states_t 		state;
	int 				deadline_counter;
} sp_data_t;

////////////////////
/// Global variables
////////////////////
// Time origin
struct timeval zero;

// Global mutex
pthread_mutex_t mutex;

// Frames datai, ogni frame deve accedere al proprio e EXECUTIVE
frame_data_t *frames;

// Executive data, solo EXECUTIVE
thread_data_t executive_data;

// Sporadic task data, devono accedervi EXECUTIVE, SPORADIC
sp_data_t sp_data;

// The current number of the frame
int frame_counter;

////////////////////////
/// Functions prototypes
////////////////////////

/// HANDLERS

/**
 * Function for the executive handling
 */
void* executive( void* param );

/**
 * Function for the frame handling
 */
void* frame_handler( void* param );

/**
 * Function for sporadic task handling
 */
void* sp_task_handler( void* param );

/**
 * The handler called if a perdiodic task deadlinemiss occour.
 */
 void deadlinemiss_handler(int frame_id );

/// INITIALIZAZION FUNCTIONS

/**
 * Executive initialization
 */
void executive_init();

/**
 * Frames task initialization
 */
void frames_init();

/**
 * Sporadic task initialization.
 */
 void sp_init();

/// UTILS

/**
 * Start the application
 */
void start();

/**
 * Gracefull application shutdown
 */
void stop();

/**
 * Check is the sporadic task can be accepted considering
 * his WCET and his DEADLINE
 */
bool sp_task_request();

/**
 * Immediate program termination
 */
void shutdown();
 
 /**
  * Function for timed debug.
  */
void print_current_time( char* string, struct timeval origin );


//////////////////
/// MAIN FUNCTIONS
//////////////////

int main(int argc, char** argv){
		printf("\n********************\nThe application will start now...\n");	
	srand(time(NULL));
	
	// Start procedures
	start();
	// Stop procedures
	stop();

	printf("End of the application...\n********************\n\n");	
	return 0;
}

/////////
/// UTILS
/////////

void start(){
	pthread_mutex_init( &mutex, NULL );
	
	task_init();	

	sp_init();

	frames_init();
	
	executive_init();
}

void stop(){	
	// Join the executive
	pthread_join( executive_data.thread , NULL );
	// Join the sporadic task
	pthread_join( sp_data.thread_data.thread, NULL );	

	int i = 0;
	for( i = 0; i < NUM_FRAMES; ++i ){
		pthread_join( frames[i].thread_data.thread, NULL );
		pthread_cond_destroy( &frames[i].thread_data.queue );
		pthread_attr_destroy( &frames[i].thread_data.attr );
	}
	
	task_destroy();
	free( frames );

	pthread_cond_destroy( &executive_data.queue );
	pthread_mutex_destroy( &mutex );
}

void shutdown(){
	task_destroy();
	free( frames );
	exit(1);
}

bool sp_task_request(){

	pthread_mutex_lock(&mutex);
	if( sp_data.state == IDLE ){
		sp_data.state = PENDING;
		printf("** SPORADIC REQUEST\n");		
	}
	pthread_mutex_unlock(&mutex);

	if( sp_data.state == PENDING ){ 
		int frame_number = SP_DLINE / (H_PERIOD / NUM_FRAMES);
		int available_execution = 0;
		int count = 0;
		while( count < frame_number ){
			available_execution += SLACK[(frame_counter + count) % NUM_FRAMES];
			count++;
		}
		if( available_execution >= SP_WCET && sp_data.state == PENDING ){
			printf("** SPORADIC JOB ACCEPTED!!\n");			
			return true;
		}
		printf("** SPORADIC JOB REJECTED - SP_WCET = %d, AVAILABLE EXECUTION = %d, on %d FRAMES\n", SP_WCET, available_execution, frame_number);

		pthread_mutex_lock(&mutex);
		sp_data.state = IDLE;
		pthread_mutex_unlock(&mutex);

	}
	return false;
}

void print_current_time(char* string, struct timeval origin){
	struct timeval now;
	gettimeofday(&now, NULL);
	printf("**** %s - TIMING second: %ld, milliseconds: %ld\n",string, now.tv_sec - origin.tv_sec, (now.tv_usec - origin.tv_usec)/1000);
}


////////////////////////////
/// INITIALIZATION FUNCTIONS
////////////////////////////

void frames_init(){
	// Memory allocation for threads
	frames = (frame_data_t*)malloc( sizeof(frame_data_t) * NUM_FRAMES );	
	
	int i = 0;	
	for( i = 0; i < NUM_FRAMES; ++i ){
		// Real-time setting
		pthread_attr_init( &frames[i].thread_data.attr );
		pthread_attr_setinheritsched( &frames[i].thread_data.attr, PTHREAD_EXPLICIT_SCHED );
		pthread_attr_setschedpolicy( &frames[i].thread_data.attr, SCHED_FIFO );
		frames[i].thread_data.param.sched_priority = sched_get_priority_max( SCHED_FIFO ) - 2;	
		pthread_attr_setschedparam( &frames[i].thread_data.attr, &frames[i].thread_data.param );
		// Set frame id	
		frames[i].id = i;	
		pthread_cond_init( &frames[i].thread_data.queue, NULL );	
	
		if( pthread_create( &frames[i].thread_data.thread, &frames[i].thread_data.attr, frame_handler, (void*)&frames[i] ) != 0 )
			perror("Error in threads creations.");
	}
}

void sp_init(){
	// Sporadic task creation
	pthread_attr_init( &sp_data.thread_data.attr );
	pthread_attr_setinheritsched( &sp_data.thread_data.attr, PTHREAD_EXPLICIT_SCHED );
	pthread_attr_setschedpolicy( &sp_data.thread_data.attr, SCHED_FIFO );
	// Mettiamo il thread del task sporadico ad una priorità minore 
	// di tutti gli altri threads in modo che non ci sia da farlo dormire 
	// su una variabile di condizione
	sp_data.thread_data.param.sched_priority = sched_get_priority_max( SCHED_FIFO ) - 3;	
	pthread_attr_setschedparam( &sp_data.thread_data.attr, &sp_data.thread_data.param );	
	pthread_cond_init( &sp_data.thread_data.queue, NULL );	
	if( pthread_create( &sp_data.thread_data.thread, &sp_data.thread_data.attr, sp_task_handler , (void*)&sp_data ) != 0 )
		perror("Error in sporadic job creations.");	
}

void executive_init(){
	pthread_cond_init( &executive_data.queue, NULL );	

	// Create the executive RT thread -------------
	pthread_attr_init( &executive_data.attr );
	pthread_attr_setinheritsched( &executive_data.attr, PTHREAD_EXPLICIT_SCHED );
	pthread_attr_setschedpolicy( &executive_data.attr, SCHED_FIFO );
	executive_data.param.sched_priority = sched_get_priority_max( SCHED_FIFO ) - 1;	
	pthread_attr_setschedparam( &executive_data.attr, &executive_data.param );

	if( pthread_create( &executive_data.thread, &executive_data.attr, executive, NULL ) != 0 )
		perror("Executive creation. Not enought privilege.");
}

////////////
/// HANDLERS
////////////

void* sp_task_handler(void *param){
	sp_data_t *sp = (sp_data_t*)param;
	printf("Sporadic job is sleeping!\n");
	while( true ){
		pthread_mutex_lock( &mutex );
		sp_data.state = IDLE;
		print_current_time(" -!!!!!!- SPORADIC JOB TERMINATED", zero);
		while( sp_data.state == IDLE ){
			pthread_cond_wait( &sp->thread_data.queue, &mutex );
		}
		pthread_mutex_unlock( &mutex );	
		printf("Sporadic job is awake!\n");
		SP_TASK();
	}
	return NULL;
}

void* frame_handler(void *param){
	frame_data_t *frame_data = (frame_data_t*)param;
	printf( "Thread %d is sleeping.\n", frame_data->id );

	// Loop infinito per l'esecuzione dei task del frame.	
	while( true ){
		pthread_mutex_lock( &mutex );
		while( frame_data->state != PENDING ){
			pthread_cond_wait( &frame_data->thread_data.queue, &mutex );
		}
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

void* executive(void* param){
	printf("***************** \nThe executive is started!!\n***************** \n");

	int high_priority = sched_get_priority_max( SCHED_FIFO ) - 2;	
	int low_priority = sched_get_priority_max( SCHED_FIFO ) - 3;	
	
	unsigned int wakeup_time = TIME_UNIT_IN_MILLI * MILLI_TO_NANO * H_PERIOD / NUM_FRAMES;
	unsigned int slack_wakeup_time = 0;

	printf("** FRAME LENGTH is: %d[ms]\n", (int)(wakeup_time/1E6));
	struct timespec time;
	struct timespec slack_time;

	frame_counter = 0;

	// Setting the starting time.	
	gettimeofday(&zero,NULL);	
	time.tv_sec = zero.tv_sec;
	time.tv_nsec = zero.tv_usec * 1000; // Conversion between microsecond and nanosecond
	
	while( true ){
		// Print info
		printf("===============================(Frame %d started)==============================\n", frame_counter);
		print_current_time("FRAME_STARTED", zero);
	
		// Settiamo il frame corrente allo stato PENDING, in modo che alla fine del frame il deadlinemiss_handler
		// controlli lo stato del frame. Se il frame viene trovato PENDING vuole dire che non è stato schedulato dal sistema.
		// Il che rappresenta una condizione ancora più critica rispetto alla "semplice" deadlinemiss, il programma verrà quindi terminato.	
		frames[frame_counter].state = PENDING;
		
		// Sporadic task creation and initialization
		if( sp_data.state == PENDING ){
			sp_data.state = BUSY;
			sp_data.deadline_counter = SP_DLINE / (H_PERIOD / NUM_FRAMES);
			pthread_mutex_lock( &mutex );	
			pthread_cond_signal( &(sp_data.thread_data.queue) );
			pthread_mutex_unlock( &mutex );
			printf("** SPORADIC TASK CREATED\n");
		}
		
		// Setto il risveglio dell'executive allo scadere dello slack, se il task sporadico è stato ritrovato running.	
		if( sp_data.state == BUSY ){
			printf("------------------------------(Slack time started)----------------------------\n");	
			if( sp_data.deadline_counter == 0 ){
				printf("**!!** SPORADIC TASK DEADLINE MISS **!!**\n");
				shutdown();
			}
			pthread_cond_signal( &(frames[frame_counter].thread_data.queue) );

			// Setto il thread del task sporadico a priorità maggiore rispetto alla deadline del frame
			frames[frame_counter].thread_data.param.sched_priority = low_priority;
			pthread_setschedparam( frames[frame_counter].thread_data.thread, SCHED_FIFO, &frames[frame_counter].thread_data.param );
			sp_data.thread_data.param.sched_priority = high_priority;	
			pthread_setschedparam( sp_data.thread_data.thread, SCHED_FIFO, &sp_data.thread_data.param );
			
			// Calcolo del tempo relativo alla scadenza dello slack time.
			slack_wakeup_time = SLACK[frame_counter] * TIME_UNIT_IN_MILLI * MILLI_TO_NANO;		
			pthread_mutex_lock( &mutex );
			slack_time.tv_sec =  time.tv_sec + slack_wakeup_time / 1000000000;
			slack_time.tv_nsec = (time.tv_nsec + slack_wakeup_time ) % 1000000000;
			pthread_cond_timedwait( &executive_data.queue, &mutex, &slack_time );
			pthread_mutex_unlock( &mutex );

			// Un frame è passato.
			sp_data.deadline_counter -= 1;
			print_current_time("EXECUTION DURING SLACK TERMINATED", zero);
			printf("------------------------------(Slack time ended)------------------------------\n");
		
			// Scaduto il tempo in cui il task sporadico può eseguire, ripristino a priorità
			// superiore i thread dei frame.
			frames[frame_counter].thread_data.param.sched_priority = high_priority;
			pthread_setschedparam( frames[frame_counter].thread_data.thread, SCHED_FIFO, &frames[frame_counter].thread_data.param );
			sp_data.thread_data.param.sched_priority = low_priority;	
			pthread_setschedparam( sp_data.thread_data.thread, SCHED_FIFO, &sp_data.thread_data.param );
		}	
		else{
			// Se non devo eseguire il task sporadico, semplicemente, segnalo al frame di partire all'inizio.	
			pthread_cond_signal( &(frames[frame_counter].thread_data.queue) );	
		}
		// Calcolo del tempo di risveglio dell'executive dopo la scadenza del frame.
		pthread_mutex_lock( &mutex );				
		time.tv_sec += ( time.tv_nsec + wakeup_time ) / 1000000000;
		time.tv_nsec = ( time.tv_nsec + wakeup_time ) % 1000000000;
		pthread_cond_timedwait( &executive_data.queue, &mutex, &time );
		pthread_mutex_unlock( &mutex );

		// --- DOPO IL RISVEGLIO
	
		// Check deadline		
		deadlinemiss_handler(frame_counter);
				
		++frame_counter; 
		frame_counter = frame_counter % NUM_FRAMES; 
	}

	return NULL;
}

void deadlinemiss_handler(int frame_id){
	frame_states_t state = frames[frame_id].state;
	if( state == BUSY ){
		printf("**!!** DEADLINE MISS **!!**\n");
		int it = frames[frame_id].iteration;
		printf("**!!** Uncompleted functions are: \n");
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
		shutdown(1);
	}
}
