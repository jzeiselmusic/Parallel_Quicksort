#include <mpi.h>
#include <stdio.h>
#include <unistd.h>
#include "helper_funcs.h"

#define barrier	  MPI_Barrier(MPI_COMM_WORLD)
#define rounds    4

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

		printf("load balancing version\n");
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
	int hex_val = 0xFFFF ^ (numprocs - 1);

	double t1, t2; // used for timing
	MPI_Status receive_handle;

/*************/

/*************/
	
	// start the timer clock
	if (myid == 0) {
		t1 = MPI_Wtime();
	}
	// logp rounds of scattering the master array
	for (k = 0; k < lprocs; k++) {
		int idcheck = myid & (hex_val >> k);
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
			MPI_Send(array_hi, array_hi_iter, 
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
		else if ((idcheck_2 & (hex_val >> k)) == idcheck_2) {
			// probe to get length of incoming message
			MPI_Probe(myid^((int)pow(2,lprocs - k - 1)), k,
							MPI_COMM_WORLD, &receive_handle);
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
/**************/
barrier;
/**************/
	}


/*************/

/*************/
	// at this point all processors should have their own 
	// array of data in master_array of size array_size
	//
	// we now want to do load balancing
	// every processor first calculates its own load value.
	// then exchanges it with its nearest neighbor by the first bit
	// then exchange data with that neighbor if necessary
	// do this logP times for every bit diff
	int bit_flipper;
	int my_load, my_actual_load;
	int ne_load, ne_actual_load;
	int num_to_send;
	int num_to_receive;
	bool higherlower; // true if sending to higher, false for sending to lower

	int send_role;

	// struct to help with picking top/bottom values and changing master array
	struct PackedArrays top_bottom_vals_return;

	// need to store a list [] of pointers that hold first mem location of neighboring data lists
	int* neighbor_lists[lprocs*rounds];
	int* neighbor_list_sizes = calloc(lprocs*rounds, sizeof(int)); // use 3 so we can initialize array to 0

	// need to keep track of who myproc has received data from
	// and who myproc has sent data to
	int* data_sent_to = calloc(lprocs*rounds, sizeof(int));  // will be 0 if not, 1 if yes
	int* data_recv_from = calloc(lprocs*rounds, sizeof(int)); // will be 0 if not, 1 if yes

	int* temp_pointer;

	int iter; // used for printing / debugging
	int ii; // ii is the value that represents i % lprocs because we are doing 3 rounds 
			// so in this case ii will go from 0 to 2 and i will go from 0 to 8

	MPI_Request request;

	for (i = 0; i < (lprocs*rounds); i++) {
		ii = i % lprocs;
		bit_flipper = (int)pow(2, ii);
		my_actual_load = array_size; // for now, we will say the load 
									 // is just size of my array
		my_load = my_actual_load;
		for (iter = 0; iter < (lprocs*rounds); iter++) {
			my_load += neighbor_list_sizes[iter];
		}
		if (myid > (myid^bit_flipper)) {
			// send first then receive
			MPI_Ssend(&my_load, 1, MPI_INT, myid^bit_flipper, 100, MPI_COMM_WORLD);
			MPI_Recv(&ne_load, 1, MPI_INT, myid^bit_flipper, 101, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Ssend(&my_actual_load, 1, MPI_INT, myid^bit_flipper, 102, MPI_COMM_WORLD);
			MPI_Recv(&ne_actual_load, 1, MPI_INT, myid^bit_flipper, 103, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		} 
		else {
			// receive first then send
			MPI_Recv(&ne_load, 1, MPI_INT, myid^bit_flipper, 100, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Ssend(&my_load, 1, MPI_INT, myid^bit_flipper, 101, MPI_COMM_WORLD);
			MPI_Recv(&ne_actual_load, 1, MPI_INT, myid^bit_flipper, 102, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Ssend(&my_actual_load, 1, MPI_INT, myid^bit_flipper, 103, MPI_COMM_WORLD);
		}
		if (my_load >= 1.2*ne_load) {
			send_role = 1; // role is sender
		}
		else if (my_load < .8*ne_load) {
			send_role = 0; // role is receiver
		}
		else {
			send_role = -1; // role is do nothing
		}

		int num_to_send_recv = floor(abs(my_load - ne_load)/4.0);

		if (num_to_send_recv > 100) {
			
			if (send_role == 1) {
				// if sending node, find diff between my load and ne load
				if (my_actual_load > num_to_send_recv) {
					// declare an array for holding max/min num_to_send values
					int* array_to_send;
					// figure out whether sending to higher or lower node
					higherlower = ((myid^bit_flipper) > myid) ? true : false;
					if (higherlower) {
						pick_top_k_values(master_array, array_size, num_to_send_recv, 
														&top_bottom_vals_return);
					}
					else {
						pick_bottom_k_values(master_array, array_size,num_to_send_recv,
														&top_bottom_vals_return);
					}
					// free this because we have a new master array
					free(master_array);
					master_array = top_bottom_vals_return.new_master_array;
					array_size = top_bottom_vals_return.new_master_array_size;
					array_to_send = top_bottom_vals_return.chosen_vals;
					num_to_send = top_bottom_vals_return.chosen_vals_size;

					// send array to neighbor
					MPI_Send(array_to_send, num_to_send_recv, MPI_INT, myid^bit_flipper,
						ii, MPI_COMM_WORLD);
					// always make sure to free this array after sending it away
					free(array_to_send);

					data_sent_to[i] = 1;
				}
			}
			else if (send_role == 0) {

				// create a new list for values received from neighbor
				// and then store that in a list of lists
				if (ne_actual_load > num_to_send_recv) {
					temp_pointer = malloc(num_to_send_recv*sizeof(int));
					MPI_Recv(temp_pointer, num_to_send_recv, MPI_INT, 
						myid^bit_flipper, ii, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 

					neighbor_lists[i] = temp_pointer;
					neighbor_list_sizes[i] = num_to_send_recv;

					data_recv_from[i] = 1;
				}
			}
		}
/**********/
barrier;
/**********/
	}


	// now every processor must sort its own master list and all of its sublists
	// if it has any sublists
	//

	insertion_sort(master_array,array_size);
	for (i = 0; i < (lprocs*rounds); i++) {
		if (neighbor_list_sizes[i] > 0) {
			insertion_sort(neighbor_lists[i], neighbor_list_sizes[i]);
		}
	}

	// end the timer clock
	if (myid == 0) {
		t2 = MPI_Wtime();
	}

/*
	int receive_buf;
	int	send_buf = 1;
	for (i = 0; i < numprocs; i++) {
	if (myid == i) {
		printf("\nmyid: %d\n", myid);
		if (array_size > 0) {
			printf("master array: \n");
			for (j = 0; j < array_size; j++) {
				printf("%d ", master_array[j]);
			}
			printf("\n");
		}
		else {
			printf("none\n");
		}
		for (j = 0; j < (lprocs*rounds); j++) {
			if (neighbor_list_sizes[j] > 0) {
				printf("sublist%d: \n", j);
				for (k = 0; k < neighbor_list_sizes[j]; k++) {
					printf("%d ", neighbor_lists[j][k]);
				}
				printf("\n");
			}
			else {
				printf("sublist%d: \n", j);
				printf("none\n");
			}
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
*/
	// now all arrays and subarrays should be sorted
	// we need to send these back to their original owners
	// we can do this by doing the same FOR loop as above, 
	// and swapping with neighbors

/*
	MPI_Status receive_handle_two;
	int receive_amount;
	for (i = 0; i < (lprocs*rounds); i++) {
		ii = i % lprocs;
		bit_flipper = (int)pow(2, ii);

		// every round everyone does a send and a receive
		if (myid > (myid^bit_flipper)) {
			if (data_recv_from[i] == 1) {
				// do a blocking send first, then a blocking receive
				MPI_Send(neighbor_lists[i], neighbor_list_sizes[i], MPI_INT, 
						myid ^ bit_flipper, i, MPI_COMM_WORLD);
				free(neighbor_lists[i]);
			}
			else if (data_sent_to[i] == 1) {
				// check to see the size of incoming list from neighbor
				MPI_Probe(myid^bit_flipper, i,
							MPI_COMM_WORLD, &receive_handle_two);
				MPI_Get_count(&receive_handle_two, MPI_INT, &receive_amount);
				// size of incoming list stored in receive amount
				int* temp_incoming_list = malloc(receive_amount*sizeof(int));
				MPI_Recv(temp_incoming_list, receive_amount, MPI_INT, 
					myid^bit_flipper, i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				neighbor_lists[i] = temp_incoming_list;
				neighbor_list_sizes[i] = receive_amount;
			}
		}
		else {
			// do the same thing as above but in reverse order
			if (data_sent_to[i] == 1) {
				// check to see the size of incoming list from neighbor
				MPI_Probe(myid^bit_flipper, i,
							MPI_COMM_WORLD, &receive_handle_two);
				MPI_Get_count(&receive_handle_two, MPI_INT, &receive_amount);
				// size of incoming list stored in receive amount
				int* temp_incoming_list = malloc(receive_amount*sizeof(int));
				MPI_Recv(temp_incoming_list, receive_amount, MPI_INT, 
					myid^bit_flipper, i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				neighbor_lists[i] = temp_incoming_list;
				neighbor_list_sizes[i] = receive_amount;
			}
			else if (data_recv_from[i] == 1) {
				MPI_Send(neighbor_lists[i], neighbor_list_sizes[i], MPI_INT, 
						myid ^ bit_flipper, i, MPI_COMM_WORLD);
				free(neighbor_lists[i]);
			}
		}
barrier;
	} 
	
	// after above message passing, every proc should have its own sent 
	// data back in its own memory, replacing the data it had 
	// from its neighbor processors

*/

barrier;

	if (myid == 0) {
		printf("\n\ntotal time: %.4f\n", t2 - t1);
	}
	return 0;
}
