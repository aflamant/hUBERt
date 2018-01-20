all : resto hubert user

user : user.c shmem.h semaphore.h msg.h
	gcc user.c -o user -g

resto : resto.c shmem.h semaphore.h msg.h
	gcc resto.c -o resto -g

hubert : hubert.c shmem.h semaphore.h msg.h
	gcc hubert.c -o hubert -g

clean :
	-rm user hubert resto
