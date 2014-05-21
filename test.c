


int main()
{
	sqlite3 *db;
	const sqlite3_tokenizer_module *ptr;
	get_character_tokenizer_module(&ptr);
	registerTokenizer(db, "character", ptr);
	
	if (sqlite3_open_v2("example.db", &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, 0)) {
        return 0;
    } else {

    }
}