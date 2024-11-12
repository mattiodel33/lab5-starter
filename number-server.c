#include "http-server.h"
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdint.h>

char const HTTP_404_NOT_FOUND[] = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\n";
char const HTTP_200_OK[] = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n";
char const HTTP_BAD_RESPONSE[] = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\n\r\n";
uint32_t currId = 0;

int hex(char c1, char c2){
	int ret = 0;
	if (c1 >= 'A' && c1 <= 'F'){
		ret += (c1 - 'A' + 10)*16;
	}
	else if (c1 >= 'a' && c1 <= 'f'){
		ret += (c1 - 'a' + 10)*16;
	}
	else{
		ret += (c1 - '0')*16;
	}
	if (c2 >= 'A' && c2 <= 'F'){
		ret += (c2 - 'A' + 10);
	}
	else if (c2 >= 'a' && c2 <= 'f'){
		ret += (c2 - 'a' + 10);
	}
	else{
		ret += (c2 - '0')*16;
	}
	return ret;
}

struct Reaction{
	char user[64];
	char message[32];
};
struct Chat{
	uint32_t id;
	char user[64];
	char message[256];
	char timestamp[100];
	uint8_t num_reactions;
	struct Reaction* reactions[20];
};

struct Chat* messages[1000];

void handle_404(int client_sock, char *path)  {
    printf("SERVER LOG: Got request for unrecognized path \"%s\"\n", path);

    char response_buff[BUFFER_SIZE];
    snprintf(response_buff, BUFFER_SIZE, "Error 404:\r\nUnrecognized path \"%s\"\r\n", path);
    // snprintf includes a null-terminator

    // TODO: send response back to client?
    write(client_sock, HTTP_404_NOT_FOUND, strlen(HTTP_404_NOT_FOUND);
    write(client_sock, response_buff, strlen(response_buff));
}
void handle400(int client_sock, char *path){
	printf("SERVER LOG: Got bad request \"%s\"\n", path);

	char response_buff[BUFFER_SIZE];
	snprintf(response_buff, BUFFER_SIZE, "400:\r\nBad request \"%s\"\r\n", path);

	write(client_sock, HTTP_BAD_RESPONSE, strlen(HTTP_BAD_RESPONSE));
	write(client_sock, response_buff, strlen(response_buff));
}
void handle200(int client_sock, char *path){
	printf("SERVER LOG: Got request for recognized path \"%s\"\n", path);

	char response_buff[BUFFER_SIZE];
	snprintf(response_buff, BUFFER_SIZE, "200:\r\nRecognized path \"%s\"\r\n", path);

	write(client_sock, HTTP_200_OK, strlen(HTTP_200_OK));
	write(client_sock, response_buff, strlen(response_buff));
}

void handle_response(char *request, int client_sock) {
    char path[256];

    printf("\nSERVER LOG: Got request: \"%s\"\n", request);

    // Parse the path out of the request line (limit buffer size; sscanf null-terminates)
    if (sscanf(request, "GET %255s", path) != 1) {
        printf("Invalid request line\n");
	char response[] = "Invalid request line \n";
	write(client_sock, response, strlen(response));
        return;
    }

    if (strstr(path, "/post") != NULL){

	    handle200(client_sock, path);
	    return;
    }
    else if (strstr(path, "/react") != NULL){
	    handle200(client_sock, path);
	    return;
    }
    else if (strstr(path, "/chats") != NULL){
	    if(
	    handle200(client_sock, path);
	    return;
    }
    else if (strstr(path, "/reset") != NULL){
	    reset();
	    handle200(client_sock, path);
	    return;
    }

    handle_404(client_sock, path);
}

uint8_t add_chat(char* username, char* message){
	currId++;
	if (currId > 999){
		return 0;
	}

	time_t now = time(NULL);
	struct tm *tm_info = localtime(&now);
	char buffer[100];
	strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm_info);

	if (strlen(username) > 64){
		username[64] = 0;
	}
	if (strlen(message) > 256){
		message[256] = 0;
	}

	struct Chat* chatp = malloc(sizeof(struct Chat)*2);
	(*chatp).id = currId;
	memcpy((*chatp).user, username, strlen(username));
	memcpy((*chatp).message, message, strlen(message));
	memcpy((*chatp).timestamp, buffer, strlen(buffer));
	(*chatp).num_reactions = 0;

	messages[currId] = chatp;

	return 1;
}

uint8_t add_reaction(char* username, char* message, char* id){
	int num = atoi(id);
	if (num > currId){
		return 0;
	}
	struct Chat* chat = messages[num];
	if ((*chat).num_reactions > 19){
		return 0;
	}
	
	if (strlen(username) > 64){
		username[64] = 0;
	}
	if (strlen(message) > 32){
		message[32] = 0;
	}

	struct Reaction* reaction = malloc(sizeof(struct Reaction)*2);
	memcpy((*reaction).user, username, strlen(username));
	memcpy((*reaction).message, message, strlen(message));

	(*chat).reactions[(*chat).num_reactions] = reaction;
	(*chat).num_reactions++;
	
	return 1;
}

void reset(){
	int i = 1;
	for (; i <= currId; i++){
		struct Chat* chat = messages[i];
		int j = 0;
		for (; j < (*chat).num_reactions; j++){
			struct Reaction* reaction = (*chat).reactions[j];
			free(reaction);
		}
		
		free(chat);
	}
}

int main(int argc, char *argv[]) {
	int port = 0;
	if(argc >= 2) { // if called with a port number, use that
		port = atoi(argv[1]);
	}
	if (1){
		char user[] = "bus";
		char msg[] = "sup bozo";
		char num[] = "1";
		add_chat(user, msg);
		struct Chat* chat = messages[1];
	
		add_reaction(msg, user, num);
	
		struct Reaction* rxn = (*chat).reactions[0];
	
		printf("%d %s %s %s %d %s", (*chat).id, (*chat).user, (*chat).message, (*chat).timestamp, (*chat).num_reactions, (*rxn).user);

		int* ptr = malloc(10);

		reset();
		
		return 0;
	}

	start_server(&handle_response, port);
}
