#include <stdint.h>
#include "jobs.h"

void *job_queue[JOB_QUEUE_LEN];
int job_next;
int job_count;
SystemJob job_system_tasks[JOB_SYSTEM_COUNT];

void job_init() {
  job_next = 0;
  job_count = 0;
  
  for(i=0;i<JOB_SYSTEM_COUNT;i++) {
    job_system_tasks[i].status = JOB_SYSTEM_UNUSED;
  }
}

void job_add(void *job) {
  if(job_count == JOB_QUEUE_LEN) {
    job_error_halt();
  } else {
    job_queue[job_next + job_count++];
  }    
}

void *job_get_next() {
  if(job_count == 0) {
    return NULL;
  } else {
    job_count--;
    job_next = (job_next + 1) % JOB_QUEUE_LEN;
    return job_queue[job_next++];
  }
}

void job_system_refresh() {
  int i;
  for(i=0;i<JOB_SYSTEM_COUNT;i++) {
    if(job_system_tasks[i].status == JOB_SYSTEM_DONE) {
      job_add(job_system_tasks[i].task);
    }
  }
}

int job_register_system_task(void *task) {
  while(