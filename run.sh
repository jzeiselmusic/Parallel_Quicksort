NAME=job_LB
echo "Submitting: ${NAME}"

PBS="#!/bin/bash\n\
#PBS -N ${NAME}\n\
#PBS -l nodes=8:ppn=4\n\
#PBS -l walltime=00:05:00\n\
#PBS -oe\n\
#PBS -q edu_shared\n\
##PBS jzeise2@uic.edu\n\
module load MPICH/3.3.2-GCC-9.3.0\n\
cd \$PBS_O_WORKDIR\n\
mpicc MPI_QS_LB.c -o ./build/mpi_qs_lb -lm\n\
mpirun ./build/mpi_qs_lb 100000 3"

echo -e ${PBS} | qsub
sleep 5.0
echo "done."

NAME=job_NLB
echo "Submitting: ${NAME}"

PBS="#!/bin/bash\n\
#PBS -N ${NAME}\n\
#PBS -l nodes=8:ppn=4\n\
#PBS -l walltime=00:05:00\n\
#PBS -j oe\n\
#PBS -q edu_shared\n\
##PBS jzeise2@uic.edu\n\
module load MPICH/3.3.2-GCC-9.3.0\n\
cd \$PBS_O_WORKDIR\n\
mpicc MPI_QS.c -o ./build/mpi_qs -lm\n\
mpirun ./build/mpi_qs 100000 3"

echo -e ${PBS} | qsub
echo "done."
