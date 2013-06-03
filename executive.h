#include "task.h"

////////////////////
/// Global variables
////////////////////
// Time origin
struct timeval zero;

// Time declaration
int nanosec = 10E8;

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

////////////////////////
/// Functions prototypes
////////////////////////
void start();
void stop();

void* executive( void* param );
void* frame_handler( void* param );

void executive_init();
void threads_init();
void print_current_time( char* string, struct timeval origin );
void print_thread_state(int thread_number);
void deadlinemiss_handler(int frame_id );
