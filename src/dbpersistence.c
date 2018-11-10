#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
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

void debug_print_character(FILE *stream, player *pl){
	fprintf(stream, "%2ld: %s (%s) created=%ld pass=%s\r\n",
		pl->id_player,
		pl->name,
		pl->nick,
		pl->created,
		pl->pass
	);
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

		mysql_num_fields(result);
		MYSQL_ROW row;

		linkedlist players = malloc(sizeof(listlink)), players_it = players;
		players->next = NULL;
		players->el = NULL;
		player *pl;

		while((row = mysql_fetch_row(result))){
			if(!players_it->el)
				players_it->el = malloc(sizeof(player));
			pl = (player*) players_it->el;

			pl->id_player = atol(row[0]);
			strcpy(pl->name, row[1]);
			strcpy(pl->nick, (row[2] ? row[2] : row[1]));

			// FIXME convert row[3] timestamp to long val.
			pl->created = 0;

			strcpy(pl->pass, row[4]);

			players_it = players_it->next = malloc(sizeof(listlink));
			players_it->next = players_it->el = NULL;
		}

		mysql_free_result(result);

		players_it = players;
		while(players_it->el){
			pl = (player*) players_it->el;
			debug_print_character(stream, pl);

			if(players_it->next)
				players_it = players_it->next;
			else break;
		}
		fprintf(stream, "\r\n");

		players_it = players;
		while(players_it->el){
			free(players_it->el);

			if(players_it->next){
				players = players_it;
				players_it = players_it->next;

				free(players);
			}
		}
		if(players_it)
			free(players_it);

	} else return; // Error

	mysql_close(mysql);
}
