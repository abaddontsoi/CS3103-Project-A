#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <sys/shm.h>   // This is necessary for using shared memory constructs
#include <semaphore.h> // This is necessary for using semaphore
#include <fcntl.h>	   // This is necessary for using semaphore
#include <pthread.h>   // This is necessary for Pthread
#include <string.h>

// To prevent multiple students to define semaphore with the same name,
// please always define the name of a semaphore ending with "_GROUP_NAME"
// where GROUP is your group number and NAME is your full name.
// For example, if you have 3 semaphores, you can define them as:
// semaphore_1_GROUP_NAME, semaphore_2_GROUP_NAME, semaphore_3_GROUP_NAME ...
#define PARAM_ACCESS_SEMAPHORE "/param_access_semaphore_GROUP_NAME"

#define THREAD_COUNT 9
#define digit_0_25_Tsoi_Yiu_Chik "/digit_0_access_semaphore_25_Tsoi_Yiu_Chik"
#define digit_1_25_Tsoi_Yiu_Chik "/digit_1_access_semaphore_25_Tsoi_Yiu_Chik"
#define digit_2_25_Tsoi_Yiu_Chik "/digit_2_access_semaphore_25_Tsoi_Yiu_Chik"
#define digit_3_25_Tsoi_Yiu_Chik "/digit_3_access_semaphore_25_Tsoi_Yiu_Chik"
#define digit_4_25_Tsoi_Yiu_Chik "/digit_4_access_semaphore_25_Tsoi_Yiu_Chik"
#define digit_5_25_Tsoi_Yiu_Chik "/digit_5_access_semaphore_25_Tsoi_Yiu_Chik"
#define digit_6_25_Tsoi_Yiu_Chik "/digit_6_access_semaphore_25_Tsoi_Yiu_Chik"
#define digit_7_25_Tsoi_Yiu_Chik "/digit_7_access_semaphore_25_Tsoi_Yiu_Chik"
#define digit_8_25_Tsoi_Yiu_Chik "/digit_8_access_semaphore_25_Tsoi_Yiu_Chik"

sem_t *digit_access_semaphore[THREAD_COUNT];

long int global_param = 0;

struct t_args
{
	int digit_posi;
	int loop_count;
};

/**
 * This function should be implemented by yourself. It must be invoked
 * in the child process after the input parameter has been obtained.
 * @parms: The input parameter from the terminal.
 */
void multi_threads_run(long int input_param, long int op_round);

