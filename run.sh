N=(10000 50000 100000 200000 400000 800000 1600000)
P=(16 32 64)

for n in ${N[@]}
do
        for p_val in ${P[@]}
        do
                seed=$((p_val + n))
                NAME=job_LB_${n}_${p_val}
                echo "Submitting: ${NAME}"

                if [ ${p_val} -ge 8 ]; then
                    echo "here"
                    ppn=$((p_val / 8))
                    n_val=8
                else
                    n_val=${p_val}
                    ppn=1
                fi

                echo ${ppn}
                echo ${n_val}

                PBS="#!/bin/bash\n\
                #PBS -N ${NAME}\n\
                #PBS -l nodes=${n_val}:ppn=${ppn}\n\
                #PBS -l walltime=02:00:00\n\
                #PBS -j oe\n\
                #PBS -q edu_shared\n\
                ##PBS jzeise2@uic.edu\n\
                module load MPICH/3.3.2-GCC-9.3.0\n\
                cd \$PBS_O_WORKDIR\n\
                mpicc MPI_QS_LB.c -o ./build/mpi_qs_lb -lm\n\
                mpirun ./build/mpi_qs_lb ${n} ${seed}"

                echo -e ${PBS} | qsub
                sleep 5.0
                echo "done."

                NAME=job_NLB_${n}_${p_val}
                echo "Submitting: ${NAME}"

                PBS="#!/bin/bash\n\
                #PBS -N ${NAME}\n\
                #PBS -l nodes=${n_val}:ppn=${ppn}\n\
                #PBS -l walltime=02:30:00\n\
                #PBS -j oe\n\
                #PBS -q edu_shared\n\
                ##PBS jzeise2@uic.edu\n\
                module load MPICH/3.3.2-GCC-9.3.0\n\
                cd \$PBS_O_WORKDIR\n\
                mpicc MPI_QS.c -o ./build/mpi_qs -lm\n\
                mpirun ./build/mpi_qs ${n} ${seed}"

                echo -e ${PBS} | qsub
                echo "done."
        done
done