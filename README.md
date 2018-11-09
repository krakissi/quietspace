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

Create tables the tables:

```
	CREATE TABLE players (
		id_player int(11) NOT NULL PRIMARY KEY,
		name varchar(8) UNIQUE NOT NULL,
		nick varchar(8) DEFAULT NULL,
		created timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
		pass varchar(64) NOT NULL
	);
```
