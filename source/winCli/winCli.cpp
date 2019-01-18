// winCli.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"

#include "CSocket.h"
#include "CCommand.h"

#include <iostream>

#pragma comment (lib, "Ws2_32.lib")


#define DEFAULT_PORT "27015"

void usage(const char* progName)
{
	std::cout << progName << " [host] [port]" << std::endl <<
		"Options:" << std::endl <<
		"\thost       The IP address of the server application" << std::endl <<
		"\tport       The port number of the server application" << std::endl;
}

int main(int argc, const char* argv[])
{
	// parse connection arg
	auto host = "localhost";
	auto port = DEFAULT_PORT;
	if (argc != 3) {
		usage(argv[0]);
		return 0;
	}
	
	host = argv[1];
	port = argv[2];

	// transport object
	auto sock = std::make_unique<CSocket>(host, port);
	if (!sock->init())
		return 1;

	// input command parser
	CCommand cmd(std::move(sock));
	cmd.run();

	return 0;
}

