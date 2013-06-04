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

// Time declaration
unsigned int wakeup_time;

// Global mutex
pthread_mutex_t mutex;

// Frame states
typedef enum{ BUSY, IDLE, PENDING } frame_states_t;

// Executive data
typedef struct executive_data_t_{
	pthread_t thread;
	pthread_cond_t queue;
	pthread_attr_t thread_attr;
	struct sched_param thread_param; 
} executive_data_t; 	

// Frame data
typedef struct frame_data_t_ {
	frame_states_t state;
	int iteration;
	int id;
	pthread_cond_t queue; 
	// Threads variable
	pthread_t thread;
	pthread_attr_t thread_attr;
	struct sched_param thread_param;
} frame_data_t;

frame_data_t *frames;
executive_data_t executive_data;

// Sporadic job request
typedef enum { CREATE, NONE } sporadic_request_t; 
sporadic_request_t sp_request;

#ifndef SP_REQUEST_PROBABILITY
	#define SP_REQUEST_PROBAILITY 0.6
#endif // SP_REQUEST_PROBABILITY
////////////////////////
/// Functions prototypes
////////////////////////
void start();
void stop();

void* executive( void* param );
void* frame_handler( void* param );
bool sp_task_request( );

void executive_init();
void threads_init();
void print_current_time( char* string, struct timeval origin );
void print_thread_state(int thread_number);
void deadlinemiss_handler(int frame_id );
