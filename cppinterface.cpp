#include "sqlite3.h"
#ifdef __cplusplus
extern "C" {
#endif 
#include "character_tokenizer.h"
#ifdef __cplusplus
}
#endif
#define DEBUGMSGRED(msg) cout << "\033[0;31m" << msg << "\033[0m" << endl;
#define DEBUGMSGGREEN(msg) cout << "\033[0;32m" << msg << "\033[0m" << endl;
#define DEBUGMSGBROWN(msg) cout << "\033[0;33m" << msg << "\033[0m" << endl;
#define DEBUGMSGCYAN(msg) cout << "\033[0;36m" << msg << "\033[0m" << endl;
#define DEBUGMSG(msg) cout << msg << endl;
#define DEBUGMSGN(msg) cout << msg;
#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
using namespace std;
//int db;

class SQLite3Command
{
public:
	SQLite3Command()
	{
		db = NULL;

	}
	~SQLite3Command()
	{
		sqlite3_free(db);
	}
	struct EXEC_RESULT
	{
		vector<string> colsName;
		vector< vector<string> > records;
	};
	inline static void show_table(const vector< vector<string> > &data)
	{
		for (int i = 0; i < data.size(); i++) {
			for (int j = 0; j < data[i].size(); j++) {
				DEBUGMSGN(data[i][j] << "\t\033[0;33m|\033[0m")
			}
			DEBUGMSG("");
		}
	}
	int open(const string &path)
	{
		return open(path, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
	}
	int open(const string &path, int flag, const char *szVfs)
	{
		int ret;
		ret = sqlite3_open_v2(path.c_str(), &db, flag, szVfs);
		if (ret != SQLITE_OK) {
			DEBUGMSGRED("[SQLite3DB open error] @ sqlite3_open_v2")
			goto END;
		}
		ret = registerCharacterTokenizer();
		if (ret != SQLITE_OK) {
			DEBUGMSGRED("[SQLite3DB open error] @ registerCharacterTokenizer")
			goto END;
		}
		END:
		return ret;
	}
	int exec(const string &cmd)
	{
		return exec(cmd, 0, 0);
	}
	static int _execDefaultCallback(void *pData, int argc, char **cols, char **colsName)
	{
		EXEC_RESULT *result = (EXEC_RESULT*) pData;
		result->records.push_back(vector<string>());
		for (int i = 0; i < argc; i++) {
			int pos = result->records.size() - 1;
			if (pos == 0) {
				result->colsName.push_back(colsName[i]);
			}
			result->records[pos].push_back(cols[i]);
		}
		return 0;
	}
	int exec(const string &cmd, vector< vector<string> > &data)
	{
		int ret;
		EXEC_RESULT result;
		char *szErrMsg = NULL;
		
 		ret = sqlite3_exec(db, cmd.c_str(), _execDefaultCallback, &result, &szErrMsg);
 		if (ret != SQLITE_OK) {
 			cmdFailed("exec", cmd, szErrMsg);
 			goto END;
 		}

 		data.clear();
 		// use '<=' instead of '<' for colsName 
 		for (int i = 0; i <= result.records.size(); i++) {
 			data.push_back(vector<string>());
 		}
 		for (int i = 0; i < result.colsName.size(); i++) {
 			data[0].push_back(result.colsName[i]);
 		}
 		for (int i = 0; i < result.records.size(); i++) {
 			for (int j = 0; j < result.records[i].size(); j++) {
 				data[i+1].push_back(result.records[i][j]);
 			}
 		}
 		END:
 		return ret;
	}
	int exec_and_show(const string &cmd)
	{
		int ret;
		vector< vector<string> > data;
		ret = exec(cmd, data);
		if (ret != SQLITE_OK) {
			goto END;
		}
		DEBUGMSG("exec_and_show")
		show_table(data);
		END:
 		return ret;
	}
	int exec(const string &cmd, int (*callback)(void*,int,char**,char**), void* pData)
	{
		int ret;
		char *szErrMsg = NULL;
		ret = sqlite3_exec(db, cmd.c_str(), callback, pData, &szErrMsg);
		if (szErrMsg != NULL) {
			cmdFailed("exec", cmd, szErrMsg);
			goto END;
		}
		END:
		return ret;
	}
	/*
		execf will not use reference.
		because 'va_start' has undefined behavior with reference types.
	*/
	int execf(vector< vector<string> > *pData, const string cmd, ...) 
	{
		int ret;
		va_list argv;
		char *szSQL;
		va_start(argv, cmd);
		szSQL = sqlite3_vmprintf(cmd.c_str(), argv);
		ret = exec(string(szSQL), *pData);
		sqlite3_free(szSQL);
		return ret;
	}
	/*
		execf without vector< vector<string> > data
	*/
	int execf_cmd_only(const string cmd, ...)
	{
		int ret;
		va_list argv;
		char *szSQL;
		va_start(argv, cmd);
		szSQL = sqlite3_vmprintf(cmd.c_str(), argv);
		ret = exec(string(szSQL));
		sqlite3_free(szSQL);
		return ret;
	}
	
	int get_table(const string &query, int &rows, int &cols, vector< vector<string> > &result)
	{
		char **result_tmp;
		int ret;
		ret = get_table(query, rows, cols, result_tmp);
		if (ret != SQLITE_OK) {
			goto END;
		}
		result.clear();
		for (int i = 0; i <= rows; ++i) {
			vector<string> tmp;
			result.push_back(tmp);
			for (int j = 0; j < cols; ++j) {
				result[i].push_back(result_tmp[i*cols+j]);
			}
		}
		END:
		return ret;
	}
	int get_table(const string &query, vector< vector<string> > &result)
	{
		int ret, rows, cols;
		ret = get_table(query, rows, cols, result);
		if (ret != SQLITE_OK) {
			goto END;
		}
		END:
		return ret;
	}
	int get_table_and_show(const string &query)
	{
		int ret, rows, cols;
		vector< vector<string> > result;
		ret = get_table(query, rows, cols, result);
		if (ret != SQLITE_OK) {
			goto END;
		}
		DEBUGMSG("get_table_and_show " << rows << " " << cols)
		show_table(result);
		END:
		return ret;
	}
	int get_table(const string &query, int &rows, int &cols, char **&result)
	{
		int ret;
		char *szErrMsg = NULL;
		ret = sqlite3_get_table(db, query.c_str(), &result, &rows, &cols, &szErrMsg);
		if (ret != SQLITE_OK) {
			cmdFailed("get_table", query, szErrMsg);
			goto END;
		}
		END:
		return ret;
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
		if (ret != SQLITE_OK) {
			DEBUGMSGRED("[SQLite3DB init error] can't register character tokenizer")
		}
		return ret;
	}
	inline void cmdFailed(const string &tag, const string &cmd, char *szErrMsg)
	{
		DEBUGMSGBROWN(">>> " << cmd)
		DEBUGMSGRED("[SQLite3DB " << tag << " failed] " << szErrMsg)
		sqlite3_free(szErrMsg);
	}
};

int main()
{
	SQLite3Command PG;
	SQLite3Command *ptr = new SQLite3Command();
	delete ptr;
	vector< vector<string> > tmp;
	PG.open("example.db");
	PG.exec("asdf");
	PG.get_table_and_show("SELECT * from note");
	PG.execf_cmd_only("sjaksdf %Q %Q %d", "asdf", "ffff", 1234);
	DEBUGMSGGREEN("=== result of INSERT")
	PG.execf(&tmp, "INSERT INTO note VALUES(%Q, %Q);", "Insert Test", "Insert Content");
	PG.show_table(tmp);
	DEBUGMSGGREEN("=== result of SELECT")
	PG.execf(&tmp, "SELECT * from note");
	PG.show_table(tmp);
	//PG.exec_and_show("select * from note");
	DEBUGMSGRED("Red test")
	DEBUGMSGGREEN("Green test")
	DEBUGMSGBROWN("Brown test")
	DEBUGMSGCYAN("Cyan test")
}