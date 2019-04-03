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
#include <fcntl.h>
#include <pwd.h>

#define PORT 3333
#define SECRET "cs30618spr"

	int main(int argc, char *argv[]){
		//Get server's IP address and message from arguments
		char *server_ip = argv[1];
		char msg[1024];
		char buffer[1024];
		char option[1024];
		int nread = 0;
		uid_t ruid;
		int user_flag = 0;
		char *username;
		struct passwd *pwname;
		//struct passwd *pwuid;
		
		if(argc > 2){
			snprintf(option, sizeof(option) + 2, "<%s>", argv[2]);
		} else {
			snprintf(option, sizeof(option) + 2,"<user>");
		}
		
		if(strncmp(option, "<user>", 6) == 0){
			ruid = getuid();
			pwname = getpwuid(ruid);
			username = pwname->pw_name;
			user_flag = 1;
		}
		
		char secret_buff[strlen(SECRET) + 3];
		snprintf(secret_buff, sizeof(secret_buff), "<%s>", SECRET);
		
		int sockfd;
		struct sockaddr_in servaddr;
		
		//Create a socket
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		
		if(sockfd == -1){
			return EXIT_FAILURE;
		}
		
		//Set up server address struct
		servaddr.sin_family = AF_INET;
		servaddr.sin_port = htons(PORT);
		inet_aton(server_ip, &servaddr.sin_addr);
		
		
		
		//Connect to a server
		if(connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) == -1){
			close(sockfd);
			return EXIT_FAILURE;
		}
		
		if(read(sockfd, buffer, 1024) == -1){ //receive remps
			close(sockfd);
			return EXIT_FAILURE;
		}
		
		if(strncmp(buffer, "<remps>", 7) == 0){
			if(write(sockfd, secret_buff, strlen(secret_buff)) == -1){ //send secret
				close(sockfd);
				return EXIT_FAILURE;
			}
		}
		
		if(read(sockfd, buffer, 1024) == -1){ //recieve ready
			close(sockfd);
			return EXIT_FAILURE;
		}
		
		if(strncmp(buffer, "<ready>", 7) == 0){
			
			if(write(sockfd, option, strlen(option)) == -1){//send option
				close(sockfd);
				return EXIT_FAILURE;
			}
			
			if(user_flag == 1){
				if(write(sockfd, username, strlen(username)) == -1){//send username
					close(sockfd);
					return EXIT_FAILURE;
				}
			}
		}
		
		while((nread = read(sockfd, msg, 1024)) > 0){//print msg
			if(write(1, msg, nread) == -1){
				close(sockfd);
				return EXIT_FAILURE;
			}
			printf("\n");
		}
		
		//Close connection
		close(sockfd);
		
		if(nread == -1){
			return EXIT_FAILURE;
		}
		
		return EXIT_SUCCESS;
	}
