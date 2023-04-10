/** @file libscheduler.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "libscheduler.h"
#include "../libpriqueue/libpriqueue.h"


/**
  Stores information making up a job to be scheduled including any statistics.

  You may need to define some global variables or a struct to store your job queue elements. 
*/
typedef struct _job_t
{
    int id;
    int arrival;
    int period;
    int wait;
    int precedence;
    int coreID;
    int timeRequired;
    int response;
    int turnover;
    int RR;
    int last;
    struct _job_t *nextJob;


} job_t;

//Globals
int mJobCount, mJobTotal, mCores;
int *mScheduler;
job_t *mJobs;
priqueue_t *mJobQueue;
scheme_t mScheme;

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

int compTime( const void * i, const void * j )
{

  return ( *(job_t*)i).timeRequired - ((*(job_t*)j).timeRequired);

}

int compPrec( const void * i, const void * j )
{
    int c = ( *( job_t* )i ).precedence - ( *( job_t* )j ).precedence;
    if( c == 0 )
    {
        return ( *( job_t* )i ).arrival - ( *( job_t* )j ).arrival;
    } 
    else{
        return c;
    } 

}

int compOne(  const void * i, const void * j )
{
    return 1;
}

void scheduler_start_up(int cores, scheme_t scheme)
{
  mScheduler = malloc( cores * sizeof(int));
  mJobQueue = malloc(sizeof( priqueue_t ));
  mScheme = scheme;
  mCores = cores;

  for( int i = 0; i < cores; i++ )
  {
    mScheduler[i] = 0;
  }

  switch( mScheme )
  {
    case FCFS : 
      priqueue_init( mJobQueue, compOne );
      break;
    
    case RR : 
      priqueue_init( mJobQueue, compOne );
      break;
    
    case SJF :
      priqueue_init( mJobQueue, compTime );
      break;

    case PSJF :
        priqueue_init( mJobQueue, compTime );
        break;
    
    case PRI :
        priqueue_init( mJobQueue, compPrec );
        break;

    case PPRI : 
        priqueue_init( mJobQueue, compPrec );
        break;
  }
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
	mJobCount++;
    mJobTotal++;
    job_t *job1;
    job_t *job2;
    bool done = 0;

    if(job_number == 0)
    {
        mJobs = malloc(sizeof(job_t ));
        job1 = mJobs; 
    }
    else if( job_number == 1 )
    {
        job1 = malloc(sizeof( job_t ));
        mJobs->nextJob = job1;
    }
    else
    {
        job1 = mJobs;
        for( int i = 0; i < (mJobTotal - 2); i++)
        {
            job1 = job1->nextJob;
        }
        job2 = malloc(sizeof( job_t ));
        job1->nextJob = job2;
        job1 = job2;
    }

    job1->id = job_number;
    job1->period = running_time;
    job1->arrival = time;
    job1->precedence = priority;
    job1->timeRequired = running_time;
    job1->last = -1;
    job1->response = -1;
    job1->coreID = -1;
    int newCoreID = -1;
    int pos = -1;

    for( int i = 0; i < priqueue_size(mJobQueue); i++ )
    {
        job_t *ptr = (job_t*)priqueue_at(mJobQueue, i);
        if( ptr->coreID != -1 )
        {
            ptr->timeRequired = ptr->period - (time - ptr->wait - ptr->arrival );
        }
    }

    for( int i = 0; i < mCores; i++ )
    {
        if( mScheduler[i] == 0 )
        {
            job1->response = 0;
            job1->wait = 0;

            newCoreID = i;
            job1->coreID = i;

            if( mScheme == RR )
            {
                job1->RR = priqueue_size( mJobQueue );
            }
            mScheduler[i] = 1;
            pos = priqueue_offer( mJobQueue, job1 );
            done = 1;
            break;
        }
    }

    if( !done && (mScheme == PPRI || mScheme == PSJF ))
    {
        pos = priqueue_offer( mJobQueue, job1 );
        if( pos < mCores )
        {
            for( int i = (priqueue_size(mJobQueue) - 1); i >= 0; i-- )
            {
                if(((job_t*)priqueue_at(mJobQueue, i))->coreID != -1 )
                {
                    job_t *ptr = (job_t*)priqueue_at( mJobQueue, i );
                    newCoreID = ptr->coreID;
                    mScheduler[newCoreID] = 1;
                    ptr->last = time;
                    ptr->coreID = -1;
                    ptr->timeRequired = ptr->period - ( time - ptr->wait - ptr->arrival );
                    if( ptr->timeRequired == ptr->period )
                    {
                        ptr->response = -1;
                        ptr->last = -1;
                        ptr->wait = -1;
                    }
                    break;
                }
            }
            job1->coreID = newCoreID;
            job1->wait = 0;
            job1->response = 0;
            done = 1;
        }
    }
    if( !done && mScheme != PSJF && mScheme != PPRI )
    {
        if( mScheme == RR )
        {
            job1->RR = priqueue_size( mJobQueue );
        }
        pos = priqueue_offer( mJobQueue, job1 );
    }
    return newCoreID;
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
	job_t *ptr;
    mJobCount--;

    mScheduler[core_id] = 0;
    int idle;

    if( mScheme != RR )
    {
        for( int i = 0; i < priqueue_size(mJobQueue); i++ )
        {
            ptr = (job_t*)priqueue_at( mJobQueue, i );
            if( ptr->id == job_number )
            {
                ptr = (job_t*)priqueue_remove_at(mJobQueue, i);
                idle = core_id;
                ptr->timeRequired = ptr->period - (time - ptr->wait - ptr->arrival );
                ptr->turnover = time - ptr->arrival;
                ptr->coreID = -1;
            }
        } 
        ptr = (job_t*)priqueue_peek(mJobQueue);
        if( ptr == NULL )
        {
            return -1;
        }
        else
        {
            int i = 0;
            while( ptr->coreID != -1 && i < priqueue_size(mJobQueue) - 1)
            {
                ptr = (job_t*)priqueue_at( mJobQueue, i );
                i++;
            }

            if( ptr->coreID != -1 )
            {
                return -1;
            }

            if( ptr->last != -1 )
            {
                ptr->wait = ptr->wait + (time - ptr->last);
                ptr->last = -1;
            }
            
            if( ptr->response == -1 )
            {
                ptr->response = time - ptr->arrival;
                ptr->wait = time - ptr->arrival;
            }

            ptr->coreID = idle;
            mScheduler[core_id] = 1;
            return ptr->id;
        }
    }
    else
    {
        for( int i = 0; i < priqueue_size( mJobQueue); i++ )
        {
            ptr = (job_t*)priqueue_at( mJobQueue, i );;
            if( ptr->id == job_number )
            {
                ptr = (job_t*)priqueue_remove_at(mJobQueue, i);
                ptr->turnover = time - ptr->arrival;
                ptr->timeRequired = ptr->period - ( time - ptr->wait - ptr->arrival );
                ptr->RR = -1;
                ptr->coreID = -1;
                idle = core_id;
            }
        }
        for( int i = 0; i < priqueue_size( mJobQueue ); i++ )
        {
            ptr = (job_t*)priqueue_at(mJobQueue, i);
            ptr->RR = i;
        }
        ptr = (job_t*)priqueue_peek(mJobQueue);

        if(ptr == NULL )
        {
            return -1;
        }
        else
        {
            int count = 0;
            while( count < (priqueue_size(mJobQueue) - 1) && ptr->coreID != -1 )
            {
                count++;
                ptr = (job_t*)priqueue_at( mJobQueue, count );
            }

            if( ptr->last != -1)
            {
                ptr->wait = ptr->wait + ( time - ptr->last );
                ptr->last = -1;
            }

            if( ptr->coreID != 1 )
            {
                return -1;
            }

            if( ptr->response == -1 )
            {
                ptr->wait = time - ptr->arrival;
                ptr->response = time-ptr->arrival;
            }
            ptr->coreID = idle;
            mScheduler[core_id] = 1;
            return ptr->id;
        }
    }
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
	job_t *job1;
    job_t *job2;

    int count = 0;

    for( int i = 0; i < priqueue_size(mJobQueue); i++ )
    {
        job1 = (job_t*)priqueue_at(mJobQueue, i );
        if( job1->coreID == core_id )
        {
            job1->RR = priqueue_size( mJobQueue );
            job1 = (job_t*)priqueue_remove_at( mJobQueue, i );
            mScheduler[core_id] = -1;
            job1->coreID = -1;
            job1->last = time;
            job1->timeRequired = job1->period - ( time - job1->wait - job1->arrival );
            break; 
        }
    }

    priqueue_offer( mJobQueue, job1 );
    job2 = (job_t*)priqueue_peek(mJobQueue);

    if( job2 == NULL )
    {
        return -1;
    }
    else
    {
        count = 0;
        while( count < (priqueue_size(mJobQueue) - 1) && job2->coreID != -1 )
        {
            count++;
            job2 = (job_t*)priqueue_at( mJobQueue, count );
        }

        if( job2->last != -1 )
        {
            job2->last = -1;
            job2->wait = job2->wait + ( time - job2->last );
        }

        if( job2->coreID != -1 )
        {
            return -1;
        }

        if( job2->response == -1 )
        {
            job2->wait = time - job2->arrival;
            job2->arrival = time - job2->arrival;
        }

        mScheduler[core_id] = 1;
        job2->coreID = core_id;
        return job2->id;
    }
}


