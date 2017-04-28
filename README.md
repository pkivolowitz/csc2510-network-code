# Carthage College Computer Organization Networking Demo

This code implements a multi-threaded server along with a client.

From the client, you can send strings which are printed on the server side. 
Sending "quit" (no quotes) causes the thread on the other side to be closed. 
This will cause the client to close as soon as it notices the connection to
the server has been lost.

server options: -p port
client options: -p port -h server_ip_address

# Framing

Framing of strings is accomplished be prepending the length of the string.

# There is a latent bug in this code

See if you can find it. Here is a hint:

<pre>
#include &lt;iostream>

using namespace std;

int main()
{

#if defined(_WIN32) && !defined(_WIN64)
	cout << "x86 binary" << endl;
#elif defined(_WIN64)
	cout << "x64 binary" << endl;
#endif

	cout << "sizeof(unsigned int): " << sizeof(unsigned int) << endl;
	cout << "sizeof(size_t):       " << sizeof(size_t) << endl;

#if defined(_WIN32) || defined(_WIN64)
	system("pause");
#endif

	return 0;
}
</pre>
