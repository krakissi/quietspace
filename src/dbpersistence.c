#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>

#include <mysql/mysql.h>

#include "dbpersistence.h"

// Get a database connection handle. Must be closed with mysql_close().
MYSQL *get_conn(){
	MYSQL *mysql = mysql_init(NULL);

	if(!mysql_real_connect(mysql, "localhost", "quietspace", "quietspace", "quietspace", 3306, NULL, 0)){
		fprintf(stderr, "Failed to connect to database!!");

		return NULL;
	}

	return mysql;
}

void db_dump_users(FILE *stream){
	MYSQL *mysql = get_conn();

	// Error
	if(!mysql)
		return;

	const char *query = "SELECT id_player, name, nick, created, pass FROM players";

	if(!mysql_real_query(mysql, query, strlen(query))){
		MYSQL_RES *result = mysql_use_result(mysql);

		// Error
		if(!result)
			return;

		int fields = mysql_num_fields(result);
		MYSQL_ROW row;

		while((row = mysql_fetch_row(result))){
			for(int i = 0; i < fields; i++)
				fprintf(stream, "\"%2s\" ", row[i] ? row[i] : "NULL");

			fprintf(stream, "\r\n");
		}

		mysql_free_result(result);
	} else return; // Error

	mysql_close(mysql);
}
