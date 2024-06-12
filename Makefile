all: UDPEchoClient UDPEchoServer
UDPEchoClient: UDPEchoClientGardener.c DieWithError.c
	gcc UDPEchoClient.c DieWithError.c -o UDPEchoClient
UDPEchoServer: UDPEchoServer.c DieWithError.c
	gcc UDPEchoServer.c DieWithError.c -o UDPEchoServer
