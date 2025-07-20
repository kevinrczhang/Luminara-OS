#ifndef TASK_SCHEDULER_H
#define TASK_SCHEDULER_H

#include "gdt.h"
#include "types.h"

struct CPUState
{
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;

    uint32_t esi;
    uint32_t edi;
    uint32_t ebp;

    uint32_t error;

    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
    uint32_t esp;
    uint32_t ss;        
} __attribute__((packed));

class Task
{
    friend class TaskScheduler;
    private:
        uint8_t stack[4096];
        CPUState* cpu_state;
    public:
        Task(GlobalDescriptorTable *gdt, void entry_point());
        ~Task();
};


class TaskScheduler
{
    private:
        Task* tasks[256];
        int num_tasks;
        int current_task;

    public:
        TaskScheduler();
        ~TaskScheduler();
        bool add_task(Task* task);
        CPUState* schedule(CPUState* cpu_state);
};

#endif
