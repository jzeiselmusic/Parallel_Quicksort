#ifndef HELPER_FUNCS_H
#define HELPER_FUNCS_H

#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>

int* initialize_list(int N) {
    // return pointer to a list of random vals length N
    int i;
    time_t t;
    int * num_list;
    num_list = malloc(N*sizeof(int));
    srand((unsigned) time(&t));
    for (i = 0; i < N; i++) {
        num_list[i] = rand();
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

#endif
