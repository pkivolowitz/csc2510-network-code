// Perry Kivolowitz  -  Carthage  College  Computer  Science
// Free to use / modify for any purpose. Leave this message.

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <memory.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/time.h>

// NOTE:
// NOTE:	There is a latent bug in this code. It is an exercise to the
// NOTE:	reader to realize what it is.  Hint, the code will work fine
// NOTE:	on two computers with the same processor.
// NOTE:

/*
	Build with:

	g++ -std=c++11 client.cpp -o client
*/

using namespace std;

bool keep_going = true;

struct MyException
{
	MyException(string f, int l, string m) : function(string(f)), line(l), msg(m) {};
	string function;
	int line;
	string msg;
};

#define MYEXCEPTION(m)		MyException(string(__FUNCTION__), __LINE__, string(m))

void SIGINTHandler(int)
{
    cout << endl << "signal caught" << endl;
    keep_going = false;
}

int InitializeNetworkConnection(int argc, char * argv[])
{
	int server_socket = -1;
	int port = 5077;
	char * ip = (char *) ("127.0.0.1");
	int opt;

	while ((opt = getopt(argc, argv, "h:p:")) != -1) 
	{
		switch (opt) 
		{
			case 'h':
				ip = optarg;
				break;

			case 'p':
				port = atoi(optarg);
				break;
		}
	}

	server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket < 0)
		throw MYEXCEPTION(string(strerror(errno)));

	hostent * server_hostent = gethostbyname(ip);
	if (server_hostent == nullptr) {
		close(server_socket);
		throw  MYEXCEPTION("Failed gethostbyname()");
	}

	sockaddr_in server_sockaddr;
	memset(&server_sockaddr, 0, sizeof(sockaddr_in));
	server_sockaddr.sin_family = AF_INET;
	memmove(&server_sockaddr.sin_addr.s_addr, server_hostent->h_addr, server_hostent->h_length);
	server_sockaddr.sin_port = htons(port);

	if (connect(server_socket, (struct sockaddr*) &server_sockaddr, sizeof(sockaddr_in)) == -1)
	{
		throw MYEXCEPTION(string(strerror(errno)));
	}

	return server_socket;
}

void Send(int server_socket, string l)
{
	size_t length = l.size();
	size_t bytes_sent;
	size_t wire_length = htonl(length);

	bytes_sent = send(server_socket, (void *) &wire_length, sizeof(wire_length), 0);
	if (bytes_sent != sizeof(wire_length))
		throw MYEXCEPTION("sending length of string");

	bytes_sent = send(server_socket, (void *) &l[0], length, 0);
	if (bytes_sent != length)
		throw MYEXCEPTION("sending string");
}

int main(int argc, char * argv[])
{
	int server_socket = -1;
	int rv = 0;
	string l;

	if (signal(SIGINT, SIGINTHandler) == SIG_ERR) {
		cerr << "Setting signal failed." << endl;
		return 1;
	}

	try {
		server_socket = InitializeNetworkConnection(argc, argv);
		while (keep_going) {
			cout << "> ";
			getline(cin, l);
			Send(server_socket, l);
		}
	}
	catch (MyException e) {
		cerr << e.function << " " << e.line << " " << e.msg << endl;	
		rv = 1;
	}

	if (server_socket >= 0)
		close(server_socket);

	return rv;
}
