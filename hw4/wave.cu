/**********************************************************************
 * DESCRIPTION:
 *   Serial Concurrent Wave Equation - C Version
 *   This program implements the concurrent wave equation
 *********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define MAXPOINTS 1000000
#define MAXSTEPS 1000000
#define MINPOINTS 20
#define PI 3.14159265

void check_param(void);
void init_line(void);
void update (void);
void printfinal (void);

int nsteps,                 	/* number of time steps */
    tpoints, 	     		/* total points along string */
    rcode;                  	/* generic return code */
float  values[MAXPOINTS+2], 	/* values at time t */
       oldval[MAXPOINTS+2], 	/* values at time (t-dt) */
       newval[MAXPOINTS+2]; 	/* values at time (t+dt) */


/**********************************************************************
 *	Checks input values from parameters
 *********************************************************************/
void check_param(void)
{
    char tchar[20];

    /* check number of points, number of iterations */
    while ((tpoints < MINPOINTS) || (tpoints > MAXPOINTS)) {
        printf("Enter number of points along vibrating string [%d-%d]: "
                ,MINPOINTS, MAXPOINTS);
        scanf("%s", tchar);
        tpoints = atoi(tchar);
        if ((tpoints < MINPOINTS) || (tpoints > MAXPOINTS))
            printf("Invalid. Please enter value between %d and %d\n", 
                    MINPOINTS, MAXPOINTS);
    }
    while ((nsteps < 1) || (nsteps > MAXSTEPS)) {
        printf("Enter number of time steps [1-%d]: ", MAXSTEPS);
        scanf("%s", tchar);
        nsteps = atoi(tchar);
        if ((nsteps < 1) || (nsteps > MAXSTEPS))
            printf("Invalid. Please enter value between 1 and %d\n", MAXSTEPS);
    }

    printf("Using points = %d, steps = %d\n", tpoints, nsteps);

}

/**********************************************************************
 *     Update all values along line a specified number of times
 *********************************************************************/
__device__ inline unsigned getIdx(){
    return blockIdx.x * blockDim.x + threadIdx.x;
}
__global__ void update(float *values, int nsteps, int tpoints)
{
    float local_value, local_old_value, local_new_value;
    int idx = getIdx();
    local_old_value = local_value = sin((2.0 * PI) * ((float)idx / (float)(tpoints - 1)));
    for (int i = 0 ; i < nsteps; i++) {
        local_new_value = (1.82 * local_value) - local_old_value;
        local_old_value = local_value;
        local_value = local_new_value;
    }
    if(0 < idx && idx < tpoints - 1){
        values[idx] = local_value;
    }else if(idx == 0 || idx == tpoints - 1){
        values[idx] = 0;
    }
}

/**********************************************************************
 *     Print final results
 *********************************************************************/
void printfinal()
{
    int i;

    for (i = 0; i < tpoints; i++) {
        printf("%6.4f ", values[i]);
        if (i % 10 == 9)
            printf("\n");
    }
}

/**********************************************************************
 *	Main program
 *********************************************************************/
int main(int argc, char *argv[])
{
    sscanf(argv[1],"%d",&tpoints);
    sscanf(argv[2],"%d",&nsteps);
    check_param();
    printf("Initializing points on the line...\n");
    printf("Updating all points for all time steps...\n");
    float *d_values;
    cudaMalloc(&d_values, sizeof(values));
    update<<<((tpoints + 1023) >> 10), 1024>>>(d_values, nsteps, tpoints);
    cudaMemcpy(values, d_values, sizeof(values), cudaMemcpyDeviceToHost);
    printf("Printing final results...\n");
    printfinal();
    cudaFree(d_values);
    printf("\nDone.\n\n");

    return 0;
}
