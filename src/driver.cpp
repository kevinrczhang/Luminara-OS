#include "driver.h"

Driver::Driver(InterruptManager* manager, uint8_t interrupt_number)
    : InterruptHandler(manager, interrupt_number)
{
    // Base driver constructor - calls InterruptHandler constructor
}

Driver::~Driver()
{

}

// Required for kernel development - provide empty implementations
// since we don't have standard library delete operators
void operator delete(void* ptr)
{

}

void operator delete(void* ptr, unsigned int size)
{

}

void operator delete[](void* ptr)
{

}

void operator delete[](void* ptr, unsigned int size)
{

}
