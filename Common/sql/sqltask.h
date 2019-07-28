#ifndef _SQLTASK_H
#define _SQLTASK_H

typedef void * (*TaskCallback)(void * data, void * user);

C_DECLARATIONS_BEGIN

void sql_task_init(unsigned workers, unsigned count);
void sql_task_set_callback(unsigned worker, TaskCallback callback, void * user);
void sql_task_shutdown(void);

void sql_task_push(unsigned worker, void * data);
void * sql_task_pop(void);
void * sql_task_trypop(void);

void sql_task_freeze(void);
void sql_task_thaw(void);

void sql_task_worker_barrier(void);

C_DECLARATIONS_END

#endif
