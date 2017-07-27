#include <stdint.h>
#include <kernel.h>

void* alloc(uint32_t size) {
    // search of a big enough, free, heap chunk
    heap_chunk_t* heap_chunk = (heap_chunk_t*)task_table[current_task]->heap_begin;
    heap_chunk_t* new_chunk;
    uint32_t heap_chunk_ptr;

again:
    if ((heap_chunk->free) && (heap_chunk->size > (size + sizeof(heap_chunk_t)))) {
        // split off a new heap_chunk
        new_chunk = (heap_chunk_t*)((uint32_t)heap_chunk + size + sizeof(heap_chunk_t));
        new_chunk->free = 1;
        new_chunk->size = heap_chunk->size - (size + sizeof(heap_chunk_t));
        new_chunk->prev_chunk = (uint32_t)heap_chunk;
        new_chunk->prev_chunk -= task_table[current_task]->base;
        heap_chunk->free = !heap_chunk->free;
        heap_chunk->size = size;
        return (void*)(((uint32_t)heap_chunk + sizeof(heap_chunk_t)) - task_table[current_task]->base);
    } else {
        heap_chunk_ptr = (uint32_t)heap_chunk;
        heap_chunk_ptr += heap_chunk->size + sizeof(heap_chunk_t);
        if (heap_chunk_ptr >= (task_table[current_task]->base + (task_table[current_task]->pages * 4096)))
            return (void*)0;        // alloc fail
        heap_chunk = (heap_chunk_t*)heap_chunk_ptr;
        goto again;
    }
}

void free(void* addr) {
    uint32_t heap_chunk_ptr = (uint32_t)addr;
    heap_chunk_ptr += task_table[current_task]->base;
    
    heap_chunk_ptr -= sizeof(heap_chunk_t);
    heap_chunk_t* heap_chunk = (heap_chunk_t*)heap_chunk_ptr;
    
    heap_chunk_ptr += heap_chunk->size + sizeof(heap_chunk_t);
    heap_chunk_t* next_chunk = (heap_chunk_t*)heap_chunk_ptr;
    
    heap_chunk_t* prev_chunk;
    if (heap_chunk->prev_chunk)
        prev_chunk = (heap_chunk_t*)(heap_chunk->prev_chunk + task_table[current_task]->base);
    else
        prev_chunk = (heap_chunk_t*)0;
    
    // flag chunk as free
    heap_chunk->free = 1;
    
    // if the next chunk is free as well, fuse the chunks into a single one
    if (next_chunk->free)
        heap_chunk->size += next_chunk->size + sizeof(heap_chunk_t);
    
    // if the previous chunk is free as well, fuse the chunks into a single one
    if (prev_chunk) {       // if its not the first chunk
        if (prev_chunk->free)
            prev_chunk->size += heap_chunk->size + sizeof(heap_chunk_t);
    }
    
    return;
}

void* realloc(void* prev_ptr, uint32_t size) {
    return (void*)0;
}

// prints a char to the current standard output
// for now, it will just print raw to the text driver
void char_to_stdout(int c) {
    text_putchar((char)c, task_table[current_task]->tty);
    return;
}

void enter_iowait_status(void) {
    task_table[current_task]->status = KRN_STAT_IOWAIT_TASK;
    return;
}

void pwd(char* pwd_dump) {
    uint32_t pwd_dump_ptr = (uint32_t)pwd_dump;
    pwd_dump_ptr += task_table[current_task]->base;
    pwd_dump = (char*)pwd_dump_ptr;

    kstrcpy(pwd_dump, task_table[current_task]->pwd);
    return;
}