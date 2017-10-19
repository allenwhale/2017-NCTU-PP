#include <bits/stdc++.h>
#include <pthread.h>
using namespace std;
typedef unsigned long long ll;
//#define RAND_R
#define XORSHIFT

#if defined(RAND_R)
const ll threshold = (ll)RAND_MAX * (ll)RAND_MAX;
#elif defined(XORSHIFT)
const ll threshold = (ll)UINT_MAX * (ll)UINT_MAX;
uint32_t xorshift128(uint32_t state[4]) {
    uint32_t t = state[3];
    t ^= t << 11;
    t ^= t >> 8;
    state[3] = state[2]; state[2] = state[1]; state[1] = state[0];
    t ^= state[0];
    t ^= state[0] >> 19;    
    state[0] = t;
    return t;
}
#endif

struct thread_arg_t{
    ll samples, in_circle;
#if defined(RAND_R)
    unsigned seed;
#elif defined(XORSHIFT)
    unsigned seed[4];
#endif
};

void* toss(void *arg){
    thread_arg_t *thread_arg = (thread_arg_t*)arg;
#ifdef RAND_R
    unsigned seed = thread_arg->seed;
#elif defined(XORSHIFT)
    unsigned seed[4];
    memcpy(seed, thread_arg->seed, sizeof(seed));
#endif
    ll samples = thread_arg->samples;
    ll in_circle = 0;
    for(ll i = samples ; --i ;){
#ifdef RAND_R
        ll x = rand_r(&seed);
        ll y = rand_r(&seed);
#elif defined(XORSHIFT)
        ll x = xorshift128(seed);
        ll y = xorshift128(seed);
#endif
        if(x * x < threshold - y * y)
            in_circle++;
    }
    thread_arg->in_circle = in_circle;
    return NULL;
}

int main(int argc, char **argv){
    if(argc != 3)
        return 0;
    srand(time(0));
    const int processors= stoi(argv[1]);
    const ll samples = stoull(argv[2]);
    ll remain_samples = samples; 
    thread_arg_t *thread_args = new thread_arg_t[processors];
    pthread_t *threads = new pthread_t[processors];
    ll batch = (samples + 1) / (ll)processors;
    for(int i = processors ; i-- ;){
        thread_args[i].samples = min(remain_samples, batch);
#ifdef RAND_R
        thread_args[i].seed = rand();
#elif defined(XORSHIFT)
        for(int j = 4 ; j-- ;)
            thread_args[i].seed[j] = rand();
#endif
        remain_samples -= min(remain_samples, batch);
        pthread_create(&threads[i], NULL, &toss, &thread_args[i]);
    }
    ll sum_in_circle = 0;
    for(int i = processors ; i-- ;){
        pthread_join(threads[i], NULL);
        sum_in_circle += thread_args[i].in_circle;
    }
    printf("%.15f\n", 4.0 * (double)sum_in_circle / (double)samples);
    delete [] thread_args;
    delete [] threads;
    return 0;
}
