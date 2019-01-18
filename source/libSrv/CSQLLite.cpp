#include "CSQLLite.h"

#define THREADSAFE 1
#define SQLITE_THREADSAFE 1

#include "sqlite3.h"
#include <sstream>
#include <windows.h>

sqlite3* CSQLLite::mDB = nullptr;

CSQLLite::~CSQLLite() {
	if (mDB)
		close();
}

bool CSQLLite::open(const std::string& aFileName) {
	bool isNewDB = false;
	// check file exist // most used way on msdn 
	if (GetFileAttributesA(aFileName.c_str()) == INVALID_FILE_ATTRIBUTES)
		isNewDB = true;

	int error = sqlite3_open(aFileName.c_str(), &mDB);
	if (error) {
		sqlite3_close(mDB);
		mDB = nullptr;
		return false;
	}

	if (isNewDB)
	{
		sqlite3_exec(mDB, "CREATE TABLE VARIABLES (VARIABLE TEXT NOT NULL PRIMARY KEY, VALUE TEXT)", NULL, NULL, NULL);
	}

	return true;
}

bool CSQLLite::state() {
	return mDB != nullptr;
}

void CSQLLite::close()
{
	sqlite3_close(mDB);
	mDB = nullptr;
}

bool CSQLLite::get(const std::string& aVar, tData& aData) {
	// clean data
	aData.first = "";
	aData.second = false;

	// prevent SQL injection -> prepared statements + escape quote
	sqlite3_stmt* stmt = nullptr;
	auto rc = sqlite3_prepare_v2(mDB, "select value from variables where variable=?", -1, &stmt, 0);
	if (rc != SQLITE_OK) {
		aData.first = sqlite3_errmsg(mDB);
		return false;
	}

	rc = sqlite3_bind_text(stmt, 1, aVar.c_str(), -1, 0);
	if (rc != SQLITE_OK) {
		aData.first = sqlite3_errmsg(mDB);
		return false;
	}

	rc = sqlite3_step(stmt);
	if (rc == SQLITE_ROW) {
		auto val = (const char *)sqlite3_column_text(stmt, 0);
		aData.first = val ? val : "NULL";
		aData.second = true;
	}

	sqlite3_finalize(stmt);
	return true;
}

bool CSQLLite::set(const std::string& aVar, const tData& aData) {
	std::string val = aData.first;

	sqlite3_stmt* stmt = nullptr;
	auto rc = sqlite3_prepare_v2(mDB, "insert into variables(variable, value) values (?, ?) on conflict(variable) do update set value=?;", -1, &stmt, 0);
	if (rc != SQLITE_OK) {
		//aData.first = sqlite3_errmsg(mDB);
		return false;
	}

	rc = sqlite3_bind_text(stmt, 1, aVar.c_str(), -1, 0);
	if (rc != SQLITE_OK) {
		//aData.first = sqlite3_errmsg(mDB);
		return false;
	}

	rc = sqlite3_bind_text(stmt, 2, val.c_str(), -1, 0);
	if (rc != SQLITE_OK) {
		//aData.first = sqlite3_errmsg(mDB);
		return false;
	}

	rc = sqlite3_bind_text(stmt, 3, val.c_str(), -1, 0);
	if (rc != SQLITE_OK) {
		//aData.first = sqlite3_errmsg(mDB);
		return false;
	}

	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE) {
		//aData.first = sqlite3_errmsg(mDB);
		return false;
	}

	sqlite3_finalize(stmt);
	return true;
}
