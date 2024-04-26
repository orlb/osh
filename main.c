// osh
// Shell for Linux systems, written in C
//
// Orlandis Brown
// CS321
// April 26 2024

#include "shell.h"

int main ( int argc, char ** argv ) {
    int status;
    char * line;
    do {
        line = shell_get_line();
        status = shell_execute_commands(line);
        free(line);
    }
    while ( status != __command_quit );
}
