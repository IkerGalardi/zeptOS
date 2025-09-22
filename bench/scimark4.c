#include "kernel/types.h"
#include "user/user.h"

#include "Random.h"
#include "kernel.h"
#include "constants.h"

void print_banner(void);


unsigned int ilog2( unsigned int N)
{
    unsigned int exp=0;
    while ( (N = (N >> 1)) )
        exp++;

    return exp;
}


int main(int argc, char *argv[])
{
        /* default to the (small) cache-contained version */

     double min_time = RESOLUTION_DEFAULT;

     int FFT_size = FFT_SIZE;
     int SOR_size =  SOR_SIZE;
     int Sparse_size_M = SPARSE_SIZE_M;
     int Sparse_size_nz = SPARSE_SIZE_nz;
     int LU_size = LU_SIZE;

     int huge_flag = 0;



     /* run the benchmark */

     double res[6] = {0.0};
     double sum[6] = {0.0};   /* checksum */
     unsigned long num_cycles[6] = {0.0};
     Random R = new_Random_seed(RANDOM_SEED);


 print_banner();
 printf("Using %f seconds min time per kenel.", min_time);
 if (huge_flag)
 {
      printf(" Approx. problem size: %d (MB)", atoi(argv[2]));
 }
 printf("\n\n");

 /* print out results  */

 kernel_measureFFT( FFT_size, min_time, R, &res[1], &sum[1], &num_cycles[1]);
 printf("FFT             Mflops: %f    (N=%d) \n", res[1],
            FFT_size);

 kernel_measureSOR( SOR_size, min_time, R, &res[2], &sum[2], &num_cycles[2]);
 printf("SOR             Mflops: %f    (%d x %d) \n", res[2],
            SOR_size, SOR_size);

 kernel_measureMonteCarlo(min_time, R, &res[3], &sum[3], &num_cycles[3]);
 printf("MonteCarlo:     Mflops: %f  \n", res[3] );

 kernel_measureSparseMatMult( Sparse_size_M,
          Sparse_size_nz, min_time, R, &res[4], &sum[4], &num_cycles[4]);
  printf("Sparse matmult  Mflops: %f    (N=%d, nz=%d)  \n",
          res[4], Sparse_size_M, Sparse_size_nz);


 kernel_measureLU( LU_size, min_time, R, &res[5], &sum[5], &num_cycles[5]);
 printf("LU              Mflops: %f    (M=%d, N=%d) \n", res[5],
     LU_size, LU_size);



     res[0] = (res[1] + res[2] + res[3] + res[4] + res[5]) / 5;
     sum[0] = (sum[1] + sum[2] + sum[3] + sum[4] + sum[5]) / 5;


     printf("\n");
     printf("************************************\n");
     printf("Composite Score:       %f\n" ,res[0]);
     printf("************************************\n");
     printf("\n");

     printf("FFT reps:              %ld\n", num_cycles[1]);
     printf("SOR reps:              %ld\n", num_cycles[2]);
     printf("Montel Carlo reps:     %ld\n", num_cycles[3]);
     printf("Sparse MatMult repss:  %ld\n", num_cycles[4]);
     printf("LU reps:               %ld\n", num_cycles[5]);
     printf("\n");

     Random_delete(R);

     return 0;

}

void print_banner()
{
 printf("**                                                              **\n");
 printf("** SciMark4 Numeric Benchmark, see http://math.nist.gov/scimark **\n");
 printf("** for details. (Results can be submitted to pozo@nist.gov)     **\n");
 printf("**                                                              **\n");
}
