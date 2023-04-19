#include "helper_funcs.h"
#include <stdio.h>
#define N 10000



int main() {
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


int main2() {
	int mylist[15] = {10, 5, 1, 2, 11, 3, 15, 20, 19, 18, 100, 100, 100, 30, 41};
	int* highest_values = pick_bottom_k_values(mylist, 15, 4);
	int i;
	for (i = 0; i < 4; i++) {
		printf("%d\n", highest_values[i]);
	}
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
