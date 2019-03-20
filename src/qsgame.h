#include "loader.h"

asset *asset_tree;

void draw_scene(FILE *stream, asset *as, char *key){
	asset_kv *kv = kv_tree_find(as->kv_tree, key);

	fprintf(stream,
		"\033[2J\033c" /* Reset and clear display */
	);

	if(kv && ((kv->type == AVT_SCENE) || (kv->type == AVT_SCENE_OVERLAY)))
		fprintf(stream, "\033[0;0H%s", kv->value.str);
	else
		fprintf(stream, "failed to find kv!!");

	draw_borders(stream, 0, 1, 80, 15);
}

void draw_overlay(FILE *stream, asset *as, char *key){
	asset_kv *kv = kv_tree_find(as->kv_tree, key);

	if(kv && ((kv->type == AVT_SCENE_OVERLAY) || (kv->type == AVT_SCENE))){
		char *graphics = kv->value.str;

		fputs("\033[0;0H", stream);
		while(*graphics){
			if(*graphics == ' ')
				fputs("\033[C", stream);
			else
				fputc(*graphics, stream);

			graphics++;
		}
	}
}

// Entry point for the actual game, once a player is logged in.
void game_start(FILE *stream, player *pl){
	char *str = NULL;
	ssize_t rd;
	size_t n;

	// Load all assets
	asset_tree = asset_load();

	asset *scene = asset_tree_find(asset_tree, "lift_doors");

	draw_scene(stream, scene, "scene_main");

	while((rd = read_cmd(&str, &n, stream, "", "$", "")) != -1){
		// Exit immediately.
		if(!strcmp(str, CMD_QUIT) || !strcmp(str, CMD_QUIT_ALT1))
			break;

		lower_str(str);

		// FIXME debug
		if(!strcmp(str, "lift"))
			scene = asset_tree_find(asset_tree, "lift_doors");
		else if(!strcmp(str, "dorm"))
			scene = asset_tree_find(asset_tree, "dorm");

		draw_scene(stream, scene, "scene_main");

		if(!strcmp(str, "overlay")){
			if(!strcmp(scene->name, "dorm"))
				draw_overlay(stream, scene, "scene_main_overlay");
		}

		cursor_position_response(stream);
		text_type(stream, ": %s", str);
	}
}

// Login command from the main menu.
player *game_login(FILE *stream){
	player *pl = NULL;

	const char *ps_perm = "$";
	char ps[16], ps_col[8];
	char *str = NULL, *hash = NULL;
	size_t n;

	char name[QS_LEN_NAME], pass[QS_LEN_PASS];

	regex_t regex_name, regex_pass;
	regcomp(&regex_name, REG_NAME, 0);
	regcomp(&regex_pass, REG_PASS, 0);

	*ps_col = 0;
	strcpy(ps, "login");

	// Request player username
	if(read_cmd_bounded(
		&str, &n, stream,
		ps, ps_perm, ps_col,
		GAME_LOGIN_BASE, GAME_LOGIN_NAMEFAIL GAME_LOGIN_BASE,
		&regex_name
	) == -1)
		goto fail;

	// Name's are always lowercase in the database.
	strcpy(name, str);
	lower_str(name);

	// Request player password
	strcpy(ps_col, "\033[8;30m");
	strcat(ps, "/pass");
	if(read_cmd_bounded(
		&str, &n, stream,
		ps, ps_perm, ps_col,
		"Enter your password.", GAME_PASS_SIZEMSG,
		&regex_pass
	) == -1)
		goto fail;

	// Compute the hash of the provided password.
	if(!(hash = hash_compute(stream, str)))
		goto fail;
	strcpy(pass, hash);

	// Retrieve player.
	pl = player_load(0, name, NULL);
	if(pl == NULL)
		goto fail;

	// Compare password hashes.
	if(strcmp(pass, pl->pass))
		goto fail;

	cursor_position_response(stream);
	text_type(
		stream,
		"Authentication successful. Welcome back \033[0;31m%s\033[0m!\r\n"
		"\r\n"
		"..........",
		pl->nick
	);

out:
	free(str);
	free(hash);
	return pl;
fail:
	cursor_position_response(stream);
	text_type(stream, "Failed to authenticate. You may be using the wrong name or password.");

	free(pl);
	pl = NULL;
	goto out;
}

// Join command from the main menu.
player *game_join(FILE *stream){
	player *pl = malloc(sizeof(player)), *pl_existing;

	const char *prompt_choose = GAME_JOIN_CHOOSE GAME_JOIN_BASE;
	const char *prompt_toolong = GAME_JOIN_TOOLONG GAME_JOIN_BASE;
	const char *prompt_pass = "Ok \033[0;31m%s\033[0m, what's your password?";
	const char *ps_perm = "$";
	char ps[16], ps_col[8];
	char *str = NULL, *a = NULL, *hash = NULL;
	size_t n;

	// Rules defining valid names and passwords.
	regex_t regex_name, regex_pass;
	regcomp(&regex_name, REG_NAME, 0);
	regcomp(&regex_pass, REG_PASS, 0);

	*ps_col = 0;
	strcpy(ps, "join");

	// Ask the player for a name.
	if(read_cmd_bounded(
		&str, &n, stream,
		ps, ps_perm, ps_col,
		prompt_choose, prompt_toolong,
		&regex_name
	) == -1)
		goto fail;

	strcpy(pl->nick, str);
	strcpy(pl->name, str);
	lower_str(pl->name);

	// Verify that the player's name isn't already in use.
	pl_existing = player_load(0, pl->name, NULL);
	if(pl_existing){
		free(pl_existing);

		cursor_position_response(stream);
		text_type(stream, "A player with that name already exists. Try \033[0;31mlogin\033[0m?");
		goto fail;
	}

	// Prepare the password prompt with the user's nickname.
	a = calloc(strlen(prompt_pass) + QS_LEN_NAME, sizeof(char));
	sprintf(a, prompt_pass, pl->nick);

	// Ask the player to enter their password.
	strcpy(ps_col, "\033[8;30m");
	strcat(ps, "/pass");
	if(read_cmd_bounded(
		&str, &n, stream,
		ps, ps_perm, ps_col,
		a, GAME_PASS_SIZEMSG,
		&regex_pass
	) == -1)
		goto fail;

	strcpy(pl->pass, str);

	// Ask the player to re-type their password.
	if(read_cmd_bounded(
		&str, &n, stream,
		ps, ps_perm, ps_col,
		"Enter your password again to confirm.", NULL,
		NULL
	) == -1)
		goto fail;

	// Password mismatch.
	if(strcmp(pl->pass, str)){
		cursor_position_response(stream);
		text_type(stream, "Your password didn't match! Failed to create player.");
		goto fail;
	}

	// Compute the hash of the player's password and store it instead.
	if(!(hash = hash_compute(stream, pl->pass))){
		cursor_position_response(stream);
		text_type(stream, "An unknown error occurred.");
		goto fail;
	}
	strcpy(pl->pass, hash);

	// Create the player in the database. This should return a non-zero player ID.
	if(!player_create(pl)){
		cursor_position_response(stream);
		text_type(stream, "An unknown error occurred.");
		goto fail;
	}

	// Load the player object so their created time is populated.
	player_load(pl->id_player, NULL, pl);

	cursor_position_response(stream);
	text_type(
		stream,
		"Welcome to Quiet Space \033[0;31m%s\033[0m!\r\n"
		"\r\n"
		"..........",
		pl->nick
	);

out:
	free(a);
	free(hash);
	return pl;
fail:
	free(pl);
	pl = NULL;
	goto out;
}

