#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mysql/mysql.h>

#include "dbpersistence.h"

#define PLAYER_INSERT_SQL_START "INSERT INTO players(name,nick,pass) VALUES ("
#define PLAYER_INSERT_SQL_END ")"

#define PLAYER_REPLACE_SQL_START "REPLACE INTO players(name,nick,pass,pos_level,pos_id) VALUES ("
#define PLAYER_REPLACE_SQL_END ")"

#define PLAYER_SELECT_SQL_START "SELECT id_player, name, nick, created, pass, pos_level, pos_id FROM players WHERE "
#define PLAYER_SELECT_SQL_ID "id_player = "
#define PLAYER_SELECT_SQL_NAME "name = "

#define SQL_FUDGE 3

// Get a database connection handle. Must be closed with mysql_close().
MYSQL *get_conn(){
	MYSQL *mysql = mysql_init(NULL);

	if(!mysql_real_connect(mysql, "localhost", "quietspace", "quietspace", "quietspace", 3306, NULL, 0)){
		fprintf(stderr, "Failed to connect to database!!");

		return NULL;
	}

	return mysql;
}


// Debug: Dumps out the player object to the specified stream for viewing.
void debug_print_character(FILE *stream, player *pl){
	fprintf(
		stream,
		"%2ld: %s (%s) created=%ld pass=%s\r\n"
		"cell: %d%c - room: %s\r\n",
		pl->id_player,
		pl->name,
		pl->nick,
		pl->created,
		pl->pass,
		(pl->pos_level ^ QS_POS_AC_IND),
		((pl->pos_level & QS_POS_AC_IND) ? 'C' : 'A'),
		pl->pos_id
	);
}

// Create a new player record. Fails on constraint violation (duplicate name, etc.)
unsigned long player_create(player *pl){
	unsigned long ret = 0L;
	char *query = NULL;

	MYSQL *mysql = get_conn();
	if(!mysql)
		goto out;

	query = calloc(
		(
		 	strlen(PLAYER_INSERT_SQL_START) +
			QS_LEN_NAME + SQL_FUDGE + // Name
			QS_LEN_NAME + SQL_FUDGE + // Nick
			QS_LEN_PASS + SQL_FUDGE + // Pass
			strlen(PLAYER_INSERT_SQL_END)
		),
		sizeof(char)
	);
	sprintf(
		query,
		PLAYER_INSERT_SQL_START "'%s','%s','%s'" PLAYER_INSERT_SQL_END,
		pl->name,
		pl->nick,
		pl->pass
	);

	if(!mysql_real_query(mysql, query, strlen(query))){
		// Set the id_player value of the record if the returned record is greater than 0.
		if((ret = (unsigned long) mysql_insert_id(mysql)))
			pl->id_player = ret;
	}

out:
	mysql_close(mysql);
	free(query);
	return ret;
}

// Save an existing player. Their id_player value should be non-zero and exist in the database.
void player_persist(player *pl){
	char *query = NULL;

	MYSQL *mysql = get_conn();
	if(!mysql)
		goto out;

	query = calloc(
		(
			strlen(PLAYER_REPLACE_SQL_START) +
			QS_LEN_NAME + SQL_FUDGE + // Name
			QS_LEN_NAME + SQL_FUDGE + // Nick
			QS_LEN_PASS + SQL_FUDGE + // Pass
			5 + SQL_FUDGE + // pos_level
			QS_LEN_POS_ID + SQL_FUDGE + // pos_id
			strlen(PLAYER_REPLACE_SQL_END)
		),
		sizeof(char)
	);
	sprintf(
		query,
		PLAYER_REPLACE_SQL_START "'%s','%s','%s', %d, '%s'" PLAYER_REPLACE_SQL_END,
		pl->name,
		pl->nick,
		pl->pass,
		pl->pos_level,
		pl->pos_id
	);

	// Persist and hope for the best.
	mysql_real_query(mysql, query, strlen(query));

out:
	mysql_close(mysql);
	free(query);
}

// Load a player, either by name or id_player (if id_player > 0). If pl is NULL this allocates memory.
player *player_load(unsigned long id_player, const char name[9], player *pl){
	char *query = NULL;
	MYSQL *mysql = get_conn();
	if(!mysql)
		goto out;

	size_t len = strlen(PLAYER_SELECT_SQL_START);
	if(id_player)
		len += strlen(PLAYER_SELECT_SQL_ID) + 20;
	else
		len += strlen(PLAYER_SELECT_SQL_NAME) + QS_LEN_NAME + SQL_FUDGE;
	query = calloc(len, sizeof(char));

	if(id_player)
		sprintf(query, PLAYER_SELECT_SQL_START PLAYER_SELECT_SQL_ID "%ld", id_player);
	else
		sprintf(query, PLAYER_SELECT_SQL_START PLAYER_SELECT_SQL_NAME "'%s'", name);

	if(!mysql_real_query(mysql, query, strlen(query))){
		MYSQL_RES *result = mysql_use_result(mysql);
		if(!result)
			goto out;

		MYSQL_ROW row = mysql_fetch_row(result);
		if(!row)
			goto out;

		// Allocate memory for the player object if it wasn't provided.
		if(!pl)
			pl = malloc(sizeof(player));

		pl->id_player = atol(row[0]);
		strcpy(pl->name, row[1]);
		strcpy(pl->nick, (row[2] ? row[2] : row[1]));

		// FIXME convert row[3] timestamp to long val.
		pl->created = 0;

		strcpy(pl->pass, row[4]);

		pl->pos_level = atoi(row[5]);
		strcpy(pl->pos_id, row[6]);

		mysql_free_result(result);
	}

out:
	mysql_close(mysql);
	free(query);
	return pl;
}
