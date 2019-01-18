#pragma once

#include <string>

struct sqlite3;

typedef std::pair <std::string, bool> tData;

class CSQLLite
{
public:
	CSQLLite() = default;
	~CSQLLite();

	bool open(const std::string& aFileName);
	bool state();
	void close();

	static bool get(const std::string& aVar, tData &aData);
	static bool set(const std::string& aVar, const tData &aData);

private:
	static sqlite3 *mDB;
};

