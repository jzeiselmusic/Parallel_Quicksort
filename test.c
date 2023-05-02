#include "helper_funcs.h"
#include <stdio.h>
#define N 10000



int main2() {
	int* a = malloc(20*sizeof(int));
	a[9] = 10;	
	a[8] = 5;
	a[7] = 4;
	a[6] = 6;
	a[5] = 3;
	a = realloc(a, 10*sizeof(int));
	int i;
	for (i = 0; i < 10; i++) {
		printf("%d ", a[i]);
	}
	printf("\n");
	return 0;
}


int main10() {
	int test = 0xFFFF;
	int flipper = test ^ (8 - 1);
	int mylist[15] = {1, 5, 2, 2, 11, 50, 15, 3, 19, 18, 100, 100, 100, 30, 41};
	struct PackedArrays p;
	pick_top_k_values(mylist, 15, 4, &p);
	int i;
	printf("starting list: \n");
	for (i = 0; i < 15; i++) {
		printf("%d ", mylist[i]);
	}
	printf("\n");
	printf("returnted list: \n");
	printf("size: %d\n", p.chosen_vals_size);
	for (i = 0; i < p.chosen_vals_size; i++) {
		printf("%d ", (p.chosen_vals)[i]);
	}
	printf("\n");
	printf("the old list: \n");
	printf("size: %d\n", p.new_master_array_size);
	for (i = 0; i < p.new_master_array_size; i++) {
		printf("%d ", (p.new_master_array)[i]);
	}
	printf("\n");
	return 0;
}

int main1() {
	int* my_list;
	my_list = initialize_list(N);
	bool is_sorted_first = check_sorted_lo_hi(my_list, N);
	quick_sort(my_list, 0, N-1);
	bool is_sorted_now = check_sorted_lo_hi(my_list,N);

	printf("array starts %s\n", is_sorted_first ? "sorted" : "unsorted");
	printf("array ends %s\n", is_sorted_now ? "sorted" : "unsorted");
	return 0;
}

int main4() {
	int master[10] = {7,8,9,10,11,12,13,14,15,16};
	int neighbor_list_2[6] = {1,2,3,4,5,6};
	int neighbor_list_3[4] = {17,18,19,20};
	int neighbor_list_4[3] = {22,23,24};
	int neighbor_list_1[5] = {30,31,32,33,34};
	int neighbor_list_sizes[2] = {6, 4};

	int* neighbor_lists[4] = {neighbor_list_1, neighbor_list_2, neighbor_list_3, neighbor_list_4};

	int sorted_ordering[5];

	//sorting_order(master, sorted_ordering, 5, neighbor_list_sizes, neighbor_lists);

	int i;
	for (i = 0; i < 5; i++ ) {
		printf("%d ", sorted_ordering[i]);
	}
	printf("\n");
	return 0;
}

int main() {

	//int list1[5] = {1,2,3,4,5};
	int list2[6] = {10,11,12,13,14,15};
	int list3[2] = {0,0};
	//int list4[5] = {100,110,112,114,115};

	int* full_list[2] = {list2, list3};//, list3, list4};
	int full_list_sizes[2] = {6,2};
	
	int total_size = 8;

	int* new_master = malloc(total_size*sizeof(int));

	concatenate_lists(full_list, full_list_sizes, 2,
					new_master, total_size);


	int i;
	for (i = 0; i < total_size; i++) {
		printf("%d ", new_master[i]);
	}
	printf("\n");

	free(new_master);

	return 0;
}