/**
  Returns the average waiting time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete (all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average waiting time of all jobs scheduled.
 */
float scheduler_average_waiting_time()
{
	job_t *job;

    int totalTime = 0;
    job = mJobs;
    for( int i = 0; i < mJobTotal; i++ )
    {
        totalTime = totalTime + job->wait;
        job = job->nextJob;
    }
    return (float)totalTime / (float)mJobTotal;
}


/**
  Returns the average turnaround time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete (all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average turnaround time of all jobs scheduled.
 */
float scheduler_average_turnaround_time()
{
		job_t *job;

    int totalTime = 0;
    job = mJobs;
    for( int i = 0; i < mJobTotal; i++ )
    {
        totalTime = totalTime + job->turnover;
        job = job->nextJob;
    }
    return (float)totalTime / (float)mJobTotal;
}


/**
  Returns the average response time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete (all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average response time of all jobs scheduled.
 */
float scheduler_average_response_time()
{
	job_t *job;

    int totalTime = 0;
    job = mJobs;
    for( int i = 0; i < mJobTotal; i++ )
    {
        totalTime = totalTime + job->response;
        job = job->nextJob;
    }
    return (float)totalTime / (float)mJobTotal;
}


/**
  Free any memory associated with your scheduler.
 
  Assumption:
    - This function will be the last function called in your library.
*/
void scheduler_clean_up()
{

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

}
