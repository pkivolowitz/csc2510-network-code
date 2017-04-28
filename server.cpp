// Perry Kivolowitz  -  Carthage  College  Computer  Science
// Free to use / modify for any purpose. Leave this message.

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <thread>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <arpa/inet.h>
#include <getopt.h>

// NOTE:
// NOTE:	There is a latent bug in this code. It is an exercise to the
// NOTE:	reader to realize what it is.  Hint, the code will work fine
// NOTE:	on two computers with the same processor.
// NOTE:

/*	This multi-threaded server will accept up to 10 connections at once.
	It will read framed strings, and print them. Receiving "quit" causes
	the thread handling the connection to quit.

	Build with:

	g++ -std=c++11 server.cpp -pthread -o server

*/

using namespace std;

bool keep_going = true;
const int max_connections = 10;

/*	This is a fairly nice touch. Note the use of the #define to 
	correctly capture __LINE__ and __FUNCTION__.
*/

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

void ConnectionHandler(sockaddr_in client_info, int socket, int cid)
{
	size_t length;
	size_t bytes_read;
	string s;

	try
	{
		while (true) {
			bytes_read = recv(socket, (void *) &length, sizeof(length), 0);
			if (bytes_read != sizeof(length))
				throw MYEXCEPTION("");

			length = ntohl(length);
			s.resize(length);

			bytes_read = recv(socket, (void *) &s[0], length, 0);
			if (bytes_read != length)
				throw MYEXCEPTION("");

			cout << "Conn[" << cid << "]: " << s << endl;
			if (s == "quit")
				throw MYEXCEPTION("");
		}
	}
	catch (MyException e)
	{
		cerr << e.function << " " << e.line << " " << e.msg << endl;		
	}

	close(socket);
}

void AcceptConnections(int port)
{
	vector<thread *> threads;

	// By default, Linux apparently retries  interrupted system calls.  This defeats the
	// purpose of interrupting them, doesn't it? The call to siginterrupt disables this.

	signal(SIGINT, SIGINTHandler);
	siginterrupt(SIGINT, 1);

	int incoming_socket = -1;
	int listening_socket = -1;

	try 
	{
		listening_socket = socket(AF_INET, SOCK_STREAM, 0);
		if (listening_socket < 0) {
			throw MYEXCEPTION("opening listening socket failed " + string(strerror(errno)));
		}

		// This sockopt allows immediate reuse of a formerly bound port.

		int optval = 1;
		optval = setsockopt(listening_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
		if (optval < 0) {
			throw MYEXCEPTION("setting socket opt failed " + string(strerror(errno)));
		}

		sockaddr_in listening_sockaddr;

		memset(&listening_sockaddr, 0, sizeof(sockaddr_in));
		listening_sockaddr.sin_family = AF_INET;
		listening_sockaddr.sin_port = htons(port);
		listening_sockaddr.sin_addr.s_addr = INADDR_ANY;

		if (bind(listening_socket, (sockaddr *) &listening_sockaddr, sizeof(sockaddr_in)) < 0) {
			throw MYEXCEPTION("bind failed " + string(strerror(errno)));
		}

		if (listen(listening_socket , max_connections) != 0) {
			throw MYEXCEPTION("listen failed " + string(strerror(errno)));
		}

		sockaddr_in client_info;
		memset(&client_info, 0, sizeof(sockaddr_in));
		int c = sizeof(sockaddr_in);

		int connection_counter = 0;

		while ((incoming_socket = accept(listening_socket, (sockaddr *) &client_info, (socklen_t *) &c)) > 0) {
			if (!keep_going)
				break;

			thread * t = new thread(ConnectionHandler, client_info, incoming_socket, connection_counter++);
			threads.push_back(t);
		}
	}
	catch (MyException e) {
		cerr << e.function << " " << e.line << " " << e.msg << endl;
	}

	if (incoming_socket >= 0)
		close(incoming_socket);

	if (listening_socket >= 0)
		close(listening_socket);

	// NOTE:
	// NOTE:	This is a questionable way of joining threads. A better
	// NOTE:	way would  notice a thread's  termination  earlier  and
	// NOTE:	join then. This code joins only when the application is
	// NOTE:	exiting.
	// NOTE:

	for (auto it = threads.begin(); it < threads.end(); it++) {
		if (*it != nullptr) {
			(*it)->join();
		}
	}
}

int main(int argc, char * argv[])
{
	int port = 5077;
	int opt;

	while ((opt = getopt(argc, argv, "p:")) >= 0) {
		switch (opt) {
			case 'p':
				port = atoi(optarg);
				break;

			default:
				break;
		}
	}
	AcceptConnections(port);
	return 0;
}

