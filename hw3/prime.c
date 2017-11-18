#pragma GCC optimize ("O3")
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "mpi.h"
typedef long long ll;
#define min(a, b) ((a) < (b) ? (a) : (b))

#define SIEVE_SIZE 100000000
ll primes[10000000];
char prime[SIEVE_SIZE + 10];
int prime_cnt = 0;


int isprime_sieve(ll n){
    if(n == 2)return 1;
    if(n < 2) return 0;
    if(n < SIEVE_SIZE)
        return prime[n] == 0;
    ll sqroot = sqrt(n);
    int i;
    for(i = 0 ; i < prime_cnt && primes[i] <= sqroot ; i++){
        if(n % primes[i] == 0)return 0;
    }
    if(i < prime_cnt)return 1;
    ll last_test = primes[i];
    int gap = last_test % 6 == 5 ? 2 : 4;
    while(last_test <= sqroot){
        if(n % last_test == 0)return 0;
        last_test += gap;
        gap = 6 - gap;
    }
    return 1;
}

int foundones[] = {0, 0, 2, 3, 3};
int pcs[] = {0, 0, 1, 2, 2};
int start[] = {5, 7, 11, 13};
int gap[][2] = {
    {0, 0},
    {2, 4},
    {6, 6},
    {8, 10},
    {12, 12}
};
ll limit;
void gen_prime(){
    memset(prime, 0, sizeof(prime));
    //primes[prime_cnt++] = 2;
    const int sieve_limit = min(SIEVE_SIZE, limit);
    for(int i = 2 ; i <= sieve_limit ; i++){
        if(prime[i] == 0)primes[prime_cnt++] = i;
        for(int j = 0 ; i * primes[j] <= sieve_limit ; j++){
            prime[i * primes[j]] = 1;
            if(i % primes[j] == 0)break;
        }
    }
}
int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);
    sscanf(argv[1], "%lld", &limit);	
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    if (rank == 0)
        printf("Starting. Numbers to be scanned= %lld\n", limit);
    ll local_foundone, foundone;
    ll local_pc, pc;
    if(limit <= SIEVE_SIZE){
        if(rank == 0)gen_prime();
        if(limit < 2){
            foundone = 0;
            pc = 0;
        }else{
            pc = prime_cnt;
            foundone = primes[prime_cnt - 1];
        }

    }else{
        gen_prime();
        local_pc = 0;
        for (ll n = start[rank], idx = rank & 1 ; n <= limit ; n += gap[size][idx], idx ^= 1) {
            if (isprime_sieve(n)) {
                local_pc++;
                local_foundone = n;
            }			
        }
        MPI_Reduce(&local_pc, &pc, 1, MPI_LONG_LONG_INT, MPI_SUM, 0, MPI_COMM_WORLD);
        MPI_Reduce(&local_foundone, &foundone, 1, MPI_LONG_LONG_INT, MPI_MAX, 0, MPI_COMM_WORLD);
        if(limit < 5){
            pc = pcs[limit];
            foundone =  foundones[limit];
        }else{
            pc += 2;
        }
    }
    if (rank == 0)
        printf("Done. Largest prime is %lld Total primes %lld\n", foundone, pc);
    MPI_Finalize();
    return 0;
} 
