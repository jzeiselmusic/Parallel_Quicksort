#ifndef HELPER_FUNCS_H
#define HELPER_FUNCS_H

#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>
#include <limits.h>
#include <stdio.h>


/* struct containing return value from *pick top or bottom k elements* */ 
struct PackedArrays {
	int* chosen_vals;	
	int chosen_vals_size;
	int* new_master_array;
	int new_master_array_size;
};

int* initialize_list(int N, int seed) {
    // return pointer to a list of random vals length N
    int i;
    time_t t;
    int * num_list;
    num_list = malloc(N*sizeof(int));
    srand((unsigned) seed);
    for (i = 0; i < N; i++) {
        num_list[i] = rand() % N;
    }
    return num_list;
}

void quick_sort(int* passed_list, int lo, int hi) {
    // do quick sort on passed list with lowest value lo and highest value hi
    int pivot_val;
    int i, j;
    int temp;
    if (lo >= 0 && hi >= 0 && lo < hi) {
        pivot_val = passed_list[(int)(floor(((float)(hi - lo))/2.0)) + lo];

        i = lo - 1;
        j = hi + 1;

        while (1) {
            i = i + 1;
            while (passed_list[i] < pivot_val) {
                i = i + 1;
            }
            j = j - 1;
            while (passed_list[j] > pivot_val) {
                j = j - 1;
            }

            if (i >= j) {
                pivot_val = j;
                break;
            }

            temp = passed_list[i];
            passed_list[i] = passed_list[j];
            passed_list[j] = temp;
        }

        quick_sort(passed_list, lo, pivot_val);
        quick_sort(passed_list, pivot_val + 1, hi);
    }
}


void bubble_sort(int* passed_list, int N) {
    // do bubble sort on passed list of length N
    int i = N - 1;
    int k;
    while ( i >= 0 ) {
        char flag = 0;
        for ( k = 0; k < i; k++) {
            if (passed_list[k] > passed_list[k + 1]) {
                int temp = passed_list[k + 1];
                passed_list[k + 1] = passed_list[k];
                passed_list[k] = temp;
                flag = 1;
            }
        }
        if (flag == 0) {
            break;
        }
        i = i - 1;
    }
}


void insertion_sort(int* passed_list, int N) {
    // do insertion sort on passed list of length N
    int i = 1;
    int j;
    int tmp;
    while (i < N) {
        j = i;
        while ((j > 0) && (passed_list[j-1] > passed_list[j])) {
            tmp = passed_list[j-1];
            passed_list[j-1] = passed_list[j];
            passed_list[j] = tmp;
            j--;
        }
        i++;
    }
}

bool check_sorted_lo_hi(int* passed_list, int N) {
    // test to see whether passed list is sorted low to high
    while (true) {
        if ((N == 0) || (N == 1)) {
            return true;
        }
        else {
            if (passed_list[N-1] < passed_list[N-2]) {
                return false;
            }
            else {
                N --; 
            }
        }
    }
}


int isArraySorted(int s[], int n) {
  // 1 is returned sorting ascending
  // 2 is returned sorting descending
  // 0 is not sorted
  int a = 1, d = 1, i = 0;

  while ((a == 1 || d == 1) && i < n - 1) {
    if (s[i] < s[i+1])
      d = 0;
    else if (s[i] > s[i+1])
      a = 0;
    i++;
  }

  if (a == 1)
    return 1;
  else if (d == 1)
    return 2;
  else
    return 0;
}

float num_inversions(int* passed_list, int N) {
    // return the number of inversions 
    // i.e. the value representing the amount of unsortedness
    int invs = 0;
    int i,j,k, iterations;
    time_t t;
    srand((unsigned) time(&t));
    for (i = 0; i < N; i++) {
        do {
            j = rand() % N;
            k = rand() % N;
        } while (j == k);
        if (j < k) {
            if (passed_list[j] > passed_list[k]) {
                invs ++;
            }
        }
        else if (j > k) {
            if (passed_list[k] > passed_list[j]) {
                invs ++;
            }
        }
    }
    float N_f = (float)N;
    float invs_f = (float)invs;
    return invs_f/N_f;
}


int pick_a_pivot(int* passed_list, int N) {
	// one possible way of finding a pivot value in a list of N numbers
	// pivot value should be close to median
	
	// this algorithm is to take median of first, second, last, second to last, and middle value
	int vals[5] = {passed_list[0],
		       passed_list[1],
		       passed_list[N-1],
		       passed_list[N-2],
		       passed_list[(int)floor(N/2)]};
	// sort these numbers using bubblesort
	bubble_sort(vals, 5);
	
	return vals[3];
}

int pick_a_rand_pivot(int* passed_list, int N, int numvals, int seed) {
	// another way of picking a pivot
	// find numvals random points in list of length N, then find the median of those
	int pivot;
	time_t t;
	int pivot_list[numvals];
	int i;
	if (numvals < N) {
		srand((unsigned) seed);		
		for (i = 0; i < numvals; i++) {
			pivot_list[i] = passed_list[(int)(rand() % N)];
		}
		quick_sort(pivot_list, 0, numvals-1);
		pivot = pivot_list[(int)(numvals/2)];
		return pivot;
	}
	else {
		return -1;
	}

}


