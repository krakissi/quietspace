Quiet Space
-----------

A multiplayer text adventure experience.


Setup
-----

Set up database user in MySQL/MariaDB:

```
	create user 'quietspace'@'localhost' identified by 'quietspace';
	grant all privileges on quietspace.* to 'quietspace'@'localhost' identified by 'quietspace';
```

Create the tables:

```
	CREATE TABLE players (
		id_player int(11) NOT NULL PRIMARY KEY,
		name varchar(8) UNIQUE NOT NULL,
		nick varchar(8) DEFAULT NULL,
		created timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
		pass varchar(64) NOT NULL
	);

	CREATE TABLE players_kv (
		id_player int(11) NOT NULL,
		name varchar(64) NOT NULL,
		value varchar(2048),

		CONSTRAINT id_name_unique UNIQUE (id_player, name),
		FOREIGN KEY (id_player)
			REFERENCES players(id_player)
			ON DELETE CASCADE
	);
```


Build
-----

The `Makefile` in `src/` will completely build both the game server and
asset encoding tool (just run `make`).

You can build the asset encoding tool itself with just `make encoder`.

For convenience, `make run` will build and the start (or restart) an
instance of the game server.


Assets
------

Game data is stored in asset files, which are base64 encoded. The
alphabet used for this encoding is *not* compatiable with standard
base64 implementations. Only the encoded version of the files is
committed to git. You can decode all of the game assets by running
`util/unpack.sh`.  This produces a directory `assets/out/`.

To modify assets, copy the `assets/out/` directory to `assets/from/`
(e.g. `mkdir assets/from; cp -r assets/out/* assets/from/`). Make your
changes, then repack the assets with `util/pack.sh` (or `make pack` in
`src/`).

Assets are compiled into the game server, so it is necessary to `make`
after packing new asset changes.


Networking
----------

By default the game will run on port 10421. If you want to connect via
the standard telnet port (or any other port), you could configure
iptables routing:

```
	iptables -t nat -A PREROUTING -i eth0 -p tcp --dport 23 -j REDIRECT --to-port 10421
```

This requires root access to the linux system where you are running the
server.
