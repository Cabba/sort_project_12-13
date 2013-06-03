#include "task.h"

////////////////////
/// Global variables
////////////////////
// Time origin
struct timeval zero;

// Time declaration
int nanosec = 10E8;

// Threads
pthread_t exec_thread;

// Global mutex
pthread_mutex_t mutex;

// Executive waiting queue
pthread_cond_t exec_wait_queue;

// Threads condition variables
pthread_cond_t *threads_cond;

// Threads 
pthread_t *threads;
int *threads_index;

// Frame states
typedef enum{ BUSY, IDLE, PENDING } frame_states_t;

typedef struct {
	frame_states_t *threads_state;
        int counter_function;
}frame_values;

// Frame states array
frame_values *threads_value;

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