int main(int argc, char **argv)
{
	int shmid, status;
	long int local_param = 0;
	long int *shared_param_p, *shared_param_c;

	if (argc < 2)
	{
		printf("Please enter a nine-digit decimal number as the input parameter.\nUsage: ./main <input_param>\n");
		exit(-1);
	}

	/*
		Creating semaphores. Mutex semaphore is used to acheive mutual
		exclusion while processes access (and read or modify) the global
		variable, local variable, and the shared memory.
	*/

	// Checks if the semaphore exists, if it exists we unlink him from the process.
	sem_unlink(PARAM_ACCESS_SEMAPHORE);

	// Create the semaphore. sem_init() also creates a semaphore. Learn the difference on your own.
	sem_t *param_access_semaphore = sem_open(PARAM_ACCESS_SEMAPHORE, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1);

	// Check for error while opening the semaphore
	if (param_access_semaphore != SEM_FAILED)
	{
		printf("Successfully created new semaphore!\n");
	}
	else if (errno == EEXIST)
	{ // Semaphore already exists
		printf("Semaphore appears to exist already!\n");
		param_access_semaphore = sem_open(PARAM_ACCESS_SEMAPHORE, 0);
	}
	else
	{ // An other error occured
		assert(param_access_semaphore != SEM_FAILED);
		exit(-1);
	}

	/*
		Creating shared memory.
		The operating system keeps track of the set of shared memory
		segments. In order to acquire shared memory, we must first
		request the shared memory from the OS using the shmget()
		system call. The second parameter specifies the number of
		bytes of memory requested. shmget() returns a shared memory
		identifier (SHMID) which is an integer. Refer to the online
		man pages for details on the other two parameters of shmget()
	*/
	shmid = shmget(IPC_PRIVATE, sizeof(long int), 0666 | IPC_CREAT); // We request an array of one long integer

	/*
		After forking, the parent and child must "attach" the shared
		memory to its local data segment. This is done by the shmat()
		system call. shmat() takes the SHMID of the shared memory
		segment as input parameter and returns the address at which
		the segment has been attached. Thus shmat() returns a char
		pointer.
	*/

	if (fork() == 0)
	{ // Child Process

		printf("Child Process: Child PID is %jd\n", (intmax_t)getpid());

		/*  shmat() returns a long int pointer which is typecast here
			to long int and the address is stored in the long int pointer shared_param_c. */
		shared_param_c = (long int *)shmat(shmid, 0, 0);

		while (1) // Loop to check if the variables have been updated.
		{
			// Get the semaphore
			sem_wait(param_access_semaphore);
			printf("Child Process: Got the variable access semaphore.\n");

			if ((global_param != 0) || (local_param != 0) || (shared_param_c[0] != 0))
			{
				printf("Child Process: Read the global variable with value of %ld.\n", global_param);
				printf("Child Process: Read the local variable with value of %ld.\n", local_param);
				printf("Child Process: Read the shared variable with value of %ld.\n", shared_param_c[0]);

				// Release the semaphore
				sem_post(param_access_semaphore);
				printf("Child Process: Released the variable access semaphore.\n");

				break;
			}

			// Release the semaphore
			sem_post(param_access_semaphore);
			printf("Child Process: Released the variable access semaphore.\n");
		}

		/**
		 * After you have fixed the issue in Problem 1-Q1,
		 * uncomment the following multi_threads_run function
		 * for Problem 1-Q2. Please note that you should also
		 * add an input parameter for invoking this function,
		 * which can be obtained from one of the three variables,
		 * i.e., global_param, local_param, shared_param_c[0].
		 */
		multi_threads_run(shared_param_c[0], strtol(argv[2], NULL, 10));

		/* each process should "detach" itself from the
		   shared memory after it is used */

		shmdt(shared_param_c);

		exit(0);
	}
	else
	{ // Parent Process

		printf("Parent Process: Parent PID is %jd\n", (intmax_t)getpid());

		/*  shmat() returns a long int pointer which is typecast here
			to long int and the address is stored in the long int pointer shared_param_p.
			Thus the memory location shared_param_p[0] of the parent
			is the same as the memory locations shared_param_c[0] of
			the child, since the memory is shared.
		*/
		shared_param_p = (long int *)shmat(shmid, 0, 0);

		// Get or lock the semaphore first
		sem_wait(param_access_semaphore);
		printf("Parent Process: Got the variable access semaphore.\n");

		global_param = strtol(argv[1], NULL, 10);
		local_param = strtol(argv[1], NULL, 10);
		shared_param_p[0] = strtol(argv[1], NULL, 10);

		// Release or unlock the semaphore
		sem_post(param_access_semaphore);
		printf("Parent Process: Released the variable access semaphore.\n");

		wait(&status);

		/* each process should "detach" itself from the
		   shared memory after it is used */

		shmdt(shared_param_p);

		/* Child has exited, so parent process should delete
		   the created shared memory. Unlike attach and detach,
		   which is to be done for each process separately,
		   deleting the shared memory has to be done by only
		   one process after making sure that noone else
		   will be using it
		 */

		shmctl(shmid, IPC_RMID, 0);

		// Close and delete semaphore.
		sem_close(param_access_semaphore);
		sem_unlink(PARAM_ACCESS_SEMAPHORE);

		exit(0);
	}

	exit(0);
}

