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
		ret += (c2 - '0');
	}
	return ret;
}

struct Reaction{
	char user[16];
	char message[16];
};
struct Chat{
	uint32_t id;
	char user[16];
	char message[256];
	char timestamp[100];
	uint8_t num_reactions;
	struct Reaction* reactions[101];
};

struct Chat* messages[100001];

void handle_404(int client_sock, char *path)  {
    printf("SERVER LOG: Got request for unrecognized path \"%s\"\n", path);

    char response_buff[BUFFER_SIZE];
    snprintf(response_buff, BUFFER_SIZE, "Error 404:\r\nUnrecognized path \"%s\"\r\n", path);
    // snprintf includes a null-terminator

    // TODO: send response back to client?
    write(client_sock, HTTP_404_NOT_FOUND, strlen(HTTP_404_NOT_FOUND));
    write(client_sock, response_buff, strlen(response_buff));
}
void handle400(int client_sock, char *path, char* error){
	printf("SERVER LOG: Got bad request \"%s\"\n", path);

	char response_buff[BUFFER_SIZE];
	snprintf(response_buff, BUFFER_SIZE, "400:\r\nBad request \"%s: %s\"\r\n", path, error);

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

uint8_t add_chat(char* username, char* message){
	currId++;
	if (currId > 100000){
		return 2;
	}

	time_t now = time(NULL);
	struct tm *tm_info = localtime(&now);
	char buffer[100];
	strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm_info);

	if (strlen(username) > 15){
		username[15] = 0;
	}
	if (strlen(message) > 255){
		message[255] = 0;
	}

	struct Chat* chatp = malloc(sizeof(struct Chat)*2);
	(*chatp).id = currId;
	memcpy((*chatp).user, username, strlen(username));
	(*chatp).user[strlen(username)] = 0;
	memcpy((*chatp).message, message, strlen(message));
	(*chatp).message[strlen(message)] = 0;
	memcpy((*chatp).timestamp, buffer, strlen(buffer));
	(*chatp).timestamp[strlen(buffer)] = 0;
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
	if ((*chat).num_reactions > 99){
		return 2;
	}
	
	if (strlen(username) > 15){
		username[15] = 0;
	}
	if (strlen(message) > 15){
		message[15] = 0;
	}

	struct Reaction* reaction = malloc(sizeof(struct Reaction)*2);
	memcpy((*reaction).user, username, strlen(username));
	(*reaction).user[strlen(username)] = 0;
	memcpy((*reaction).message, message, strlen(message));
	(*reaction).message[strlen(message)] = 0;

	(*chat).reactions[(*chat).num_reactions] = reaction;
	(*chat).num_reactions++;
	
	return 1;
}
/*
uint8_t edit(char* id, char* message){
	int num = atoi(id);
	if (num > currId){
		return 0;
	}
	struct Chat* chat = messages[num];
	memcpy((*chat).message, message, strlen(message));
	(*chat).message[strlen(message)] = 0;
	return 1;
}
*/
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
	currId = 0;
}

void handlepath(char* path){
	int i = 0;
	for (; i < strlen(path); i++){
		if (path[i] == '%'){
			path[i] = hex(path[i + 1], path[i + 2]);
			int j = i + 1;
			for (; j < strlen(path) - 2; j++){
				path[j] = path[j + 2];
			}
			path[strlen(path) - 2] = 0;
		}
	}
	return;
}

void respond_with_chats(char* path, int client){
	int i = 1;
	write(client, HTTP_200_OK, strlen(HTTP_200_OK));
	for (; i <= currId; i++){
		struct Chat* chat = messages[i];
		char buff[500];
		snprintf(buff, 475, "[#%d %s]     %s: %s\r\n", (*chat).id, (*chat).timestamp, (*chat).user, (*chat).message);
		write(client, buff, strlen(buff));

		int j = 0;
		for (; j < (*chat).num_reactions; j++){
			struct Reaction* rxn = (*chat).reactions[j];
			char buff2[150];
			snprintf(buff2, 125, "                      (%s)     %s\r\n", (*rxn).user, (*rxn).message);
			write(client, buff2, strlen(buff2));
		}
	}
}

void handle_post(char* path, int client){
	handlepath(path);
	char* start = strstr(path, "?");
	if (start == NULL){
		handle400(client, path, "no ?");
		return;
	}
	start++;

	char* userp = strstr(start, "user=");
	if (userp == NULL){
		handle400(client, path, "missing params");
		return;
	}
	char* msgp = strstr(userp, "message=");
	char* andp = strstr(userp, "&");
	if (msgp == NULL || andp == NULL){
		handle400(client, path, "missing params");
		return;
	}
	if (userp > msgp || andp < userp || andp != msgp - 1){
		handle400(client, path, "bad order");
		return;
	}
	userp += 5;
	msgp += 8;
	char user[16];
	char message[256];

	int i = 0;
	for (; i < andp - userp && i < 16; i++){
		user[i] = userp[i];
	}
	user[i] = 0;
	i = 0;
	for (; i < &path[strlen(path)] - msgp && i < 256; i++){
		message[i] = msgp[i];
	}
	message[i] = 0;
	if (strlen(message) > 255 || strlen(user) > 15){
		handle400(client, path, "too long");
		return;
	}
	uint8_t t = add_chat(user, message);
	if (t == 0){
		handle400(client, path, "none");
		return;
	}
	if (t == 2){
		handle_404(client, path);
		return;
	}
	respond_with_chats(path, client);
	//handle200(client, path);
}

