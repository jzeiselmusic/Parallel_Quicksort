#include "helper_funcs.h"
#include <stdio.h>
#define N 10000

int main() {
	int* my_list;
	my_list = initialize_list(N);
	bool is_sorted_first = check_sorted_lo_hi(my_list, N);
	quick_sort(my_list, 0, N-1);
	bool is_sorted_now = check_sorted_lo_hi(my_list,N);

	printf("array starts %s\n", is_sorted_first ? "sorted" : "unsorted");
	printf("array ends %s\n", is_sorted_now ? "sorted" : "unsorted");
	return 0;
}
