#include "memory_manager.h"

MemoryManager* MemoryManager::memory_manager { nullptr };

MemoryManager::MemoryManager(size_t start, size_t size)
{
    memory_manager = this;
    
    if (size < sizeof(MemoryChunk)) {
        first = 0;
    } else {
        first = (MemoryChunk*) start;
        
        first -> allocated = false;
        first -> prev = 0;
        first -> next = 0;
        first -> size = size - sizeof(MemoryChunk);
    }
}

MemoryManager::~MemoryManager()
{
    if (memory_manager == this) {
        memory_manager = nullptr;
    }
}

void* MemoryManager::malloc(size_t requested_size)
{
    MemoryChunk* result_chunk { nullptr };
    
    for (MemoryChunk* chunk = first; chunk != nullptr && result_chunk == nullptr; chunk = chunk->next) {
        if (chunk->size > requested_size && !chunk->allocated) {
            result_chunk = chunk;
        }
    }
        
    if (result_chunk == nullptr) {
        return nullptr;
    }
    
    if (result_chunk->size >= requested_size + sizeof(MemoryChunk) + 1) {
        MemoryChunk* temp = (MemoryChunk*) ((size_t) result_chunk + sizeof(MemoryChunk) + requested_size);
        
        temp->allocated = false;
        temp->size = result_chunk->size - requested_size - sizeof(MemoryChunk);
        temp->prev = result_chunk;
        temp->next = result_chunk->next;

        if (temp->next != nullptr) {
            temp->next->prev = temp;
        }
        
        result_chunk->size = requested_size;
        result_chunk->next = temp;
    }
    
    result_chunk->allocated = true;

    return (void*)(((size_t) result_chunk) + sizeof(MemoryChunk));
}

void MemoryManager::free(void* ptr)
{
    MemoryChunk* chunk = (MemoryChunk*) ((size_t) ptr - sizeof(MemoryChunk));
    
    chunk -> allocated = false;
    
    if (chunk->prev != nullptr && !chunk->prev->allocated) {
        chunk->prev->next = chunk->next;
        chunk->prev->size += chunk->size + sizeof(MemoryChunk);
        
        if (chunk->next != nullptr) {
            chunk->next->prev = chunk->prev;
        }
        
        chunk = chunk->prev;
    }
    
    if (chunk->next != nullptr && !chunk->next->allocated) {
        chunk->size += chunk->next->size + sizeof(MemoryChunk);
        chunk->next = chunk->next->next;

        if (chunk->next != nullptr) {
            chunk->next->prev = chunk;
        }
    }
}

void* operator new(size_t size)
{
    if (MemoryManager::memory_manager == nullptr) {
        return 0;
    }
    return MemoryManager::memory_manager->malloc(size);
}

void* operator new[](size_t size)
{
    if (MemoryManager::memory_manager == nullptr) {
        return 0;
    }
    return MemoryManager::memory_manager->malloc(size);
}

void* operator new(size_t size, void* ptr)
{
    return ptr;
}

void* operator new[](size_t size, void* ptr)
{
    return ptr;
}

void operator delete(void* ptr)
{
    if (MemoryManager::memory_manager != nullptr) {
        MemoryManager::memory_manager->free(ptr);
    }
}

void operator delete[](void* ptr)
{
    if (MemoryManager::memory_manager != nullptr) {
        MemoryManager::memory_manager->free(ptr);
    }
}
