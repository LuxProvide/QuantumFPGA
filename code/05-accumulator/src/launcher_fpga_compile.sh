#!/bin/bash -l
#SBATCH --nodes=1                          # number of nodes
#SBATCH --ntasks=1                         # number of tasks
#SBATCH --cpus-per-task=1                  # number of cores per task
#SBATCH --time=40:00:00                    # time (HH:MM:SS)
#SBATCH --account=p200117                  # project account
#SBATCH --partition=fpga                   # partition
#SBATCH --qos=default                         # QOS
#SBATCH --mail-user=emmanuel.kieffer@uni.lu
#SBATCH --mail-type=BEGIN,FAIL,END


# Uncomment the following lines
# module use $PROJET/apps/u100057/easybuild/modules/all

#module load CMake/3.23.1-GCCcore-11.3.0
module load 520nmx/20.4
module load intel-compilers/2023.2.1-fpga_compile

echo $QUARTUS_ROOTDIR

icpx -v -fsycl -fintelfpga  -Xshardware -Xsboard=p520_hpc_m210h_g3x16 -D FPGA_HARDWARE $1 -o ${1//cpp/fpga}


