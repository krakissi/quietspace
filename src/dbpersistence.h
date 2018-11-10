/*
	dbpersistence
	mperron (2018)

	Load/persist database objects for Quiet Space.
*/

#ifndef QS_DBPERSISTENCE_H
#define QS_DBPERSISTENCE_H

#define QS_LEN_NAME 9
#define QS_LEN_PASS 65

typedef struct player {
	unsigned long id_player;
	long created;

	char name[QS_LEN_NAME], nick[QS_LEN_NAME], pass[QS_LEN_PASS];
} player;

typedef struct listlink {
	void *el;
	struct listlink *next;
} *linkedlist, listlink;

// Debug functions
extern void debug_print_character(FILE *stream, player *pl);

// Player persistence
extern unsigned long player_create(player *pl);
extern void player_persist(player *pl);
extern player *player_load(unsigned long id_player, char name[9], player *pl);

#endif
