#include <string>
using namespace std;
class Server {
public:

	int opt = 1;
	int master_socket, addrlen, client_socket[8], max_clients, activity, valread, sd, max_sd;
	struct sockaddr_in address;
	char buffer[1024];  //data buffer of 1K
	int PORT;
	fd_set readfds;
	Server(int port,int max) {
		PORT = port;
		max_clients = max;
		char message[] = "90,90,90!";
		for (int i = 0; i < max_clients; i++) {//initialise all client_socket[] to 0 so not checked  
			client_socket[i] = 0;
		}
		if ((master_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {//create a master socket  
			perror("socket failed");
			exit(EXIT_FAILURE);
		}
		if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char*)& opt, sizeof(opt)) < 0) {
			//set master socket to allow multiple connections ,  this is just a good habit, it will work without this  
			perror("setsockopt");
			exit(EXIT_FAILURE);
		}
		address.sin_family = AF_INET;	//type of socket created  
		address.sin_addr.s_addr = INADDR_ANY;
		address.sin_port = htons(PORT);

		if (bind(master_socket, (struct sockaddr*) & address, sizeof(address)) < 0) {//bind the socket to localhost port 1336  
			perror("bind failed");
			exit(EXIT_FAILURE);
		}
		printf("Listener on port %d \n", PORT);

		if (listen(master_socket, 8) < 0) {//try to specify maximum of 8 pending connections for the master socket  
			perror("listen");
			exit(EXIT_FAILURE);
		}
		addrlen = sizeof(address);//accept the incoming connection  
		puts("Waiting for connections ...");
	}
	string ReadClient(int i) {
		sd = i;
		if (FD_ISSET(sd, &readfds)) {//Check if it was for closing , and also read the incoming message  
			if ((valread = read(sd, buffer, 1024)) == 0) {//Somebody disconnected , get his details and print  
				getpeername(sd, (struct sockaddr*) & address, (socklen_t*)& addrlen);
				printf("Host disconnected , ip %s , port %d \n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
				close(sd);
				//client_socket[i] = 0;
			}
			else {
				buffer[valread] = '\0';//set the string terminating NULL byte on the end of the data read
				return buffer;
			}
			return "-1";
		}
		return "-1";
	}
	string* ReadAllClients() {
		string* out = new string[8];
		for (int i = 0; i < 8;	i++) {
			out[i]=ReadClient(i);
		}
		return out;
	}
	int Activity() { //time this
		int new_socket = 0;
		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 500000;

		activity = select(max_sd + 1, &readfds, NULL, NULL, &tv);//wait for an activity on one of the sockets , timeout is NULL ,  so wait indefinitely 
		if ((activity < 0) && (errno != EINTR))	 printf("select error");
		if (FD_ISSET(master_socket, &readfds)) {//If something happened on the master socket ,  then its an incoming connection  
			if ((new_socket = accept(master_socket,(struct sockaddr*) & address, (socklen_t*)& addrlen)) < 0) {
				perror("accept");
				exit(EXIT_FAILURE);
			}
			printf("New connection , socket fd is %d , ip is : %s , port : %d  \n", new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));//inform user of socket number - used in send and receive commands  

			return new_socket; //return !0 if somthing happens
		}

		return new_socket; // return 0 if nothing happens
	}

	void AddSocket(int new_socket) {
		if (new_socket != 0) {
			for (int i = 0; i < max_clients; i++) {//add new socket to array of sockets  
				if (client_socket[i] == 0) {//if position is empty 
					client_socket[i] = new_socket;
					printf("Adding to list of sockets as client number %d\n", i);
					break;
				}
			}
		}
	}
	void Clean() { //time this
		FD_ZERO(&readfds);//clear the socket set  
		FD_SET(master_socket, &readfds);//add master socket to set  
		max_sd = master_socket;
		for (int i = 0; i < max_clients; i++) { //add child sockets to set    
			sd = client_socket[i];//socket descriptor
			if (sd > 0)	FD_SET(sd, &readfds);//if valid socket descriptor then add to read list  
			if (sd > max_sd) max_sd = sd;//highest file descriptor number, need it for the select function  
		}
	}
	void SendAll(string message) {
		for (int i=0; i < max_sd; i++) {
			send(client_socket[i], message.c_str(), strlen(message.c_str()), 0); //simple send all
		}
	}
};
