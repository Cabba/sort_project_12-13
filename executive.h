#include <stdbool.h>
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
	int 			id;
	int 			iteration;
	frame_states_t 	state; 
	thread_data_t 	thread_data;
} frame_data_t;

// Sporadic task data
typedef struct sp_data_t_{
	thread_data_t thread_data;
	frame_states_t state;
	int deadline_counter;
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

