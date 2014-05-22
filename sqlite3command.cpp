#include "sqlite3command.h"
using namespace std;

SQLite3Command::SQLite3Command()
{
	db = NULL;
}
SQLite3Command::~SQLite3Command()
{
	sqlite3_free(db);
}
int SQLite3Command::open(const string &path)
{
	return open(path, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
}
int SQLite3Command::open(const string &path, int flag, const char *szVfs)
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
int SQLite3Command::exec(const string &cmd)
{
	return exec(cmd, 0, 0);
}

int SQLite3Command::exec(const string &cmd, vector< vector<string> > &data)
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
int SQLite3Command::exec_and_show(const string &cmd)
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
int SQLite3Command::exec(const string &cmd, int (*callback)(void*,int,char**,char**), void* pData)
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
int SQLite3Command::execf(vector< vector<string> > *pData, const string cmd, ...) 
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
int SQLite3Command::execf_cmd_only(const string cmd, ...)
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

int SQLite3Command::get_table(const string &query, int &rows, int &cols, vector< vector<string> > &result)
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
int SQLite3Command::get_table(const string &query, vector< vector<string> > &result)
{
	int ret, rows, cols;
	ret = get_table(query, rows, cols, result);
	if (ret != SQLITE_OK) {
		goto END;
	}
	END:
	return ret;
}
int SQLite3Command::get_table_and_show(const string &query)
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
int SQLite3Command::get_table(const string &query, int &rows, int &cols, char **&result)
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
sqlite3 *SQLite3Command::getDB()
{
	return db;
}

int SQLite3Command::registerTokenizer(
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
int SQLite3Command::registerCharacterTokenizer()
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
inline void SQLite3Command::cmdFailed(const string &tag, const string &cmd, char *szErrMsg)
{
	DEBUGMSGBROWN(">>> " << cmd)
	DEBUGMSGRED("[SQLite3DB " << tag << " failed] " << szErrMsg)
	sqlite3_free(szErrMsg);
}