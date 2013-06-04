#include <stdbool.h>
#include "task.h"

////////////////////
/// Global variables
////////////////////
// Time origin
struct timeval zero;

#ifndef TIME_UNIT_IN_MILLI
	#define TIME_UNIT_IN_MILLI 10
#endif

#ifndef MILLI_TO_NANO
	#define MILLI_TO_NANO 1E6
#endif

// Global mutex
pthread_mutex_t mutex;

// Frame states
typedef enum{ BUSY, IDLE, PENDING } frame_states_t;

// Generical thread data
typedef struct thread_data_t_{
	pthread_t thread;
	pthread_cond_t queue;
	pthread_attr_t attr;
	struct sched_param param; 
} thread_data_t; 	

// Frame data
typedef struct frame_data_t_ {
	frame_states_t state;
	int iteration;
	int id;
	pthread_cond_t queue; 
	thread_data_t thread_data;
} frame_data_t;

// Sporadic task data
typedef struct sp_data_t_{
	thread_data_t thread_data;
	frame_states_t state;
	int deadline_counter;
} sp_data_t;

// Frames datai, ogni frame deve accedere al proprio e EXECUTIVE
frame_data_t *frames;

// Executive data, solo EXECUTIVE
thread_data_t executive_data;

// Sporadic task data, devono accedervi EXECUTIVE, SPORADIC
sp_data_t sp_data;

#ifndef SP_REQUEST_PROBABILITY
	#define SP_REQUEST_PROBABILITY 0.1
#endif // SP_REQUEST_PROBABILITY
////////////////////////
/// Functions prototypes
////////////////////////
void start();
void stop();

void* executive( void* param );
void* frame_handler( void* param );
void* sp_task_handler( void* param );
bool sp_task_request( );

void shutdown();
void executive_init();
void threads_init();
void print_current_time( char* string, struct timeval origin );
void deadlinemiss_handler(int frame_id );
