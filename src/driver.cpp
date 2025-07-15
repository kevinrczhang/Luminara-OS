#include "driver.h"

Driver::Driver(InterruptManager* manager, uint8_t interrupt_number)
    : InterruptHandler(manager, interrupt_number)
{

}

Driver::~Driver()
{

}

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
