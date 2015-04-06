#include <cstdlib>
#include <pthread.h>
#include <iostream>
#include <time.h>
using namespace std;

void merge (int*, int, int, int);
void mergeSort_mt(int *, int, int, int);
void* mergeSort_thread (void *);

struct Params{
    int *arr;
    int low;
    int hi;
    int depth;
};

pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

void  merge(int* a, int start, int mid, int end){
    int i = start;
    int j = mid+1;
    int k = 0;
    int* kArr = (int*) malloc( (end-start +1) * sizeof(*kArr));

    while (i <= mid && j <= end){
        if (a[i] < a[j]){
            kArr[k] = a[i];
            i++;
        }else{
            kArr[k] = a[j];
            j++;
        }
        k++;
    }
    while (i <= mid){
        kArr[k] = a[i];
        k++;i++;
    }
    while(j <= end){
        kArr[k] = a[j];
        k++;j++;
    }
    k--;
    while (k >=0){
        a[k+start] = kArr[k];
        k--;
    }

    free(kArr);

}

void  mergeSort (int *arr, int low, int hi){
    if (low < hi){
        int mid = (low + hi )/2;
        mergeSort(arr,low,mid);
        mergeSort(arr,mid+1,hi);
        merge(arr,low,mid,hi);
        return;
    }
}

void mergeSort_mt (int *arr, int low, int hi, int depth){
    if (low >= hi){
        return;
    }
    int mid = (hi + low) /2;
    pthread_mutex_lock(&mtx);
    pthread_mutex_unlock(&mtx);
    if (depth <= 0 || (hi-low) < 4){
        //call function recursive, no need for new thread
        mergeSort_mt(arr,low, mid, 0);
        mergeSort_mt(arr,mid+1,hi,0);
    }else{
        struct Params params = { arr, low, mid, depth/2 };
        pthread_t thread;

        pthread_mutex_lock(&mtx);
        printf("Starting subthread..%d \n",depth);
        pthread_mutex_unlock(&mtx);

        pthread_create(&thread,NULL,mergeSort_thread, &params);
        mergeSort_mt (arr,mid+1,hi,depth/2);

        pthread_join(thread,NULL);

        pthread_mutex_lock(&mtx);
        printf("Finished subthread \n");
        pthread_mutex_unlock(&mtx);

    }
    //merge partitions
    merge(arr,low,mid,hi);
}

void* mergeSort_thread (void* params){
    
    struct Params* p = (struct Params*) params;
    mergeSort_mt (p->arr,p->low,p->hi,p->depth);
    return params;
}

void mergeSort_pub (int * arr, int start, int end){
    mergeSort_mt(arr,start,end,4); //2N -1 threads are created
}


int main(){
    static const unsigned int N = 2048;
    int *data = (int*)malloc (N * sizeof(*data));
    unsigned int i;
    srand((unsigned(time(0))));

    for (i = 0; i < N; ++i){
        data[i] = rand() % 1024;
    }

       mergeSort_pub(data,0,N-1);

    for (int i = 0; i < N;i++){
        printf("%4d ",data[i]);
        if ((i+1) % 8 == 0){
            printf("\n");
        }
    
    }
    printf("\n");

    free(data);
    return 0;
}

