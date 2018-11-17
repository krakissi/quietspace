/*
	Various constant definitions for Quiet Space
*/

// Command strings

#define CMD_QUIT "quit"
#define CMD_QUIT_ALT1 "q"

#define CMD_LOGIN "login"
#define CMD_JOIN "join"

// Regular expressions
#define REG_NAME "^[A-Za-z]\\{1,8\\}$"
#define REG_PASS "^.\\{4,64\\}$"

// Message strings

#define GAME_PASS_SIZEMSG "Your password should be at least five, but not more than 64 characters."

#define GAME_JOIN_CHOOSE "Choose a name."
#define GAME_JOIN_TOOLONG "That name is invalid."
#define GAME_JOIN_BASE " It may be no more than eight characters long and should consist only of letters ([A-Z][a-z])."

#define GAME_LOGIN_BASE "Who are you?"
#define GAME_LOGIN_NAMEFAIL "That doesn't look right. "
