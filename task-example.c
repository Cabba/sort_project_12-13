#include "task.h"

#include <stdlib.h>
#include <sys/time.h>
#include <limits.h>
#include <stdio.h>

/* Lunghezza dell'iperperiodo */
#define H_PERIOD_ 20

/* Numero di frame */
#define NUM_FRAMES_ 5

/* Numero di task */
#define NUM_P_TASKS_ 2

void task0_code();
void task1_code();

void sp_task_code();

/**********************/

void busy_calib();

/**********************/

/* Questo inizializza i dati globali */
const unsigned int H_PERIOD = H_PERIOD_;
const unsigned int NUM_FRAMES = NUM_FRAMES_;
const unsigned int NUM_P_TASKS = NUM_P_TASKS_;

task_routine P_TASKS[NUM_P_TASKS_];
task_routine SP_TASK;
int * SCHEDULE[NUM_FRAMES_];
int SLACK[NUM_FRAMES_];

int SP_WCET;
int SP_DLINE;

void task_init()
  {
  /* Inizializzazione di P_TASKS[] */
  P_TASKS[0] = task0_code;
  P_TASKS[1] = task1_code;
  /* ... */
  
  /* Inizializzazione di SP_TASK */
  SP_TASK = sp_task_code;


  /* Inizializzazione di SCHEDULE e SLACK (se necessario) */

  /* frame 0 */
  SCHEDULE[0] = (int *) malloc( sizeof( int ) * 1 );
  SCHEDULE[0][0] = 0;
  SCHEDULE[0][1] = -1;

  SLACK[0] = 4; /* tutto il frame */


  /* frame 1 */
  SCHEDULE[1] = (int *) malloc( sizeof( int ) * 1 );
  SCHEDULE[1][0] = -1;

  SLACK[1] = 4; /* tutto il frame */


  /* frame 2 */
  SCHEDULE[2] = (int *) malloc( sizeof( int ) * 1 );
  SCHEDULE[2][0] = -1;

  SLACK[2] = 4; /* tutto il frame */


  /* frame 3 */
  SCHEDULE[3] = (int *) malloc( sizeof( int ) * 1 );
  SCHEDULE[3][0] = -1;

  SLACK[3] = 4; /* tutto il frame */


  /* frame 4 */
  SCHEDULE[4] = (int *) malloc( sizeof( int ) * 1 );
  SCHEDULE[4][0] = -1;
  
  SLACK[4] = 4; /* tutto il frame */
  
  /* inizializzazione dei dati relativi al task sporadico */
  SP_WCET = 1;
  SP_DLINE = 10;

  /* Custom Code */
  busy_calib();
  }

void task_destroy()
  {
  unsigned int i;

  /* Custom Code */

  for ( i = 0; i < NUM_FRAMES; ++i )
    free( SCHEDULE[i] );
  }

/**********************************************************/

unsigned int calib = UINT_MAX/32;

void busy_wait(unsigned int millisec)
{
  unsigned int i, j;
  volatile unsigned int value;

  for (i=0; i < millisec; ++i)
    for (j=0; j < calib; ++j)
      value *= i+j; // dummy code
}

void busy_calib()
  {
  struct timeval before;
  struct timeval after;
  unsigned int duration;

  gettimeofday(&before, NULL);
  busy_wait(1);
  gettimeofday(&after, NULL);

  duration = (after.tv_sec - before.tv_sec) * 1000000;
  duration += after.tv_usec;
  duration -= before.tv_usec;

  calib /= duration;
  calib *= 1000;
  }
  
/**********************************************************/

/* Nota: nel codice dei task e' lecito chiamare sp_task_request() */

void task0_code(){
	printf("Funzione 0\n");	
}

  void task1_code()
   {
   /* Custom Code */
   }


  void sp_task_code()
   {
   /* Custom Code */
   }
