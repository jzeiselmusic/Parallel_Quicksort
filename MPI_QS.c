#include <mpi.h>
#include <stdio.h>
#include "helper_funcs.h"

int main(int argc, char** argv) {
	int MAX_ARRAY_SIZE = 1000000;
	int numprocs, myid;
	int i; // used for loops
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);

	int* master_array; // this is the array that starts in the master
	if (myid == 0){
		master_array = initialize_list(MAX_ARRAY_SIZE);
	}
	else {
		// needs to be declared for everyone because we will be 
		// storing local lists at this address
		master_array = malloc(MAX_ARRAY_SIZE*sizeof(int));
	}
	int rounds[3] = {0, 1, 3};
	int array_size = MAX_ARRAY_SIZE;
	int k;
	int pivot;
	int actual_receive;
	int array_lo_iter, array_hi_iter;
	MPI_Status receive_handle;

	MPI_Barrier(MPI_COMM_WORLD);
	
	// three rounds
	for (k = 0; k < (int)log2(numprocs); k++) {
		// senders are 0, then 0 and 1, then 0,1,2,3
		if (myid <= rounds[k]) {
			// pick a pivot point for master array
			pivot = pick_a_rand_pivot(master_array, array_size, 31);
			// dynamically allocate low list and high list
			int* array_lo = malloc(array_size*sizeof(int));
			int* array_hi = malloc(array_size*sizeof(int));
			array_lo_iter = 0;
			array_hi_iter = 0;
			// split master array into low list and high list
			// at the end, array_lo_iter+1 will be length of low list
			// and array_hi_iter+q will be length of high list
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
			array_lo = realloc(array_lo,(array_lo_iter+1)*sizeof(int));
			array_hi = realloc(array_hi, (array_hi_iter+1)*sizeof(int));
			// send high list to next processor
			MPI_Send(array_hi, array_hi_iter+1, 
				MPI_INT, myid^((int)pow(2, k)), k, MPI_COMM_WORLD);
			printf("myid: %d sent data: %d to %d\n", myid, array_hi_iter+1, myid^((int)pow(2,k)));
			// we no longer need data pointed to by master_list
			free(master_array);
			// master array is now the lower list
			master_array = array_lo;
			// size of master array is now size of lower list
			array_size = array_lo_iter+1;
			// we no longer need high list because we sent it
			free(array_hi);
		}
		else if ((myid^((int)pow(2,k))) <= rounds[k]) {
			// create a temporary array to hold received data, max size is size of unsplit list
			int* temp_array = malloc(array_size*sizeof(int));
			// receive list
			MPI_Recv(temp_array, array_size, MPI_INT,
				myid^((int)pow(2,k)), k, MPI_COMM_WORLD,
				&receive_handle);
			// get the actual number of data elements received aka the sent list length
			MPI_Get_count(&receive_handle, MPI_INT, &actual_receive);
			printf("myid: %d received data: %d from %d\n", myid, actual_receive, myid^((int)pow(2,k)));
			if (actual_receive < array_size) {
				// truncate allocated temporary array to length of actual sent list length
				temp_array = realloc(temp_array, actual_receive*sizeof(int)); 
			}
			// we no longer need memory pointed to by master array
			free(master_array);
			// master array will now point to temporary array
			master_array = temp_array;
			// master array size is received data size
			array_size = actual_receive;
		}
	}

	// at this point all processors should have their own array of data in master_array of size array_size
	//
	// tests to make sure data is correct
	MPI_Barrier(MPI_COMM_WORLD);
	printf("myid: %d\n\tnumelements: %d\n", myid, array_size);

	MPI_Barrier(MPI_COMM_WORLD);
	printf("myid: %d\n\tis sorted: %s\n", myid, check_sorted_lo_hi(master_array, array_size) ? "yes" : "no");
	//
	// now do sorting on every processor
	//
	insertion_sort(master_array, array_size);
	
	// now check if all processors are sorted
	//
	//
	MPI_Barrier(MPI_COMM_WORLD);
	printf("myid: %d\n\tis sorted: %s\n", myid, check_sorted_lo_hi(master_array, array_size) ? "yes" : "no");	
	return 0;
}
