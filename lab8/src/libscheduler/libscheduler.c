/** @file libscheduler.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libscheduler.h"
#include "../libpriqueue/libpriqueue.h"


/**
  Stores information making up a job to be scheduled including any statistics.

  You may need to define some global variables or a struct to store your job queue elements. 
*/
typedef struct _job_t
{
  int pid;
  int arrival_time;
  int priority;
  int original_process_time;
  int process_time;
  int response_time;
  int last_checked_time;
} job_t;

int m_cores;
job_t **core_array;
scheme_t m_type;
int amt_jobs;
double wait_time;
double tat;
double response_time;

priqueue_t q;

int compareFCFS( const void * a, const void * b )
{
  return 1;
}

int compareSJF( const void * a, const void * b )
{
  return ( *( job_t* )a ).process_time - ( *( job_t* )b ).process_time;
}

int comparePRI( const void * a, const void * b  )
{
  int c = ( *( job_t* )a ).priority - ( *( job_t* )b ).priority;
  if( c == 0 ) return ( *( job_t* )a ).arrival_time - ( *( job_t* )b ).arrival_time;
  else return c;
}

/**
  Initalizes the scheduler.
 
  Assumptions:
    - You may assume this will be the first scheduler function called.
    - You may assume this function will be called once once.
    - You may assume that cores is a positive, non-zero number.
    - You may assume that scheme is a valid scheduling scheme.

  @param cores the number of cores that is available by the scheduler. These cores will be known as core(id=0), core(id=1), ..., core(id=cores-1).
  @param scheme  the scheduling scheme that should be used. This value will be one of the six enum values of scheme_t
*/
void scheduler_start_up(int cores, scheme_t scheme)
{
  m_cores = cores;
  core_array = malloc( cores * sizeof( job_t ) );

  wait_time = 0;
  response_time = 0;
  tat = 0;
  amt_jobs = 0;
  m_type = scheme;

  for( int i = 0; i < cores; ++i ){ core_array[ i ] = NULL; }

  if( m_type == FCFS || m_type == RR ) priqueue_init( &q, compareFCFS );
  else if( m_type == SJF || m_type == PSJF ) priqueue_init( &q, compareSJF );
  else if( m_type == PRI || m_type == PPRI) priqueue_init( &q, comparePRI );
}

int find_idle_core()
{
  for( int i = 0; i < m_cores; ++i )
  {
    if( core_array[ i ] == NULL ) return i;
  }

  return -1;
}

/**
  Called when a new job arrives.
 
  If multiple cores are idle, the job should be assigned to the core with the
  lowest id.
  If the job arriving should be scheduled to run during the next
  time cycle, return the zero-based index of the core the job should be
  scheduled on. If another job is already running on the core specified,
  this will preempt the currently running job.
  Assumption:
    - You may assume that every job wil have a unique arrival time.

  @param job_number a globally unique identification number of the job arriving.
  @param time the current time of the simulator.
  @param running_time the total number of time units this job will run before it will be finished.
  @param priority the priority of the job. (The lower the value, the higher the priority.)
  @return index of core job should be scheduled on
  @return -1 if no scheduling changes should be made. 
 
 */
int scheduler_new_job(int job_number, int time, int running_time, int priority)
{
	int first_idle_core = find_idle_core();
  job_t *temp = malloc( sizeof( job_t ) );

  temp->pid = job_number;
  temp->arrival_time = time;
  temp->priority = priority;
  temp->original_process_time = running_time;
  temp->process_time = running_time;
  temp->response_time = -1;

  if( first_idle_core != -1 )
  {
    core_array[ first_idle_core ] = temp;
    core_array[ first_idle_core ]->response_time = time - core_array[ first_idle_core ]->arrival_time;

    if( m_type == PSJF )
    {
      temp->last_checked_time = time;
    }

    return first_idle_core;
  }

  else if( m_type == PSJF ) 
  {
    int longest_run_time = -1;
    int lrt_index;

    for( int i = 0; i < m_cores; ++i )
    {
      core_array[ i ]->process_time = core_array[ i ]->process_time - ( time - core_array[ i ]->last_checked_time );
      core_array[ i ]->last_checked_time = time;

      if( core_array[ i ]->process_time > longest_run_time )
      {
        longest_run_time = core_array[ i ]->process_time;
        lrt_index = i;
      }
    }

    if( longest_run_time > running_time )
    {
      if( core_array[ lrt_index ]->response_time == time - core_array[ lrt_index ]->arrival_time ) core_array[ lrt_index ]->response_time = -1;

      priqueue_offer( &q, core_array[ lrt_index ] );
      core_array[ lrt_index ] = temp;

      if( core_array[ lrt_index ]->response_time == -1 ) core_array[ lrt_index ]->response_time = time - core_array[ lrt_index ]->arrival_time;
      return lrt_index;
    }
  }

  else if( m_type == PPRI )
  {
    int current_lowest_pri = core_array[ 0 ]->priority;
    int lowest_pri_core = 0;

    for( int i = 0; i < m_cores; i++ )
    {
      if( core_array[ i ]->priority > current_lowest_pri )
      {
        current_lowest_pri = core_array[i]->priority;
        lowest_pri_core = i;
      }

      else if( core_array[ i ]->priority == current_lowest_pri && core_array[ i ]->arrival_time > core_array[ lowest_pri_core ]->arrival_time ) lowest_pri_core = i;
    }

    if( current_lowest_pri > temp->priority )
    {
      if( core_array[ lowest_pri_core ]->response_time == time - core_array[ lowest_pri_core ]->arrival_time )
      {
        core_array[ lowest_pri_core ]->response_time = -1;
      }

      priqueue_offer( &q, core_array[ lowest_pri_core ] );
      core_array[ lowest_pri_core ] = temp;

      if( core_array[ lowest_pri_core ]->response_time == -1 )
      {
        core_array[ lowest_pri_core ]->response_time = time - core_array[ lowest_pri_core]->arrival_time;
      }

      return lowest_pri_core;
    }
  }

  priqueue_offer( &q, temp );
  return -1;
}


