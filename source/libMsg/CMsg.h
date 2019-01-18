#pragma once
#include "libmsg_global.h"
#include <string>
#include <tuple>

typedef std::pair<bool, std::string> tResponse;
typedef std::tuple<short, std::string, std::string> tRequest;

class LIBMSG_EXPORT CMsg {
public:
	static std::string createRequest(const std::string&);
	static std::string createRequest(const std::string&, const std::string&);
	static tRequest parseRequest(const char* aReq, int aSize);

	static std::string createResponse(const tResponse&);
	static std::string createResponse(bool, const std::string&);
	static tResponse parseResponse(const char* aReq, int aSize);
};
