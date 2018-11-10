/*
	dbpersistence
	mperron (2018)

	Load/persist database objects for Quiet Space.
*/

#ifndef QS_DBPERSISTENCE_H
#define QS_DBPERSISTENCE_H

typedef struct player {
	long id_player;
	long created;

	char name[9], nick[9], pass[65];
} player;

typedef struct listlink {
	void *el;
	struct listlink *next;
} *linkedlist, listlink;

// FIXME debug
extern void db_dump_users(FILE *stream);
extern void debug_print_character(FILE *stream, player *pl);

#endif
