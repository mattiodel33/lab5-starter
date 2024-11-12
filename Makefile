all: chat-server

chat-server: http-server.c number-server.c
	gcc -std=c11 -Wall -Wno-unused-variable -fsanitize=address -g http-server.c number-server.c -o chat-server

clean:
	rm -f chat-server


