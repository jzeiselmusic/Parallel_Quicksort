
seed=(10 20 30 40 50 60 70 80 90 100)

for s in ${seed[@]}
do

        NAME=job_LB_${s}
        echo "Submitting: ${NAME}"

        PBS="#!/bin/bash\n\
        #PBS -N ${NAME}\n\
        #PBS -l nodes=8:ppn=2\n\
        #PBS -l walltime=00:10:00\n\
        #PBS -j oe\n\
        #PBS -q edu_shared\n\
        ##PBS jzeise2@uic.edu\n\
        module load MPICH/3.3.2-GCC-9.3.0\n\
        cd \$PBS_O_WORKDIR\n\
        mpicc MPI_QS_LB.c -o ./build/mpi_qs_lb -lm\n\
        mpirun ./build/mpi_qs_lb 500000 ${s}"

        echo -e ${PBS} | qsub
        sleep 5.0
        echo "done."

        NAME=job_NLB_${s}
        echo "Submitting: ${NAME}"

        PBS="#!/bin/bash\n\
        #PBS -N ${NAME}\n\
        #PBS -l nodes=8:ppn=2\n\
        #PBS -l walltime=00:10:00\n\
        #PBS -j oe\n\
        #PBS -q edu_shared\n\
        ##PBS jzeise2@uic.edu\n\
        module load MPICH/3.3.2-GCC-9.3.0\n\
        cd \$PBS_O_WORKDIR\n\
        mpicc MPI_QS.c -o ./build/mpi_qs -lm\n\
        mpirun ./build/mpi_qs 500000 ${s}"

        echo -e ${PBS} | qsub
        echo "done."

done