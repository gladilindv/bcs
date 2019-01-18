#pragma once

#include <string>

class CSocket;

class CCommand
{
public:
	CCommand() = delete;
	~CCommand() = default;
	CCommand(std::unique_ptr<CSocket> &&);

	void run();

private:
	std::unique_ptr<CSocket> mSock;

	void doGet(const std::string&);
	void doSet(const std::string&, const std::string&);
};

