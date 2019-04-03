#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <pwd.h>

#define PORT 3333
#define SECRET "cs30618spr"

void handle_client(int connect_fd);

	int main(int argc, char *argv[]){
	
		//char msg[201];
		int listen_fd, connect_fd;
		//int nread;
		struct sockaddr_in servaddr;
		
		//Create main server socket
		listen_fd = socket(AF_INET, SOCK_STREAM, 0);
		
		if(listen_fd == -1){
			return EXIT_FAILURE;
		}
		
		//Allow immediate reuse of port
		int i = 1;
		if(setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(i)) == -1){
			return EXIT_FAILURE;
		}
		
		//Set up server address struct and give socket address
		servaddr.sin_family = AF_INET;
		servaddr.sin_port =  htons(PORT);
		servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
		
		if(bind(listen_fd, (struct sockaddr *) &servaddr, sizeof(servaddr)) == -1){
			return EXIT_FAILURE;
		}
		
		//Start socket listening
		if(listen(listen_fd, 10) == -1){
			return EXIT_FAILURE;
		}
		
		//Main server loop
		while(1){
			//Accept a new connection and get socket to use for client
			connect_fd = accept(listen_fd, (struct sockaddr *) NULL, NULL);
			
			handle_client(connect_fd);
			
			//Read client message and print it
			/*nread = read(connect_fd, msg, 200);
			msg[nread] = '\0';
			printf("Client message: %s\n", msg);*/
		}
	
	}
	
	void handle_client(int connect_fd){
		
		char client_message[1024];
		char *remps = "<remps>";
		char *ready = "<ready>";
		char opt[1024];
		char secret_buff[strlen(SECRET) + 3];
		int optNum = 0;
		int oldfd1;
		int oldfd2;
		char user[8096];
		char command[8133];
		int nuser = 0;
		snprintf(secret_buff, sizeof(secret_buff), "<%s>", SECRET);
		
		if(write(connect_fd, remps, strlen(remps)) == -1){//send remps
			close(connect_fd);
		}
		 
		if(read(connect_fd, client_message, 1024) == -1){ //receive secret
			close(connect_fd);
		}
		
		if(strncmp(client_message, secret_buff, strlen(secret_buff)) != 0){ //check secret
			close(connect_fd);
		}
		
		if(write(connect_fd, ready, strlen(ready)) == -1){ //send ready
			close(connect_fd);
		}
		
		optNum = read(connect_fd, opt, sizeof(opt)); //read directive
		
		if(optNum == -1){
			close(connect_fd);
		}
		
		oldfd1 = dup(1);//save old file descriptors
		oldfd2 = dup(2);
		dup2(connect_fd, 1);//redirect output
		dup2(connect_fd, 2);
		
		//write(connect_fd, opt, optNum);
		
		if(strncmp(opt, "<cpu>", optNum) == 0){
			system("ps -NT -o pid,ppid,%cpu,%mem,args --sort -%cpu | head");
		} else if(strncmp(opt, "<mem>", optNum) == 0){
			system("ps -NT -o pid,ppid,%cpu,%mem,args --sort -%mem | head");
		} else if(strncmp(opt, "<user>", optNum) == 0){
			nuser = read(connect_fd, user, sizeof(user));
			if(nuser == -1){
				close(connect_fd);
			}
			if(getpwnam(user) == NULL){
				close(connect_fd);
				return;
			}
			//write(1, user, nuser);
			snprintf(command, sizeof(command), "ps -u %s -o pid,ppid,%%cpu,%%mem,args", user);
			//write(1, command, sizeof(command));
			system(command);
		} else {
			close(connect_fd);
		}
		
		dup2(oldfd1, 1);
		dup2(oldfd2, 2);
		
		close(connect_fd);
		
	}
