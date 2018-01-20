/*
 * Header: shmem.h
 * 
 * Provides wrapper functions around POSIX system calls shmget, 
 * shmctl, shmat and shmdt allowing the creation, opening
 * and disposal of shared memory segments and connecting to and
 * disconnecting from them.
 * 
 * 
 * Copyright 2014 Razmig Kéchichian, INSA de Lyon, TC department
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 * 
 */ 


#ifndef __SHMEM_H_SHMEM__
#define __SHMEM_H_SHMEM__


#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>


int create_shmem(int key, int memsize)
{
	return shmget(key, memsize, 0660 | IPC_CREAT) ;
}

int open_shmem(int key, int memsize)
{
	return shmget(key, memsize, 0660) ;
}

int remove_shmem(int id) 
{
	return shmctl(id, IPC_RMID, 0) ;
}

void* attach_shmem(int id)
{
	return shmat(id, 0, 0) ;
}

int detach_shmem(const void* buffer)
{
	return shmdt(buffer)  ;
}


#endif