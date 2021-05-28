#include <iostream>
#include <string>
#include <WS2tcpip.h>

#pragma comment (lib, "ws2_32.lib")

using namespace std;

bool newGame(short nOfPlayers);
void initializeServer();

int main()
{
	short tmp;

	cout << "1] New game" << endl;
	cout << "______________" << endl;
	cout << ": ";
	cin >> tmp;

	switch (tmp) {
	case 1:
		tmp = 0;
		cout << "------------" << endl;
		while ((tmp < 2) || (tmp > 4)) {
			cout << "Number of players (max 4; have at least 2 to start): ";
			cin >> tmp;
		}
		if (!newGame(tmp)) {
			cout << "### Shutting down...";
			return 1;
		}
		break;
	default:
		cout << "### Wrong input, try again" << endl;
		main();
		break;
	}

	return 0;
}

bool newGame(short nOfPlayers) {
	try {
		initializeServer();
	}
	catch (exception e) {
		cout << "### There was a problem with initializing game server, terminating..." << endl;
		return false;
	}
}

void initializeServer() {
	// Initialze winsock
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);

	int wsOk = WSAStartup(ver, &wsData);
	if (wsOk != 0)
	{
		cerr << "### Can't Initialize winsock! Quitting..." << endl;
		return;
	}

	// Create a socket
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
	if (listening == INVALID_SOCKET)
	{
		cerr << "### Can't create a socket! Quitting..." << endl;
		return;
	}

	// Bind the ip address and port to a socket
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(54000);
	hint.sin_addr.S_un.S_addr = INADDR_ANY; // Could also use inet_pton .... 

	bind(listening, (sockaddr*)&hint, sizeof(hint));

	// Tell Winsock the socket is for listening 
	listen(listening, SOMAXCONN);

	// Wait for a connection
	sockaddr_in client;
	int clientSize = sizeof(client);

	SOCKET clientSocket = accept(listening, (sockaddr*)&client, &clientSize);

	char host[NI_MAXHOST];		// Client's remote name
	char service[NI_MAXSERV];	// Service (i.e. port) the client is connect on

	ZeroMemory(host, NI_MAXHOST); // same as memset(host, 0, NI_MAXHOST);
	ZeroMemory(service, NI_MAXSERV);

	if (getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
	{
		cout << host << " connected on port " << service << endl;
	}
	else
	{
		inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
		cout << host << " connected on port " <<
			ntohs(client.sin_port) << endl;
	}

	// Close listening socket
	closesocket(listening);

	// While loop: accept and echo message back to client
	char buf[4096];

	while (true)
	{
		ZeroMemory(buf, 4096);

		// Wait for client to send data
		int bytesReceived = recv(clientSocket, buf, 4096, 0);
		if (bytesReceived == SOCKET_ERROR)
		{
			cerr << "### Error in recv(). Quitting..." << endl;
			break;
		}

		if (bytesReceived == 0)
		{
			cout << "Client disconnected " << endl;
			break;
		}

		cout << "Client: " << string(buf, 0, bytesReceived) << endl;

		// Echo message back to client
		send(clientSocket, buf, bytesReceived + 1, 0);

	}

	// Close the socket
	closesocket(clientSocket);

	// Cleanup winsock
	WSACleanup();
}