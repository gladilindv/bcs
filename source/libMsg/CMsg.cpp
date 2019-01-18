// libMsg.cpp : Defines the exported functions for the DLL application.
//
#include "CMsg.h"
#include "libMsg.pb.h"


std::string CMsg::createRequest(const std::string& aVar) {
	Request r;

	r.set_command(Request_Command::Request_Command_GET);
	r.set_variable(aVar);

	return r.SerializeAsString();
}

std::string CMsg::createRequest(const std::string& aVar, const std::string& aVal) {
	Request r;

	r.set_command(Request_Command::Request_Command_SET);
	r.set_variable(aVar);
	r.set_value(aVal);

	return r.SerializeAsString();
}

tRequest CMsg::parseRequest(const char* aReq, int aSize) {
	Request r;
	
	if (!r.ParseFromArray(aReq, aSize))
		return tRequest(-1,"","");

	return tRequest(r.command(),r.variable(), r.value());
}

std::string CMsg::createResponse(const tResponse& aRes) {
	return createResponse(aRes.first, aRes.second);
}

std::string CMsg::createResponse(bool aState, const std::string& aMsg) {
	Response r;

	r.set_state(aState);
	r.set_value(aMsg);

	return r.SerializeAsString();
}

tResponse CMsg::parseResponse(const char* aRsp, int aSize) {
	Response r;
	if (!r.ParseFromArray(aRsp, aSize))
		return tResponse(false, "parse error");

	return tResponse(r.state(), r.value());
}
