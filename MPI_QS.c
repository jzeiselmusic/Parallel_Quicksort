#include <mpi.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "helper_funcs.h"

#define barrier    MPI_Barrier(MPI_COMM_WORLD)

int main(int argc, char** argv) {
	int MAX_ARRAY_SIZE;
	int numprocs, myid;
	int i, j; // used for loops
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);

	int* master_array; // this is the array that starts in the master

		if (argc != 3) {
		return -1;
	}

	char* remaining;
	char* input_val = argv[1];
	MAX_ARRAY_SIZE = strtol(input_val, &remaining, 10);

	input_val = argv[2];
	int seed = strtol(input_val, &remaining, 10);

	if (myid == 0){
		master_array = initialize_list(MAX_ARRAY_SIZE, seed);

		printf("non-load balancing version\n");

	}
	else {
		// needs to be declared for everyone because we will be 
		// storing local lists at this address
		master_array = malloc(MAX_ARRAY_SIZE*sizeof(int));
	}
	int array_size = MAX_ARRAY_SIZE;
	int k;
	int pivot;
	int actual_receive;
	int array_lo_iter, array_hi_iter;
	int lprocs = (int)log2(numprocs);

	double t1, t2;
	MPI_Status receive_handle;

	barrier;
	
	// logp rounds
	if (myid == 0) {
		t1 = MPI_Wtime();
	}
	for (k = 0; k < lprocs; k++) {
		int idcheck = myid & (0xF8 >> k);
		int idcheck_2 = myid^((int)pow(2,lprocs - k - 1));
		// senders are 0, then 0 and 1, then 0,1,2,3
		if (idcheck == myid) {
			// pick a pivot point for master array
			pivot = pick_a_rand_pivot(master_array, array_size, 4, seed);
			// dynamically allocate low list and high list
			int* array_lo = malloc(array_size*sizeof(int));
			int* array_hi = malloc(array_size*sizeof(int));
			array_lo_iter = 0;
			array_hi_iter = 0;
			// split master array into low list and high list
			// at the end, array_lo_iter will be length of low list
			// and array_hi_iter will be length of high list
			for (i = 0; i < array_size; i++) {
				if (master_array[i] <= pivot) {
					array_lo[array_lo_iter] = master_array[i];
					array_lo_iter++;
				}
				else {
					array_hi[array_hi_iter] = master_array[i];
					array_hi_iter++;
				}
			}
			// truncate the split lists to the actual length of the data 	
			array_lo = realloc(array_lo,(array_lo_iter)*sizeof(int));
			array_hi = realloc(array_hi, (array_hi_iter)*sizeof(int));
			// send high list to next processor
			MPI_Ssend(array_hi, array_hi_iter, 
				MPI_INT, myid^((int)pow(2, lprocs - k - 1)), k, MPI_COMM_WORLD);
			// we no longer need data pointed to by master_list
			free(master_array);
			// master array is now the lower list
			master_array = array_lo;
			// size of master array is now size of lower list
			array_size = array_lo_iter;
			// we no longer need high list because we sent it
			free(array_hi);
		}
		else if ((idcheck_2 & (0xF8 >> k)) == idcheck_2) {
			// probe to get length of incoming message
			MPI_Probe(myid^((int)pow(2,lprocs - k - 1)), k, MPI_COMM_WORLD, &receive_handle);
			// use receive handle to get size of incoming message
			MPI_Get_count(&receive_handle, MPI_INT, &actual_receive);
			// create a temp array to hold recvd data
			int* temp_array = malloc(actual_receive*sizeof(int));
			// receive list
			MPI_Recv(temp_array, actual_receive, MPI_INT,
				myid^((int)pow(2, lprocs - k - 1)), k, MPI_COMM_WORLD,
				MPI_STATUS_IGNORE);
			// we no longer need memory pointed to by master array
			free(master_array);
			// master array will now point to temporary array
			master_array = temp_array;
			// master array size is received data size
			array_size = actual_receive;
		}
	barrier;
	}

	int* initial_array_size = malloc(numprocs*sizeof(int));
	MPI_Allgather(&array_size, 1, MPI_INT, initial_array_size, 
					1, MPI_INT, MPI_COMM_WORLD);
	// calculate initial load imbalance

	// find standard deviation of the initial balance
	// first find the mean
	float sd = calculateSD(initial_array_size, numprocs);
	float load_imbalance = sd / ((float)MAX_ARRAY_SIZE / numprocs);

	if (myid == 0) {
		printf("load balances: \n");
		for (i = 0; i < numprocs; i++ ) {
			printf("%d ", initial_array_size[i]);
		}
		printf("\n");
		printf("load imbalance: %.3f\n", load_imbalance);
	}

	// at this point all processors should have their own array of data in master_array of size array_size
	//
	barrier;

	//
	// now do sorting on every processor
	//
	insertion_sort(master_array,array_size);
	
	barrier;
	if (myid == 0) {
		t2 = MPI_Wtime();
	}
	
	MPI_File fh;
	MPI_File_open(MPI_COMM_WORLD, "/home/jzeise2/Parallel_Quicksort/Sorted-No-LB.txt",
					MPI_MODE_CREATE|MPI_MODE_WRONLY, MPI_INFO_ENV, &fh);

	int receive_buf;
	int send_buf = 0;
	char write_buf[256*4];

	if (myid == 0) {
		send_buf += sprintf(write_buf, 
			"N = %d, P = %d, s = 0, load-imbalance-metric: %.3f\n", 
			MAX_ARRAY_SIZE, numprocs, load_imbalance);
		MPI_File_write_at(fh, send_buf - strlen(write_buf), write_buf, strlen(write_buf), MPI_CHAR, MPI_STATUS_IGNORE);
	}

	for (i = 0; i < numprocs; i++) {
		if (myid == i) {
			send_buf += sprintf(write_buf, "Processor %d:\n", myid);
			MPI_File_write_at(fh, send_buf - strlen(write_buf), write_buf, strlen(write_buf), MPI_CHAR, MPI_STATUS_IGNORE);
			if (array_size > 0) {
				for (j = 0; j < array_size; j++) {
					if (j == array_size - 1) { 
						send_buf += sprintf(write_buf, "%d", master_array[j]);
					}
					else { 
						send_buf += sprintf(write_buf, "%d, ", master_array[j]);
					}
					MPI_File_write_at(fh, send_buf - strlen(write_buf), write_buf, strlen(write_buf), MPI_CHAR, MPI_STATUS_IGNORE);
				}
				send_buf += sprintf(write_buf, "\n");
				MPI_File_write_at(fh, send_buf - strlen(write_buf), write_buf, strlen(write_buf), MPI_CHAR, MPI_STATUS_IGNORE);
			}
			else {
				send_buf += sprintf(write_buf, "none\n");
				MPI_File_write_at(fh, send_buf - strlen(write_buf), write_buf, strlen(write_buf), MPI_CHAR, MPI_STATUS_IGNORE);
			}
			sleep(1);
			if (myid < (numprocs-1)) {
				MPI_Ssend(&send_buf, 1, MPI_INT, myid+1, 0, MPI_COMM_WORLD);
			}
			else {
				break;
			}
			break;
		}

		else if (myid == (i+1)) {
			MPI_Recv(&receive_buf, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			send_buf = receive_buf;
		}
	}
	barrier;

	MPI_File_close(&fh);

	MPI_File_open(MPI_COMM_WORLD, "/home/jzeise2/Parallel_Quicksort/QS_LB_stats.txt",
					MPI_MODE_CREATE|MPI_MODE_WRONLY|MPI_MODE_APPEND, MPI_INFO_ENV, &fh);

	if (myid == 0) {
		sprintf(write_buf, "Parallel Time w/o LB = %.4f\n", t2 - t1);
		MPI_File_write(fh, write_buf, strlen(write_buf), MPI_CHAR, MPI_STATUS_IGNORE);
	}
	MPI_File_close(&fh);


	if (myid == 0) {
		printf("\n\ntotal time: %.4f\n", t2 - t1);
	}

	free(master_array);
	MPI_Finalize();
	return 0;
}
