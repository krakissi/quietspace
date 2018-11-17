player *game_login(FILE *stream){
	player *pl = NULL;

	// Request player username
	// TODO

	// Request player password
	// TODO

	// Retrieve player and verify password.
	// TODO

	return pl;
}

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
	for(a = str; *a; a++)
		*a = tolower(*a);
	strcpy(pl->name, str);
	a = NULL;

	// Verify that the player's name isn't already in use.
	pl_existing = player_load(0, pl->name, NULL);
	if(pl_existing){
		free(pl_existing);

		cursor_position_response(stream);
		text_type(stream, "A player with that name already exists. Try \033[0;31mlogin\033[0m?");
		goto fail;
	}

	a = calloc(strlen(prompt_pass) + QS_LEN_NAME, sizeof(char));
	sprintf(a, prompt_pass, pl->nick);
	strcpy(ps_col, "\033[8m");
	strcat(ps, "/pass");

	// Ask the player to enter their password.
	if(read_cmd_bounded(
		&str, &n, stream,
		ps, ps_perm, ps_col,
		a, "Your password should be at least five, but not more than 64 characters.",
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
	text_type(stream, "Welcome to Quiet Space \033[0;31m%s\033[0m!", pl->nick);

out:
	free(a);
	free(hash);
	return pl;
fail:
	free(pl);
	pl = NULL;
	goto out;
}

