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
class SQLite3Command
{
public:
	SQLite3Command();
	~SQLite3Command();
	struct EXEC_RESULT
	{
		vector<string> colsName;
		vector< vector<string> > records;
	};
	int open(const string &path);
	int open(const string &path, int flag, const char *szVfs);
	int exec(const string &cmd);
	int exec(const string &cmd, vector< vector<string> > &data);
	int exec_and_show(const string &cmd);
	int exec(const string &cmd, int (*callback)(void*,int,char**,char**), void* pData);
	int execf(vector< vector<string> > *pData, const string cmd, ...);
	int execf_cmd_only(const string cmd, ...);
	int get_table(const string &query, int &rows, int &cols, vector< vector<string> > &result);
	int get_table(const string &query, vector< vector<string> > &result);
	int get_table_and_show(const string &query);
	int get_table(const string &query, int &rows, int &cols, char **&result);
	sqlite3 *getDB();

	inline static void show_table(const vector< vector<string> > &data)
	{
		for (int i = 0; i < data.size(); i++) {
			for (int j = 0; j < data[i].size(); j++) {
				DEBUGMSGN(data[i][j] << "\t\033[0;33m|\033[0m")
			}
			DEBUGMSG("");
		}
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

private:
	sqlite3 *db;
	int registerTokenizer(
		sqlite3 *db,
		const char *zName,
		const sqlite3_tokenizer_module *p
	);
	int registerCharacterTokenizer();
	inline void cmdFailed(const string &tag, const string &cmd, char *szErrMsg);
};