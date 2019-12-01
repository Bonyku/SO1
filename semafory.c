#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>

/* Polecenie: 
	Stwórz dwa procesy - jeden niech utworzy semafor, ustawi wartosc na 1
	i poczeka aż drugi proces zmieni ta wartosc na 0.
*/

void up_and_wait(int semset_id)
{
	struct sembuf up_operation = {0,1,0};
	struct sembuf wait_operation = {0,0,0};
	char error_message[15];

	if(semop(semset_id,&up_operation,1)<0) {
		sprintf(error_message, "semop %u", __LINE__-1);
		perror(error_message);
	}
	if(semop(semset_id,&wait_operation,1)<0) {
		sprintf(error_message, "semop %u", __LINE__-1);
		perror(error_message);
	}
}

void down(int semset_id)
{
	struct sembuf down_operation = {0,-1,0};
	if(semop(semset_id,&down_operation,1)<0)
		perror("semop");
}

int get_sem_val(int sid, int semnum)
{
        int val;
        val = semctl(sid,semnum,GETVAL,0);
        if(val!=-1)
                return val;
        else{
                perror("semctl");
                exit(0);
        }
}

void do_child_work(int key)
{
	int semset_id = semget(key, 1,0);
	if(semset_id<0)
		perror("child, semset");

        printf("Wartosc semafora(ID: %d) w potomku: %d\n", semset_id, get_sem_val(semset_id,0));
	puts("Czekam 3 sekundy, nastepnie zeruje wartosc semafora");
	sleep(3);
	down(semset_id);
}

void do_parent_work(int key)
{
        int semset_id = semget(key, 1, 0600 | IPC_CREAT|IPC_EXCL);
	if(semset_id<0)
		perror("parent, semset");
	up_and_wait(semset_id);
	printf("Wartosc semafora(ID: %d) w rodzicu: %d\n", semset_id, get_sem_val(semset_id,0));
	if(semctl(semset_id,0,IPC_RMID)<0)
		perror("semctl");
}

int main()
{
	int key = ftok("/tmp", 8);
	if(key<0)
		perror("ftok");
	int pid = fork();
	if(pid<0)
		perror("fork");
	if(pid==0)
		do_child_work(key);
	else
		do_parent_work(key);	
}