void *the_thread(void *args)
{
	struct t_args *pt = (struct t_args *)args;
	int count = pt->loop_count;

	// for (int i = 0; i < count; i++)
	// {
	// 	sem_wait(digit_access_semaphore[pt->digit_posi]);
	// 	sem_wait(digit_access_semaphore[pt->digit_posi + 1]);

	// 	printf("%d %d\n", pt->digit_posi, pt->loop_count);

	// 	sem_post(digit_access_semaphore[pt->digit_posi + 1]);
	// 	sem_post(digit_access_semaphore[pt->digit_posi]);
	// }

	sem_wait(digit_access_semaphore[pt->digit_posi]);
	sem_wait(digit_access_semaphore[pt->digit_posi + 1]);

	int loop = pt->loop_count;
	for (int i=0; i<loop; i++) {
		printf("%d %d\n", pt->digit_posi, pt->loop_count);
	}

	sem_post(digit_access_semaphore[pt->digit_posi + 1]);
	sem_post(digit_access_semaphore[pt->digit_posi]);

	pthread_exit(NULL);
}

/**
 * This function should be implemented by yourself. It must be invoked
 * in the child process after the input parameter has been obtained.
 * @parms: The input parameter from terminal.
 */
void multi_threads_run(long int input_param, long op_round)
{
	// Add your code here

	// unlink semaphore
	sem_unlink(digit_0_25_Tsoi_Yiu_Chik);
	sem_unlink(digit_1_25_Tsoi_Yiu_Chik);
	sem_unlink(digit_2_25_Tsoi_Yiu_Chik);
	sem_unlink(digit_3_25_Tsoi_Yiu_Chik);
	sem_unlink(digit_4_25_Tsoi_Yiu_Chik);
	sem_unlink(digit_5_25_Tsoi_Yiu_Chik);
	sem_unlink(digit_6_25_Tsoi_Yiu_Chik);
	sem_unlink(digit_7_25_Tsoi_Yiu_Chik);
	sem_unlink(digit_8_25_Tsoi_Yiu_Chik);


	// open semaphore
	digit_access_semaphore[0] = sem_open(digit_0_25_Tsoi_Yiu_Chik, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1);
	digit_access_semaphore[1] = sem_open(digit_1_25_Tsoi_Yiu_Chik, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1);
	digit_access_semaphore[2] = sem_open(digit_2_25_Tsoi_Yiu_Chik, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1);
	digit_access_semaphore[3] = sem_open(digit_3_25_Tsoi_Yiu_Chik, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1);
	digit_access_semaphore[4] = sem_open(digit_4_25_Tsoi_Yiu_Chik, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1);
	digit_access_semaphore[5] = sem_open(digit_5_25_Tsoi_Yiu_Chik, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1);
	digit_access_semaphore[6] = sem_open(digit_6_25_Tsoi_Yiu_Chik, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1);
	digit_access_semaphore[7] = sem_open(digit_7_25_Tsoi_Yiu_Chik, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1);
	digit_access_semaphore[8] = sem_open(digit_8_25_Tsoi_Yiu_Chik, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 1);

	pthread_t threads[THREAD_COUNT];
	struct t_args thread_args[THREAD_COUNT];

	// create thread
	for (int i = 0; i < THREAD_COUNT; i++)
	{
		thread_args[i].digit_posi = i;
		thread_args[i].loop_count = op_round;
		pthread_create(&threads[i], NULL, &the_thread, (void *)&thread_args[i]);
	}

	// create thread
	for (int i = 0; i < THREAD_COUNT; i++)
	{
		pthread_join(threads[i], NULL);
	}

	// close semaphore
	for (int i = 0; i < THREAD_COUNT; i++)
	{
		sem_close(digit_access_semaphore[i]);
	}

	// unline semaphore
	sem_unlink(digit_0_25_Tsoi_Yiu_Chik);
	sem_unlink(digit_1_25_Tsoi_Yiu_Chik);
	sem_unlink(digit_2_25_Tsoi_Yiu_Chik);
	sem_unlink(digit_3_25_Tsoi_Yiu_Chik);
	sem_unlink(digit_4_25_Tsoi_Yiu_Chik);
	sem_unlink(digit_5_25_Tsoi_Yiu_Chik);
	sem_unlink(digit_6_25_Tsoi_Yiu_Chik);
	sem_unlink(digit_7_25_Tsoi_Yiu_Chik);
	sem_unlink(digit_8_25_Tsoi_Yiu_Chik);
}