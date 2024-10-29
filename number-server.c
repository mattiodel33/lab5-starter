#include "http-server.h"
#include <string.h>

char const HTTP_404_NOT_FOUND[] = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\n";

void handle_404(int client_sock, char *path)  {
    printf("SERVER LOG: Got request for unrecognized path \"%s\"\n", path);

    char response_buff[BUFFER_SIZE];
    snprintf(response_buff, BUFFER_SIZE, "Error 404:\r\nUnrecognized path \"%s\"\r\n", path);
    // snprintf includes a null-terminator

    // TODO: send response back to client?
    write(client_sock, "HTTP/1.1 404 you sussy baka\r\nContent-Type: text/plain\r\n\r\n", 54);
}
void handle200(int client_sock, char *path){
	printf("SERVER LOG: Got request for recognized path \"%s\"\n", path);

	char response_buff[BUFFER_SIZE];
	snprintf(response_buff, BUFFER_SIZE, "200:\r\nRecognized path \"%s\"\r\n", path);

	write(client_sock, "HTTP/1.1 200 WOOHOO\r\nContent-Type: text/plain\r\n\r\n", 46);
}

void handle_response(char *request, int client_sock) {
    char path[256];

    printf("\nSERVER LOG: Got request: \"%s\"\n", request);

    // Parse the path out of the request line (limit buffer size; sscanf null-terminates)
    if (sscanf(request, "GET %255s", path) != 1) {
        printf("Invalid request line\n");
        return;
    }

    if (strstr(path, "/shownum") != path[strlen(path)]){
	    handle200(client_sock, path);
	    write(client_sock, "Your number is ", 15);
	    write(client_sock, num, 4);
    }
    else if (strstr(path, "/increment") != path[strlen(path)]){
	    handle200(client_sock, path);
    }
    else if (strstr(path, "/add?value=") < path[strlen(path)]){
	    handle200(client_sock, path);
    }

    handle_404(client_sock, path);
}

int main(int argc, char *argv[]) {
    int port = 0;
    if(argc >= 2) { // if called with a port number, use that
        port = atoi(argv[1]);
    }

    start_server(&handle_response, port);
}
