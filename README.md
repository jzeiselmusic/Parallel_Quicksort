

/**********************************************************************************************/

Parallel Quicksort with Load Balancing 

1. The parallel quicksort program with load balancing is contained in file ./MPI_QS_LB.c
   The same program without load balancing is ./MPI_QS_NLB.c. Both programs are written only in C.

2. In order to compile either of these programs on their own run "mpicc $FILE -lm"
   In order to submit single jobs to Extreme server use quicksort_lb.pbs or quicksort_nlb.pbs.

   *NOTE* 
   when running the binary or submitting a pbs script, both executables (LB and NLB) take 2 arguments,
   N, and seed in that order. The seed is for generating random data. Different random data 
   will have a different S value, same seed random data will have the same S value. 

3. Both programs "import" the file helper_funcs.h which acts as a library for often used functions

4. In order to submit a string of jobs with varying P and N values, edit and run ./run_N_P.sh
   this shell script will create a .pbs file and submit jobs in a for-loop

   In order to submit jobs that have varying seeds for generating different random data but a static
   number of N and P, run ./run_seeds.sh. In ./run_N_P.sh the seed is defined as N+P

5. In order to clean the output and start over, run ./clean.sh.

   *NOTE* 
   the ./clean.sh script will get rid of QS_LB_stats.txt file. If you want to keep this for 
   adding data to it over time, delete this or comment out from the ./clean.sh script. 

6. As requested, MPI_QS_LB.c generates an output file called Sorted-LB_N_P.txt and MPI_QS_NLB.c outputs 
   a file called Sorted-NLB_N_P.txt. These files contain information about run time and skewness as well as
   the final sorted values. Note that this file can get very long if N is very large.

   These programs also write to (or create, if it doesn't already exist) a file called QS_LB_stats.txt 
   that contains more information such as time with pivoting and time without pivoting. 

7. Both programs contain commented out code for printing outputs to standard output. This was used 
   for debugging purposes but is left in the code in case any inquiries are made into what the data
   looks like at different stages of the sorting process in each processor. 

/**********************************************************************************************/

