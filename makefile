all : resto hubert user

user : user.c user.h shmem.h semaphore.h hubert_types.h
	gcc user.c -o user -g

resto : resto.c resto.h shmem.h semaphore.h hubert_types.h
	gcc resto.c -o resto -g

hubert : hubert.c hubert.h semaphore.h hubert_types.h
	gcc hubert.c -o hubert -g

clean :
	-rm user hubert resto