void handle_reaction(char* path, int client){
	handlepath(path);
	char* start = strstr(path, "?");
	if (start == NULL){
		handle400(client, path, "no ?");
		return;
	}
	start++;

	char* userp = strstr(start, "user=");
	if (userp == NULL){
		handle400(client, path, "missing params");
		return;
	}
	char* and1 = strstr(userp, "&");
	if (and1 == NULL){
		handle400(client, path, "missing params");
		return;
	}
	char* msgp = strstr(and1, "message=");
	if (msgp == NULL){
		handle400(client, path, "missing params");
		return;
	}
	char* and2 = strstr(msgp, "&");
	if (and2 == NULL){
		handle400(client, path, "missing params");
		return;
	}
	char* idp = strstr(and2, "id=");

	if (idp == NULL || and1 != msgp - 1 || and2 != idp - 1){
		handle400(client, path, "missing params");
		return;
	}
	userp += 5;
	msgp += 8;
	idp += 3;
	char user[16];
	char message[16];
	char id[10];
	
	int i = 0;
	for (; i < and1 - userp && i < 16; i++){
		user[i] = userp[i];
	}
	user[i] = 0;
	i = 0;
	for (; i < and2 - msgp && i < 16; i++){
		message[i] = msgp[i];
	}
	message[i] = 0;
	i = 0;
	for (; i < &path[strlen(path)] - idp && i < 10; i++){
		id[i] = idp[i];
	}
	id[i] = 0;
	if (strlen(message) > 15 || strlen(user) > 15){
		handle400(client, path, "too long");
		return;
	}
	uint8_t t = add_reaction(user, message, id);
	if(t == 1){
		respond_with_chats(path, client);
	}
	else if (t == 0){
		char cId[10];
		sprintf(cId, "%d", currId);
		char error[strlen(id) + strlen(cId) + 2];
		memcpy(error, id, strlen(id));
		error[strlen(id)] = '>';
		memcpy(error + strlen(id) + 1, cId, strlen(cId));
		error[strlen(id) + strlen(cId) + 1] = 0;
		handle400(client, path, error);
		return;
	}
	else if (t == 2){
		handle_404(client, path);
		return;
	}
	//handle200(client, path);
}
/*
void handle_edit(char* path, int client){
	handlepath(path);
	char* start = strstr(path, "?");
	if (start == NULL){
		handle400(client, path, "no ?");
		return;
	}
	start++;

	char* idp = strstr(start, "id=");
	if (idp == NULL){
		handle400(client, path, "missing params");
		return;
	}
	char* andp = strstr(idp, "&");
	if (andp == NULL){
		handle400(client, path, "missing params");
		return;
	}
	char* msgp = strstr(andp, "message=");

	if (msgp == NULL || andp != msgp - 1){
		handle400(client, path, "missing params");
		return;
	}
	idp += 3;
	msgp += 8;
	char message[256];
	char id[10];

	int i = 0;
	for (; i < andp - idp && i < 10; i++){
		id[i] = idp[i];
	}
	id[i] = 0;
	i = 0;
	for (; i < &path[strlen(path)] - msgp && i < 256; i++){
		message[i] = msgp[i];
	}
	message[i] = 0;
	if (strlen(message) > 255){
		handle400(client, path, "too long");
		return;
	}

	uint8_t t = edit(id, message);
	if (t == 1){
		respond_with_chats(path, client);
		return;
	}
	else if (t == 0){
		char cId[10];
		sprintf(cId, "%d", currId);
		char error[strlen(id) + strlen(cId) + 2];
		memcpy(error, id, strlen(id));
		error[strlen(id)] = '>';
		memcpy(error + strlen(id) + 1, cId, strlen(cId));
		error[strlen(id) + strlen(cId) + 1] = 0;
		handle400(client, path, error);
		return;
	}
	return;
}

*/
void handle_response(char *request, int client_sock) {
    char path[1001];

    printf("\nSERVER LOG: Got request: \"%s\"\n", request);

    // Parse the path out of the request line (limit buffer size; sscanf null-terminates)
    if (sscanf(request, "GET %255s", path) != 1) {
        printf("Invalid request line\n");
	char response[] = "Invalid request line \n";
	write(client_sock, response, strlen(response));
        return;
    }

    if (strlen(path) > 1000){
	    handle_404(client_sock, path);
	    return;
    }

    if (strstr(path, "/post") == path){
	    handle_post(path, client_sock);
	    return;
    }
    else if (strstr(path, "/react") == path){
	    handle_reaction(path, client_sock);
	    return;
    }
    else if (strstr(path, "/chats") == path){
	    if (strlen(path) != 6){
		    handle400(client_sock, path, "chats not alone");
		    return;
	    }
	    respond_with_chats(path, client_sock);
	    //handle200(client_sock, path);
	    return;
    }
    else if (strstr(path, "/reset") == path){
	    if (strlen(path) != 6){
		    handle400(client_sock, path, "reset not alone");
		    return;
	    }
	    write(client_sock, HTTP_200_OK, strlen(HTTP_200_OK));
	    reset();
	    //handle200(client_sock, path);
	    return;
    }
    else if (strstr(path, "/edit") == path){
	    //handle_edit(path, client_sock);
	    return;
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
