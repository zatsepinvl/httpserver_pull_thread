#include<stdio.h>
#include<http_client.h>
#include <pthread.h>
void* start(void* arg)
{
    http_client_start();
}

int main(int Count, char *Strings[])
{
   http_client_start();
}
