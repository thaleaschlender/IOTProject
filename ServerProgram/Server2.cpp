//Example code: A simple server side code, which echos back the received message. 
//Handle multiple socket connections with select and fd_set on Linux  
#include <stdio.h>
#include <string.h>   //strlen  
#include <stdlib.h>  
#include <errno.h>  
#include <unistd.h>   //close  
#include <arpa/inet.h>    //close  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros  
#include <string>
#include <sstream>
#include <iostream> 
#include <fstream> 
#include <array> // for array, at() 
#include "Server.h"	

#define PORT 8888
#define MAX 8 //note if changed change in header aswell

using namespace std;

int main(int echo,char** unused){
	Server server(PORT,MAX);
	int socket_count = 0;
	int sockets[MAX];
	string data = "";
	ofstream ofs;
	ofs.open("log.txt", ofstream::out | ofstream::trunc);
	ofs.close();
	bool hit = false;
	while (1) {
		string fbstate = "";
		string lrstate = "";
		string dustate = "";
		string angle = "";
		string time = "0";
		string dir = "";
		server.Clean();
		int new_socket = server.Activity();

		

		if (new_socket != 0) {
			server.AddSocket(new_socket);
			sockets[socket_count] = new_socket;
			socket_count++;
		}
		//receive
		for (int i = 0; i < MAX; i++) {
			 string msg = server.ReadClient(i);
			 if (!(msg == "-1")) {
				 cout <<msg<<endl;
				 string x,y,z;
				 stringstream ss(msg);
				 getline(ss, x,',');
				 getline(ss, y, ',');
				 getline(ss, z, '!');

				 int xi = atoi(x.c_str());
				 int yi = atoi(y.c_str());
				 int zi = atoi(z.c_str());

				 hit = true;
				 
				 if (xi < 1000) {
					 fbstate = "f";
				 }
				 if (xi > 3000) {
					 fbstate = "b";
				 }
				 if (yi < 1000) {
					 lrstate = "l";
				 }
				 if (yi > 3000) {
					 lrstate = "r";
				 }
				 if (zi = 0) {
					 dustate = "d";
				 }
				 else {
					 dustate = "u";
				 }
			 }
		}
		ifstream fp;
		fp.open("log.txt");
		while (fp) {
			getline(fp, data);
		}
		fp.close();

		ofs.open("log.txt", ofstream::out | ofstream::trunc);
		ofs.close();

		//process
		if (hit) {
			if (fbstate == "f") {
				if (lrstate == "l") {
					angle = "45";
					dir = "1";
					time = "50";
				}
				else {
					if (lrstate == "r") {
						angle = "135";
						dir = "1";
						time = "50";
					}else{
						angle = "90";
						dir = "1";
						time = "0";
				}
			}
				
			}

			if (fbstate == "b") {
				if (lrstate == "r") {
					angle = "45";
					dir = "0";
					time = "50";
				}
				else {
					if (lrstate == "l") {
						angle = "135";
						dir = "0";
						time = "50";
					}else{
						angle = "90";
						dir = "0";
						time = "0";
					}
				}
			}
			
			data = time + ","+dir+","+angle+"!";
		}

		//send
		if (!(time == "0")) {
			cout << data << endl;
			data = data + '\n';
			server.SendAll(data);
		}
	}
	return 0;
}
