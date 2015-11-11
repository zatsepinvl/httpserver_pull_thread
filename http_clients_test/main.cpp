#include<stdio.h>
#include<http_client.h>
#include <pthread.h>
void* start(void* arg)
{
    http_client_start();
}

int main(int Count, char *Strings[])
{
    pthread_t threads[10];
    int n=10;
    for(int i=0; i<n; i++)
    {
        pthread_create(&threads[i],NULL,start,NULL);
    }
    for(int i=0; i<n; i++)
    {
        pthread_join(threads[i],NULL);
    }
}
