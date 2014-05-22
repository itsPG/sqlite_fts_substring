#include "sqlite3command.h"
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