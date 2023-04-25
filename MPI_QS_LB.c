#include <mpi.h>
#include <stdio.h>
#include <unistd.h>
#include "helper_funcs.h"

int main(int argc, char** argv) {
	int MAX_ARRAY_SIZE = 80000;
	int numprocs, myid;
	int i, j; // used for loops
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);

	int* master_array; // this is the array that starts in the master
	if (myid == 0){
		master_array = initialize_list(MAX_ARRAY_SIZE);

		/*printf("%d: \n", myid);
		for (i = 0; i < MAX_ARRAY_SIZE; i++) {
			printf("%d ", master_array[i]);
		}
		printf("\n");*/
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

	MPI_Barrier(MPI_COMM_WORLD);
	
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
			pivot = pick_a_rand_pivot(master_array, array_size, 2);
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
			MPI_Probe(myid^((int)pow(2,lprocs - k - 1)), k,MPI_COMM_WORLD, &receive_handle);
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
	MPI_Barrier(MPI_COMM_WORLD);
	}

	printf("myid:  %d   numvals:   %d\n", myid, array_size);
	MPI_Barrier(MPI_COMM_WORLD);
	if (myid == 0) {
		printf("#################\n");
	}
	// at this point all processors should have their own array of data in master_array of size array_size
	//
	// we now want to do load balancing
	// every processor first calculates its own load value.
	// then exchanges it with its nearest neighbor by the first bit
	// then exchange data with that neighbor if necessary
	// do this logP times for every bit diff
	int bit_flipper;
	int my_load;
	int my_actual_load;
	int ne_load;
	int num_to_send;
	int num_to_receive;
	bool higherlower; // true if sending to higher, false for sending to lower

	int send_role;

	// struct to help with picking top/bottom values and changing master array
	struct PackedArrays top_bottom_vals_return;

	// need to store a list [] of pointers that hold first mem location of neighboring data lists
	int* neighbor_lists[3];
	int neighbor_list_sizes[3] = {0, 0, 0};
	int* temp_pointer;

	int iter;

	for (i = 0; i < lprocs; i++) {
		bit_flipper = (int)pow(2, i);
		my_actual_load = array_size; // for now, we will say the load is just size of my array
		my_load = my_actual_load;
		for (iter = 0; iter < lprocs; iter++) {
			my_load += neighbor_list_sizes[iter];
		}
		printf("rd%d   myid:  %d   numvals:   %d\n", i, myid, my_load);
		MPI_Sendrecv(&my_load, 1, MPI_INT, myid^bit_flipper,
				i, &ne_load, 1, MPI_INT, myid^bit_flipper,
				i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		if ((my_load > 100) && (my_load >= 1.2*ne_load)) {
			send_role = 1; // role is sender
		}
		else if ((ne_load > 100) && (my_load <= 0.8*ne_load)) {
			send_role = 0; // role is receiver
		}
		else {
			send_role = -1; // role is do nothing
		}
		
		if (send_role == 1) {
			// if sending node, find diff between my load and ne load
			num_to_send = floor((my_load - ne_load)/2.0);
			// declare an array for holding max/min num_to_send values
			int* array_to_send;
			// figure out whether sending to higher or lower node
			higherlower = ((myid^bit_flipper) > myid) ? true : false;
			if (higherlower) {
				pick_top_k_values(master_array, array_size, num_to_send, &top_bottom_vals_return);
			}
			else {
				pick_bottom_k_values(master_array, array_size,num_to_send, &top_bottom_vals_return);
			}
			// free this because we have a new master array
			free(master_array);
			master_array = top_bottom_vals_return.new_master_array;
			array_size = top_bottom_vals_return.new_master_array_size;
			array_to_send = top_bottom_vals_return.chosen_vals;
			num_to_send = top_bottom_vals_return.chosen_vals_size;
			//printf("myid: %d  num_to_send:  %d\n", myid, num_to_send);  
			// send array to neighbor
			MPI_Ssend(array_to_send, num_to_send, MPI_INT, myid^bit_flipper,
				i, MPI_COMM_WORLD);
			// always make sure to free this array after sending it away
			free(array_to_send);
		}
		else if (send_role == 0) {
			num_to_receive = floor((ne_load - my_load)/2.0);
			//printf("myid:  %d  num_to_receive:  %d\n", myid, num_to_receive);
			temp_pointer = malloc(num_to_receive*sizeof(int));
			MPI_Recv(temp_pointer, num_to_receive, MPI_INT, 
				myid^bit_flipper, i, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
			neighbor_lists[i] = temp_pointer;
			neighbor_list_sizes[i] = num_to_receive;
		}
		MPI_Barrier(MPI_COMM_WORLD);
	}

	printf("myid:   %d   numvals:   %d\n", myid, my_load);
	// now do sorting on every processor
	//
	insertion_sort(master_array,array_size);
	
	MPI_Barrier(MPI_COMM_WORLD);
	if (myid == 0) {
		t2 = MPI_Wtime();
	}
	// now check if all processors are sorted
	//
	//
	/*
	int receive_buf;
	int send_buf = 1;
	for (i = 0; i < numprocs; i++) {
	if (myid == i) {
		printf("\nmyid: %d\n", myid);
		if (array_size > 0) {
			for (j = 0; j < array_size; j++) {
				printf("%d ", master_array[j]);
			}
		}
		else {
			printf("none\n");
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
	}
	}
	
	MPI_Barrier(MPI_COMM_WORLD);
	*/
	if (myid == 0) {
		printf("\n\ntotal time: %.4f\n", t2 - t1);
	}
	return 0;
}
