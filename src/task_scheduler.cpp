#include "task_scheduler.h"

Task::Task(GlobalDescriptorTable *gdt, void entry_point())
{
    cpu_state = (CPUState*) (stack + 4096 - sizeof(CPUState));
    
    cpu_state -> eax = 0;
    cpu_state -> ebx = 0;
    cpu_state -> ecx = 0;
    cpu_state -> edx = 0;

    cpu_state -> esi = 0;
    cpu_state -> edi = 0;
    cpu_state -> ebp = 0;
    
    cpu_state -> eip = (uint32_t) entry_point;
    cpu_state -> cs = gdt->get_code_segment_selector();

    cpu_state -> eflags = 0x202;
    
}

Task::~Task()
{

}

        
TaskScheduler::TaskScheduler()
{
    num_tasks = 0;
    current_task = -1;
}

TaskScheduler::~TaskScheduler()
{
}

bool TaskScheduler::add_task(Task* task)
{
    if (num_tasks >= 256) {
        return false;
    }
    tasks[num_tasks++] = task;
    return true;
}

CPUState* TaskScheduler::schedule(CPUState* cpu_state)
{
    if (num_tasks <= 0) {
        return cpu_state;
    }
    
    if (current_task >= 0) {
         tasks[current_task]->cpu_state = cpu_state;
    }
    
    if (++current_task >= num_tasks) {
        current_task %= num_tasks;
    }
    return tasks[current_task]->cpu_state;
}
