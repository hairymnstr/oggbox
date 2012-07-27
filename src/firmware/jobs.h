#ifndef JOBS_H
#define JOBS_H 1

#define JOB_QUEUE_LEN  32
#define JOB_SYSTEM_COUNT 8

#define JOB_SYSTEM_UNUSED 0
#define JOB_SYSTEM_DONE 1
#define JOB_SYSTEM_PENDING 2

typedef struct {
  void *task;
  int status;
} SystemJob;

#endif /* ifndef JOBS_H */