/**
  Called when a job has completed execution.
 
  The core_id, job_number and time parameters are provided for convenience. You may be able to calculate the values with your own data structure.
  If any job should be scheduled to run on the core free'd up by the
  finished job, return the job_number of the job that should be scheduled to
  run on core core_id.
 
  @param core_id the zero-based index of the core where the job was located.
  @param job_number a globally unique identification number of the job.
  @param time the current time of the simulator.
  @return job_number of the job that should be scheduled to run on core core_id
  @return -1 if core should remain idle.
 */
int scheduler_job_finished(int core_id, int job_number, int time)
{
  wait_time += time - core_array[ core_id ]->arrival_time - core_array[ core_id ]->original_process_time;
  tat += time - core_array[ core_id ]->arrival_time;
  response_time += core_array[ core_id ]->response_time;
  amt_jobs++;

  free( core_array[ core_id ] );
  core_array[ core_id ] = NULL;

  if( priqueue_size( &q ) != 0 )
  {
    job_t* temp = ( job_t* )priqueue_poll( &q );

    if( m_type == PSJF ) temp->last_checked_time = time;

    core_array[ core_id ] = temp;
    if( core_array[ core_id ]->response_time == -1 ) core_array[core_id]->response_time = time - core_array[core_id]->arrival_time;
    return temp->pid;
  }

  return -1;
}


/**
  When the scheme is set to RR, called when the quantum timer has expired
  on a core.
 
  If any job should be scheduled to run on the core free'd up by
  the quantum expiration, return the job_number of the job that should be
  scheduled to run on core core_id.

  @param core_id the zero-based index of the core where the quantum has expired.
  @param time the current time of the simulator. 
  @return job_number of the job that should be scheduled on core cord_id
  @return -1 if core should remain idle
 */
int scheduler_quantum_expired(int core_id, int time)
{
  job_t* job_on_core = core_array[ core_id ];

  if( job_on_core == NULL )
  {
    if( priqueue_size( &q ) == 0 ) return -1;
  }
  else priqueue_offer( &q, job_on_core );

  core_array[ core_id ] = priqueue_poll( &q );
  if( core_array[ core_id ]->response_time == -1 ) core_array[ core_id ]->response_time = time - core_array[core_id]->arrival_time;
  return core_array[ core_id ]->pid;
}


/**
  Returns the average waiting time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete (all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average waiting time of all jobs scheduled.
 */
float scheduler_average_waiting_time()
{
	return wait_time / amt_jobs;
}


/**
  Returns the average turnaround time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete (all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average turnaround time of all jobs scheduled.
 */
float scheduler_average_turnaround_time()
{
	return tat / amt_jobs;
}


/**
  Returns the average response time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete (all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average response time of all jobs scheduled.
 */
float scheduler_average_response_time()
{
	return response_time / amt_jobs;
}


/**
  Free any memory associated with your scheduler.
 
  Assumption:
    - This function will be the last function called in your library.
*/
void scheduler_clean_up()
{
  for( int i = 0; i < m_cores; ++i ){ if( core_array[ i ] != NULL ) free( core_array[ i ] ); }
  free( core_array );
}


/**
  This function may print out any debugging information you choose. This
  function will be called by the simulator after every call the simulator
  makes to your scheduler.
  In our provided output, we have implemented this function to list the jobs in the order they are to be scheduled. Furthermore, we have also listed the current state of the job (either running on a given core or idle). For example, if we have a non-preemptive algorithm and job(id=4) has began running, job(id=2) arrives with a higher priority, and job(id=1) arrives with a lower priority, the output in our sample output will be:

    2(-1) 4(0) 1(-1)  
  
  This function is not required and will not be graded. You may leave it
  blank if you do not find it useful.
 */
void scheduler_show_queue()
{
  printf( "wow\n" );
}
