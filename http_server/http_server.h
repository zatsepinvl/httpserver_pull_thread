#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#endif // HTTP_SERVER_H
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>


int cd=0;
int ld=0;

pthread_mutex_t queue_mutex;
pthread_mutex_t thread_mutex;
pthread_t *threadPool;
int thread_pool_size=5;

int* cqueue;
int client_queue_size=100;
int pointer=-1;

static void handler(int signo) {
    close(cd);
    close(ld);
    pthread_mutex_destroy(&queue_mutex);
}

int getclient()
{
    int client=-1;
    pthread_mutex_lock(&queue_mutex);
    if(pointer>=0)
    {
        client = cqueue[pointer--];
    }
    pthread_mutex_unlock(&queue_mutex);
    return client;
}

int setclient(int cd)
{
    pthread_mutex_lock(&queue_mutex);
    if(pointer<client_queue_size-1)
    {
        cqueue[++pointer]=cd;
    }
    pthread_mutex_unlock(&queue_mutex);
}


void headers(int client, int size, int httpcode, char* contentType) {
    char buf[1024];
    char strsize[20];
    sprintf(strsize, "%d", size);
    if (httpcode == 200) {
        strcpy(buf, "HTTP/1.0 200 OK\r\n");
    }
    else if (httpcode == 404) {
        strcpy(buf, "HTTP/1.0 404 Not Found\r\n");

    }
    else {
        strcpy(buf, "HTTP/1.0 500 Internal Server Error\r\n");
    }
    send(client, buf, strlen(buf), 0);

    strcpy(buf, "Connection: keep-alive\r\n");
    send(client, buf, strlen(buf), 0);

    strcpy(buf, "Content-length: ");
    send(client, buf, strlen(buf), 0);

    strcpy(buf, strsize);
    send(client, buf, strlen(buf), 0);

    strcpy(buf, "\r\n");
    send(client, buf, strlen(buf), 0);

    strcpy(buf, "simple-server");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, "\r\n");
    send(client, buf, strlen(buf), 0);

    sprintf(buf, "Content-Type:");
    send(client, buf, strlen(buf), 0);
    strcpy(buf, contentType);
    send(client, buf, strlen(buf), 0);
    strcpy(buf, "\r\n");
    send(client, buf, strlen(buf), 0);

    strcpy(buf, "\r\n");
    send(client, buf, strlen(buf), 0);
}

void parseFileName(char *line, char **filepath, size_t *len) {
    char *start = NULL;
    while ((*line) != '/') line++;
    start = line + 1;
    while ((*line) != ' ') line++;
    (*len) = line - start;
    *filepath = (char*)malloc(*len + 1);
    *filepath = strncpy(*filepath, start, *len);
    (*filepath)[*len] = '\0';
    printf("%s \n", *filepath);
}

void getContentType(char *filePath, char** contentType) {
    char subbuff[20]="html";
    int n = strlen(filePath);
    int i=0;
    while ((i<n)&&(filePath[i]!='.'))
    {
        i++;
    }
    if(filePath[i]=='.')
    {
        i++;
        memcpy(subbuff, &filePath[i], n-i);
        subbuff[n-i] = '\0';
        if (strstr(subbuff,"png")!=NULL)
        {
            *contentType="image/png";
            return;
        }
        else if (strstr(subbuff,"jpg")!=NULL)
        {
            *contentType="image/jpg";
            return;
        }
        else if (strstr(subbuff,"pdf")!=NULL)
        {
            *contentType="application/pdf";
            return;
        }

    }
    *contentType="text/html";
}
int handle_client(int cd)
{
    char *line = NULL;
    char *filepath = NULL;
    int res = 0;
    int empty_str_count = 0;
    int filesize = 0;
    size_t len = 0;
    size_t filepath_len = 0;
    FILE *fd;
    FILE *file;

    if (cd == -1) {
        printf("accept error \n");
    }
    fd = fdopen(cd, "r");
    if (fd == NULL) {
        printf("error open client descriptor as file \n");
    }
    while ((res = getline(&line, &len, fd)) != -1) {
        if (strstr(line, "GET")) {
            parseFileName(line, &filepath, &filepath_len);
        }
        if (strcmp(line, "\r\n") == 0) {
            empty_str_count++;
        }
        else {
            empty_str_count = 0;
        }
        if (empty_str_count == 1) {
            break;
        }
        printf("%s", line);
    }
    printf("open %s \n", filepath);
    file = fopen(filepath, "r");
    if (file == NULL) {
        printf("404 File Not Found \n");
        headers(cd, 0, 404, "text/html");
    }
    else {
        FILE *f  = fopen(filepath, "rb");
        fseek(f, 0L, SEEK_END);
        filesize = ftell(f);
        char* content_type;
        getContentType(filepath, &content_type);
        headers(cd, filesize, 200, content_type);
        fseek(f, 0L, SEEK_SET);
        char buffer[1024];
        int result;
        while (( result = fread( buffer, 1, 1024, f )) > 0 ) {
            send( cd, buffer, result, 0 );
        }
        fclose(f);
    }
    close(cd);
}
void *thread_work(void *arg)
{
    while(true)
    {
        pthread_mutex_lock(&thread_mutex);
        int cd = getclient();
        if(cd!=-1)
        {
            printf("\nTHREAD %d HANDLE CLIENT %d\n", *((int *)arg), cd);
            handle_client(cd);
        }
    }
}

int startServer()
{
    printf("server started\n");
    signal(SIGKILL,handler);
    pthread_mutex_init(&queue_mutex,NULL);
    pthread_mutex_init(&thread_mutex,NULL);
    pthread_mutex_lock(&thread_mutex);
    cqueue = new int[client_queue_size];
    for(int i=0; i<client_queue_size; i++)
    {
        cqueue[i]=0;
    }
    threadPool=new pthread_t[thread_pool_size];
    //for(int i=0; i<thread_pool_size; i++)
    {
        //int j=i;
        int i1=0;
        pthread_create(&threadPool[0], NULL, thread_work, &i1);
        int i2=1;
        pthread_create(&threadPool[1], NULL, thread_work, &i2);
        int i3=2;
        pthread_create(&threadPool[2], NULL, thread_work, &i3);
        int i4=3;
        pthread_create(&threadPool[3], NULL, thread_work, &i4);
        int i5=4;
        pthread_create(&threadPool[4], NULL, thread_work, &i5);
    }
    const int backlog = 10;
    struct sockaddr_in saddr;
    struct sockaddr_in caddr;
    socklen_t size_saddr;
    socklen_t size_caddr;
    int res=0;
    ld = socket(AF_INET, SOCK_STREAM, 0);
    if (ld == -1) {
        printf("listener create error \n");
    }

    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(8080);
    saddr.sin_addr.s_addr = INADDR_ANY;
    res = bind(ld, (struct sockaddr *)&saddr, sizeof(saddr));
    if (res == -1) {
        printf("bind error \n");
        return 0;
    }
    res = listen(ld, backlog);
    if (res == -1) {
        printf("listen error \n");
    }
    else
    {
        while (1) {
            cd = accept(ld, (struct sockaddr *)&caddr, &size_caddr);
            setclient(cd);
            pthread_mutex_unlock(&thread_mutex);
        }
    }
    close(ld);
    return 0;
}

