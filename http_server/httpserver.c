#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

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
            //subbuff[n-i] = '\0';
       	    if (strstr(subbuff,"png")!=NULL)
       	 	{
                  char s[20] = "image/";
                  strcat(s, subbuff);
                  *contentType=s;//"image.png";
		  return;
       		 }
            else if (strstr(subbuff,"jpg")!=NULL)
       		 {
                   char s[20] = "image/";
                   strcat(s, subbuff);
                   *contentType=s;
		   return;
       	 	}
	    else if (strstr(subbuff,"pdf")!=NULL)
       		 {
                   char s[20] = "application/";
                   strcat(s, subbuff);
                   *contentType=s;
		   return;
       	 	}

        }
    	*contentType="text/html";
}

int main() {
	int ld = 0;
	int res = 0;
	int cd = 0;
	int filesize = 0;
	const int backlog = 10;
	struct sockaddr_in saddr;
	struct sockaddr_in caddr;
	char *line = NULL;
	size_t len = 0;
	char *filepath = NULL;
	size_t filepath_len = 0;
	int empty_str_count = 0;
	socklen_t size_saddr;
	socklen_t size_caddr;
	FILE *fd;
	FILE *file;

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
	}
	res = listen(ld, backlog);
	if (res == -1) {
		printf("listen error \n");
	}
	while (1) {
		cd = accept(ld, (struct sockaddr *)&caddr, &size_caddr);
		if (cd == -1) {
			printf("accept error \n");
		}
		printf("client in %d descriptor. Client addr is %d \n", cd, caddr.sin_addr.s_addr);
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
			printf("_server_Content type: %s\n",content_type);
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
	return 0;
}
