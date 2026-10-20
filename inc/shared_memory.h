#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#include <sys/ipc.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <stdio.h>

void free_shared_memory (void *ptr_sh);
void remove_shared_memory (int shmid);
int get_shared_memory (key_t shmKey, size_t size);
void *attach_shared_memory (int shmid, int shmflg);

#endif