void pick_top_k_values(int* passed_list, int N, int k, struct PackedArrays* return_struct) {
	// takes in malloc'd list, returns struct with 2 new malloc'd lists. 
	// remember to free this list when done!!!
	int* return_list = malloc(k*sizeof(int));
	int* new_master_list = malloc(N*sizeof(int));
	int master_iter = 0;	
	int flag = 0;
	int i;	
	int j;
	int temp;
	int myval;
	for (i = 0; i < k; i++) {
		return_list[i] = -INT_MAX;
	}
	for (i = 0; i < N; i++) {
		flag = 0;
		myval = passed_list[i];
		for (j = k-1; j >= 0; j--) {
			if (myval >= return_list[j]) {
				flag = 1;
				temp = return_list[j];
				return_list[j] = myval;
				myval = temp;
				if (j == 0) {
					flag = 2;
				}
			}
		}
		// place myval back in the new master list if 
		// 1. it didnt get popped into the return list
		// 2. if it got replaced by another value in the return list
		if ((flag == 0) || (flag == 2)) {
			if (myval != -INT_MAX) {		
				new_master_list[master_iter] = myval;
				master_iter += 1;
			}
		} 
	}
	// resize master_list to actual size //
	new_master_list = realloc(new_master_list, master_iter*sizeof(int));

	return_struct->chosen_vals = return_list;
	return_struct->chosen_vals_size = k;
	return_struct->new_master_array = new_master_list;
	return_struct->new_master_array_size = master_iter;
}


void pick_bottom_k_values(int* passed_list, int N, int k, struct PackedArrays* return_struct) {
	// remember to free this list when done!!
	// returns struct with 2 mallocd lists
	int* return_list = malloc(k*sizeof(int));
	int* new_master_list = malloc(N*sizeof(int));
	int flag = 0;
	int master_iter = 0;
	int pushed_val;

	int i;
	int j;
	int kk;
	int temp;
	int myval;
	for (i = 0; i < k; i++) {
		return_list[i] = INT_MAX;
	}
	for (i = 0; i < N; i++) {
		flag = 0;
		myval = passed_list[i];
		if (myval < return_list[0]) {
			flag = 2;
			if (return_list[0] != INT_MAX) {
				flag = 1;
				pushed_val = return_list[0];
			}
			for (j = 0; j < k-1; j++) {
				return_list[j] = return_list[j+1];
			}
			return_list[k-1] = myval;
			for (j = k-2; j >= 0; j--) {
				if (myval > return_list[j]) {
					temp = return_list[j];
					return_list[j] = myval;
					return_list[j+1] = temp;
				}
				else {
					break;
				}
			}
		}
		if (flag == 1) {
			new_master_list[master_iter] = pushed_val;
			master_iter += 1;
		}
		else if (flag == 0) {
			new_master_list[master_iter] = myval;
			master_iter += 1;
		}
	}
	new_master_list = realloc(new_master_list, master_iter*sizeof(int));

	return_struct->chosen_vals = return_list;
	return_struct->chosen_vals_size = k;
	return_struct->new_master_array = new_master_list;
	return_struct->new_master_array_size = master_iter;	
}

bool a_in_b(int a, int* b, int N) {
	// check if int a is in int* list b of size N
	int i;
	for (i = 0; i < N; i++) {
		if (b[i] == a) {
			return true;
		}
	}
	return false;
}

void sorting_order(int ma_zero, int* sorting_order, int N, int* nls, int** nl) {
	// look through master and sublists, return sorting order of length N 
	// where the number in the array refers to the i value of the lists in order
	// and i = -1 refers to the master array
	
	// create list to hold processors we have placed in sorting order list
	int* proc_list_done = malloc(N*sizeof(int));
	int proc_list_done_size = 0;

	// go through sub lists and master list and find smallest
	// when smallest is found, put that id in the sorting order list
	// and take that id out of the proc list and into the proc list done list
	int min_val;
	int min_id;
	int i, iter, k;
	for (iter = 0; iter < N; iter++) {
		min_val = INT_MAX;
		if (a_in_b(-1, proc_list_done, proc_list_done_size) == 0) {
			min_id = -1; // -1 refers to my own master list
			min_val = ma_zero;
		}
		for (i = 0; i < (N-1); i++) {
			if (nls[i] > 0) {
				if (nl[i][0] < min_val) {
					if (a_in_b(i, proc_list_done, proc_list_done_size) == 0) {
						min_val = nl[i][0];
						min_id = i;
					}
				}
			}
		}
		sorting_order[iter] = min_id;
		proc_list_done[proc_list_done_size] = min_id;
		proc_list_done_size++;
	}
	free(proc_list_done);
}


void print_array(int* passed_list, int N, int myid) {
	int i;
	printf("myid: %d\n", myid);
	for (i = 0; i < N; i++) {
		printf("%d ", passed_list[i]);
	}
	printf("\n");
}



void concatenate_lists(int** lists, int* list_sizes, int t_num,
					int* new_master, int total_size) {

	// new master is an allocd array of size master_array plus all neighbor
	// sizes. which should equal total size

	// lists contains neighbor lists and master list
	// t_num is total number of neighbor lists plus master list
	// which is lprocs*rounds + 1

	int i, j;
	int min = INT_MAX;
	int min_index;
	int* done_list = calloc(t_num, sizeof(int));
	int master_iter = 0;

	for (j = 0; j < t_num; j++) {
		min = INT_MAX;
		for (i = 0; i < t_num; i++) {
			if (done_list[i] != 1) {
				min_index = lists[i][0] <= min ? i : min_index;
				min = lists[i][0];
			}
		}
		done_list[min_index] = 1;
		for (i = 0; i < list_sizes[min_index]; i++) {
			new_master[master_iter++] = lists[min_index][i];
		}
	}

	free(done_list);
}


float calculateSD(int* data, int N) {
    float sum = 0.0, mean, SD = 0.0;
    int i;

    for (i = 0; i < N; ++i) {
        sum += (float)data[i];
    }

    mean = sum / N;

    for (i = 0; i < N; ++i) {
        SD += pow((float)data[i] - mean, 2);
    }

    return sqrt(SD / N);
}



#endif
