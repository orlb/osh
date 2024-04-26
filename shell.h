#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h> // waitpid
#include <unistd.h> // size_t, chdir, getcwd
#include <string.h> // strtok, printf, strdup, strcmp, strcpy
#include <ctype.h>

#define __message_prefix    "-osh: "
#define __help_message      \
    "osh, Orlandis Brown 2024\n" \
    "These shell commands are defined internally.\n" \
    "\n" \
    "help       - Show this list\n" \
    "cd [dir]   - Change working directory to dir\n" \
    "exit       - Leave the shell\n"

#define __token_group_delim ";"
#define __command_delim     "&"
#define __args_delim        " \t\r\n\a"

#define __TL_size_base      16
#define __line_size_max     100

#define __command_success   1
#define __command_fail      0
#define __command_quit      -1

struct _Token_List;
typedef struct _Token_List _Token_List;

char * shell_get_line ();
const int shell_execute_commands (const char * line);
