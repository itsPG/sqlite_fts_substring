#include "sqlite3.h"
#ifdef __cplusplus
extern "C" {
#endif 
#include "character_tokenizer.h"
#ifdef __cplusplus
}
#endif 
#define DEBUGINFO(stat) stat
#define DEBUGMSG(msg) cout << msg << endl;
#define DEBUGMSGN(msg) cout << msg;
#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
using namespace std;
//int db;

class SQLite3DBCommand
{
public:

	int open(const string &path)
	{
		return open(path, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
	}
	int open(const string &path, int flag, const char *szVfs)
	{
		int ret;
		ret = sqlite3_open_v2(path.c_str(), &db, flag, szVfs);
		if (ret) {
			DEBUGMSG("[SQLite3DB open error] @ sqlite3_open_v2")
			return ret;
		}
		ret = registerCharacterTokenizer();
		if (ret) {
			DEBUGMSG("[SQLite3DB open error] @ registerCharacterTokenizer")
			return ret;
		}
		return ret;
	}
	int exec(const string &cmd)
	{
		return exec(cmd, 0, 0);
	}
	int exec(const string &cmd, int (*callback)(void*,int,char**,char**), void* data)
	{
		int ret;
		char *errMsg = NULL;
		ret = sqlite3_exec(db, cmd.c_str(), callback, data, &errMsg);
		if (errMsg != NULL) {
			cmdFailed("exec", cmd, errMsg);
			return ret;
		}
		return 0;
	}
	/*
		execf will not use const string &cmd
		because 'va_start' has undefined behavior with reference types.
	*/
	int execf(const string cmd, ...) 
	{
		va_list argv;
		char *szSQL;
		va_start(argv, cmd);
		szSQL = sqlite3_vmprintf(cmd.c_str(), argv);
		exec(string(szSQL));
		sqlite3_free(szSQL);
	}
	
	int get_table(const string &query, int &rows, int &cols, vector< vector<string> > &result)
	{
		char **result_tmp;
		int ret;
		ret = get_table(query, rows, cols, result_tmp);
		if (ret) {
			return ret;
		}
		result.clear();
		for (int i = 0; i <= rows; ++i) {
			vector<string> tmp;
			result.push_back(tmp);
			for (int j = 0; j < cols; ++j) {
				result[i].push_back(result_tmp[i*cols+j]);
			}
		}
		return 0;
	}
	int get_table(const string &query)
	{
		int ret, rows, cols;
		vector< vector<string> > result;
		ret = get_table(query, rows, cols, result);
		if (ret) {
			return ret;
		}
		return 0;
	}
	int get_table_and_show(const string &query)
	{
		int ret, rows, cols;
		vector< vector<string> > result;
		ret = get_table(query, rows, cols, result);
		if (ret) {
			return ret;
		}
		DEBUGMSG("get_table_and_show " << rows << " " << cols)
		for (int i = 0; i < result.size(); ++i) {
			for (int j = 0; j < result[i].size(); ++j) {
				DEBUGMSGN(result[i][j] << " ")
			}
			DEBUGMSG("")
		}
		return 0;
	}
	int get_table(const string &query, int &rows, int &cols, char **&result)
	{
		char *errMsg = NULL;
		sqlite3_get_table(db, query.c_str(), &result, &rows, &cols, &errMsg);
		if (errMsg != NULL) {
			cmdFailed("get_table", query, errMsg);
			return 1;
		}
		return 0;
	}
	sqlite3 *getDB()
	{
		return db;
	}
private:
	sqlite3 *db;
	int registerTokenizer(
		sqlite3 *db,
		const char *zName,
		const sqlite3_tokenizer_module *p
	){
		int rc;
		sqlite3_stmt *pStmt;
		const char *zSql = "SELECT fts3_tokenizer(?, ?)";
		rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
		if( rc!=SQLITE_OK ){
			return rc;
		}
		sqlite3_bind_text(pStmt, 1, zName, -1, SQLITE_STATIC);
		sqlite3_bind_blob(pStmt, 2, &p, sizeof(p), SQLITE_STATIC);
		sqlite3_step(pStmt);
		return sqlite3_finalize(pStmt);
	}
	int registerCharacterTokenizer()
	{
		int ret;
		const string tokenName = "character";
		const sqlite3_tokenizer_module *ptr;
		get_character_tokenizer_module(&ptr);
		ret = registerTokenizer(db, tokenName.c_str(), ptr);
		if (ret) {
			DEBUGMSG("[SQLite3DB init error] can't register character tokenizer")
		}
		return ret;
	}
	inline void cmdFailed(const string &tag, const string &cmd, char *errMsg)
	{
		DEBUGMSG(">>> " << cmd)
		DEBUGMSG("[SQLite3DB " << tag << " failed] " << errMsg)
		sqlite3_free(errMsg);
	}

};

int main()
{
	SQLite3DBCommand PG;
	PG.open("example.db");
	PG.exec("asdf");
	PG.get_table_and_show("SELECT * from note-");
	PG.execf("sjaksdf %Q %Q %d", "asdf", "ffff", 1234);

}