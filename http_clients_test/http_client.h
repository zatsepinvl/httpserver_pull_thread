#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#endif // HTTP_CLIENT_H
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <resolv.h>
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAXBUF  1024

char* server_address="127.0.0.1";
char* request="/test.pdf";
int port=8080;

int http_client_start()
{
    printf("_CLIENT_START_\n");
    int sock, bytes_read;
    struct sockaddr_in dest;
    char buffer[MAXBUF];
    /*---Initialize server address/port struct---*/
    bzero(&dest, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(port);
    inet_aton(server_address, &dest.sin_addr);

    /*---Connect to server---*/
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if( connect(sock, (struct sockaddr*)&dest, sizeof(dest)) != 0 )
    {
         printf("error\n");
         return 0;
    }

    sprintf(buffer, "GET %s HTTP/1.0\r\n\r\n", request);
    send(sock, buffer, strlen(buffer), 0);
    bool ok=false;
    do
    {
        bzero(buffer, sizeof(buffer));
        bytes_read = recv(sock, buffer, sizeof(buffer), 0);
        if ( bytes_read > 0 )
        {
            //printf("%s", buffer);
            ok=true;
        }
    }
    while ( bytes_read > 0 );
    if(ok)
    {
        printf("__DONE__\n\n");
    }
    else
    {
        printf("__ERROR__\n");
    }
    /*---Clean up---*/
    close(sock);
    return 0;
}
