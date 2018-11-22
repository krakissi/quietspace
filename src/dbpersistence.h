/*
	dbpersistence
	mperron (2018)

	Load/persist database objects for Quiet Space.
*/

#ifndef QS_DBPERSISTENCE_H
#define QS_DBPERSISTENCE_H

#define QS_LEN_NAME 9
#define QS_LEN_PASS 257
#define QS_LEN_POS_ID 64

// pos_level is a cell number & QS_POS_AC_IND (indicating anode or cathode)
#define QS_POS_AC_IND 0b100

typedef struct player {
	// Database-assigned values
	unsigned long id_player;
	long created;

	// Name and password
	char name[QS_LEN_NAME], nick[QS_LEN_NAME], pass[QS_LEN_PASS];

	// Position information
	int pos_level;
	char pos_id[QS_LEN_POS_ID];
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
extern player *player_load(unsigned long id_player, const char name[9], player *pl);

#endif
