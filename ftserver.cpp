/************************************************************************
** Program Name: ftserver
** Author: Joshua Curtis
** Date: November, 29 2015
** Description: 
** - Server for file transfer program
*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <iostream>
#include <dirent.h>
#include <time.h>
#include <istream>
#include <fstream>
#include <iomanip>

#define MAXDATASIZE 501
#define BACKLOG 10	 // how many pending connections queue will hold
#define ACK 2

using std::string;
using namespace std;

void sigchld_handler(int s)
{
	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;

	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

//print directory
int printDir (void)
{
//copy the directory into the message
    DIR *dp;
    struct dirent *ep; 
	std::string dirList;
    dp = opendir ("./");

    if (dp != NULL)
    {
        while (ep = readdir (dp)){
			dirList.append(ep->d_name);
			dirList.append("\n");
		}
    (void) closedir (dp);
	printf("client: connecting to %s\n", dirList.c_str());
    }
    else
        perror ("Couldn't open the directory");
  return 0;
}

int dataConnect(char const* cHostName, char const* DATA_PORT, char const* message, char const* fileName) 
{
	int datafd, numbytes;  
	struct addrinfo dataCon, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];
	DIR *dp;
    struct dirent *ep; 
	std::string dirList;
    dp = opendir ("./");
	ifstream fp;
	std::string fileMessage;

	//Check to make sure host name and port number are specified
	memset(&dataCon, 0, sizeof dataCon);
	dataCon.ai_family = AF_UNSPEC;
	dataCon.ai_socktype = SOCK_STREAM;

	// Load address information into struct
	if ((rv = getaddrinfo(cHostName, DATA_PORT, &dataCon, &servinfo)) != 0) 
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) 
	{
		if ((datafd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) 
		{
			perror("client: socket");
			continue;
		}
		
		//connect the socket to the server
		if (connect(datafd, p->ai_addr, p->ai_addrlen) == -1) 
		{
			close(datafd);
			perror("client: connect");
			continue;
		}
		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}
	
	//Convert IP address to human readable form and print
	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),s, sizeof s);

	freeaddrinfo(servinfo); // all done with this structure

	if (message == "send list") {
		if (dp != NULL) {
        while (ep = readdir (dp)){
			dirList.append(ep->d_name);
			dirList.append("\n");
		}
		(void) closedir (dp);
		}
		message = dirList.c_str();
		//Send message to client
        if ((send(datafd, message, strlen(message),0))== -1) {
                fprintf(stderr, "Failure Sending Message\n");
                close(datafd);
                exit(1);
        }
	}
	
	if (message == "send file") {
		if (dp != NULL) {
			message = "FILE NOT FOUND";
			while ((ep = readdir (dp)) !=NULL){
				if (strcmp(fileName,ep->d_name) == 0) {
					message = "File found in directory";
					printf("Sending %s",fileName);
					printf("to %",cHostName);
					printf(":",DATA_PORT);
					break;
				}
			}
			(void) closedir (dp);
		}
		if ((send(datafd, message, strlen(message),0))== -1) {
            fprintf(stderr, "Failure Sending Message\n");
            close(datafd);
            exit(1);
		}
		if (message == "File found in directory") {
			fp.open(fileName);
			while(!fp.eof()) {
				getline(fp, fileMessage);
				message = fileMessage.c_str();
				if ((send(datafd, message, strlen(message),0))== -1) {
					fprintf(stderr, "Failure Sending Message\n");
					close(datafd);
					exit(1);
				}
			}
		}
		if (message == "FILE NOT FOUND") {
			printf("File Not Found. Sending error message to %s",cHostName);
			printf(":",DATA_PORT);
		}
	}
	close(datafd);
	return 0;
}

int main(int argc, char *argv[])
{
	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv;
	int numbytes;
	char buffer[MAXDATASIZE]; //Stores messages sent to and from the client
	string data[4];
	char const* DATA_PORT;
	char const* ClientHostName;
	char const* message;
	char const* fileName;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	// Get address info from client and load information into struct
	if ((rv = getaddrinfo(NULL, argv[1], &hints, &servinfo)) != 0) 
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {	
		if ((sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) 
		{
			perror("server: socket");
			continue;
		}
		
		//Set socket options
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,sizeof(int)) == -1) 
		{
			perror("setsockopt");
			exit(1);
		}
		//Associate the socket with the IP address
		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) 
		{
			close(sockfd);
			perror("server: bind");
			continue;
		}
		break;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	printf("server: waiting for connections...\n");

	while(1) {  // main accept() loop
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}

		//Convert IP address to human readable form and print
		inet_ntop(their_addr.ss_family,get_in_addr((struct sockaddr *)&their_addr),s, sizeof s);
		printf("server: got connection from %s\n\n", s);
		
		//Loop to keep connection open while messages are being sent
		while(1) {
			//Receive data from socket and check for error
            if ((numbytes = recv(new_fd, buffer, 1024,0))== -1) {
                perror("recv");
                exit(1);
            }
            else if (numbytes == 0) {
                printf("Connection closed\n");
                        //So chatserve can now wait for another client
                        break;
                }
            buffer[numbytes] = '\0';
			
			//Put message from client into tokens and print tokens
			data[0] = strtok (buffer, " ");
			data[1] = strtok (NULL, " ");
			data[2] = strtok (NULL, " ");
			data[3] = strtok (NULL, " ");
			if (data[0] == "-l") {
				DATA_PORT = data[1].c_str();
				ClientHostName = data[2].c_str();
				printf("Connection from %s\n",ClientHostName);
				printf("list directory requested ");
				printf("on port %s\n",DATA_PORT);
				printf("sending directory contents to %s",ClientHostName);
				printf(":%s\n",DATA_PORT);
				message = "send list";
				dataConnect(ClientHostName, DATA_PORT, message, fileName);
			}
			else if (data[0] == "-g") {
			    fileName = data[1].c_str();
				DATA_PORT = data[2].c_str();
				ClientHostName = data[3].c_str();
				printf("Connection from %s\n",ClientHostName);
				printf("file %s requested",fileName);
				printf(" on port %s\n",DATA_PORT);
				message = "send file";
				dataConnect(ClientHostName, DATA_PORT, message, fileName);
			}
            close(new_fd);
            break;
        } //End of Inner While...
	} //Outer While
	close(sockfd);
	return 0;
}

