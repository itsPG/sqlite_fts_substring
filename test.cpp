#include "sqlite3.h"
#ifdef __cplusplus
extern "C" {
#endif 
#include "character_tokenizer.h"
#ifdef __cplusplus
}
#endif 

#include <iostream>
#include <string>
#include <vector>
using namespace std;

// char *zName is modified to const char *zName by PG Tsai
static int registerTokenizer(
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
/*
	This function combined some steps that is required for registing a tokenizer.
	Chech original DEMO for more information (if you want to do it manually).
*/
static int registerCharacterTokenizer(sqlite3 *db)
{
	const string tokenName = "character";
	const sqlite3_tokenizer_module *ptr;
	get_character_tokenizer_module(&ptr);
	return registerTokenizer(db, tokenName.c_str(), ptr);

}
static int execQuery(sqlite3 *db, const string &cmd, const string &debug_msg)
{
	char *errMsg = NULL;
	sqlite3_exec(db, cmd.c_str(), 0, 0, &errMsg);

	if (errMsg != NULL) {
		cout << "[execQuery failed] at " << debug_msg << " : " << endl;
		cout << errMsg << endl;  
		free(errMsg);
		return 1;
	}
	return 0;
}
static int execQuery(sqlite3 *db, const string &cmd)
{
	return execQuery(db, cmd, "");
}

class TestingDB
{
public:
	int init(const string &_path)
	{
		this->path = _path;
		if (sqlite3_open_v2("example.db", &(this->db), SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL)) {
			return -1;
		} else {
			cout << "success" << endl;
		}
		if (registerCharacterTokenizer(this->db)) {
			return -1;
		}
		return 0;
		
	}
	int insertData(const string &title, const string &text)
	{
		char *szSQL = sqlite3_mprintf(
			"INSERT INTO note VALUES(%Q, %Q);",
			title.c_str(),
			text.c_str()
		);
		int ret = execQuery(this->db, szSQL);
		sqlite3_free(szSQL);
		return ret;
	}
	void queryAll()
	{
		getTable("SELECT * FROM note");
	}
	void FTS(const string &query)
	{
		char *szSQL = sqlite3_mprintf(
			"SELECT title FROM note WHERE note MATCH %Q",
			query.c_str()
		);
		cout << "===> FTS ===>" << endl;
		int ret = getTable(szSQL);
		cout << "<=== FTS <===" << endl;
	}
	int getTable(const string &query)
	{
		int rows, cols;
		char **result;
		return getTable(query, rows, cols, result);
	}
	int getTable(const string &query, int &rows, int &cols, char **&result)
	{
		char *errMsg = NULL;
		sqlite3_get_table(this->db, query.c_str(), &result, &rows, &cols, &errMsg);
		if (errMsg != NULL) {
			cout << "getTable Error" << endl;
			cout << errMsg << endl;
			free(errMsg);
			return -1;
		}
		cout << "rows: " << rows << " / cols: " << cols << endl;
		for (int i = 0; i <= rows; i++) {
			for (int j = 0; j < cols; j++) {
				cout << result[i*cols+j] << "\t";
			}
			cout << endl;
		}
		return 0;
	}
	int createTable()
	{
		string cmd =
		"CREATE VIRTUAL TABLE note USING fts3(title TEXT, content TEXT, tokenize=character);";
		return execQuery(this->db, cmd);
	}
	int dropTable()
	{
		string cmd =
		"DROP TABLE note;";
		return execQuery(this->db, cmd);
	}
	
private:
	sqlite3 *db;
	string path;
};


int main()
{
	TestingDB PG;
	PG.init("example.db");
	PG.dropTable();
	PG.createTable();
	PG.insertData("asdf", "asdfasdfasdf");
	PG.insertData("test", "asdfasdfasdf chi 測試");
	PG.insertData("中文測試", "中文測試 This is chinese test.");
	PG.queryAll();
	PG.FTS("測");
	return 0;
}