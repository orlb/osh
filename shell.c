#include "shell.h"
#include <unistd.h>

struct _Token_List {
    // invariant: last element of tokens is NULL
    char ** tokens; size_t size;
};

static _Token_List * _get_token_list (const char * string, const char * delimiter) {
    // from string, get list of strings separated by delimiter
    _Token_List * token_list = malloc(sizeof(_Token_List));
    if ( string == NULL ) {
        token_list->size = 0;
        token_list->tokens = NULL;
        return token_list;
    }
    token_list->size = __TL_size_base;
    token_list->tokens = malloc(token_list->size * sizeof(char *));

    char * _string_copy, * _token, * _state;
    _string_copy = strdup(string);
    size_t _position = 0;
    for ( _token = strtok_r(_string_copy, delimiter, &_state); _token; _token = strtok_r(NULL, delimiter, &_state)) {
        token_list->tokens[_position] = strdup(_token);
        _position ++;
        if ( _position >= token_list->size ) {
            token_list->size += __TL_size_base;
            token_list->tokens = realloc(token_list->tokens, token_list->size * sizeof(char *));
            if ( ! token_list ) {
                perror(__message_prefix);
                free(_string_copy);
                free(_token);
                token_list->size = 0;
                token_list->tokens = NULL;
                return token_list;
            }
        }
    }
    free(_string_copy);
    free(_token);
    token_list->tokens[_position] = NULL;
    return token_list;
}

static void _destroy_token_list (_Token_List * token_list) {
    for ( int n = 0; n < token_list->size; n++ ) {
        free(token_list->tokens[n]);
    }
    free(token_list->tokens);
    free(token_list);
}

static const char _get_command_operator (const char * command, const char * token_group, const size_t position) {
    // get operator, i.e. detect & symbol
    // https://stackoverflow.com/questions/12460264/c-determining-which-delimiter-used-strtok
    if ( command == NULL ) {
        return ';';
    }
    else {
        char operator = ';';
        char * _token_group_copy = strdup(token_group);
        char * _string_copy = strdup(token_group);
        char * _token = strtok(_token_group_copy, __command_delim);
        size_t _position = 0;
        while ( _token ) {
            if ( _position == position ) {
                operator = _string_copy[_token - _token_group_copy + strlen(_token)];
                break;
            }
            _position++;
            _token = strtok( NULL, __command_delim );
        }
        free(_token_group_copy);
        free(_string_copy);
        if ( operator == '&' ) {
            return '&';
        }
        else {
            return ';';
        }
    }
}

static const int _execute_command (const char * command, const char operator) {
    // get command args and run builtin commands
    // otherwise execute in foreground or background
    // depending on operator
    if ( command == NULL ) {
        return __command_success;
    }
    _Token_List * _command_tl = _get_token_list(command, __args_delim);
    char * const * _args = _command_tl->tokens;
    const char * _program = _command_tl->tokens[0];

    if ( strcmp(_program, "exit") == 0 ) {
        return __command_quit;
    }
    else if ( strcmp(_program, "help") == 0 ) {
        printf(__help_message);
        return __command_success;
    }
    else if ( strcmp(_program, "cd") == 0 ) {
        if ( chdir(_args[1]) != 0) perror(__message_prefix);
    }
    else {
        int _status;
        pid_t _pid = fork();
        if ( _pid == 0 ) {
            // Child process
            if ( execvp(_program, _args) == - 1 ) {
                perror(__message_prefix);
            }
            exit(EXIT_FAILURE);
        }
        else if ( _pid < 0 ) {
            // Error forking
            perror(__message_prefix);
        }
        else {
            // Parent process
            if ( operator == ';' ) {
                do {
                    waitpid(_pid, &_status, WUNTRACED);
                } while ( ! WIFEXITED(_status) && ! WIFSIGNALED(_status) );
            }
            else if ( operator == '&' ) {
                return __command_success;
            }
        }
        _destroy_token_list(_command_tl);
        //return _status;
        return __command_success;
    }
}

char * shell_get_line () {
    char * line = malloc(100 * sizeof(char));
    size_t _size = 100;
    if ( fgets(line, _size, stdin) == NULL ) {
        // https://cboard.cprogramming.com/c-programming/65783-resetting-stdin-after-eof.html
        // reopen stdin when EOF received
        // Not sure how else to handle without exiting
        freopen("/dev/tty", "rw", stdin);
        printf("\n");
        return NULL;
    }
    while (*line != '\0') { // check if empty
        if ( ! isspace((unsigned char) *line) ) {
            // This if statement results in 'free(): Invalid pointer', even when the condition is false.
            // Really not sure why
            // if (  strlen(line) > 101 ) {
            //     fprintf(stderr,
            //         "%sCommand too long, must be shorter than 100 characters. Truncating.\n",
            //         __message_prefix);
            //     _size = 101; // 100 + null byte
            //     char * _line = malloc(_size * sizeof(char));
            //     strncpy(_line, line, _size);
            //     free(line);
            //     line = _line;
            // }
            return line;
        }
        else {
            line++;
        }
    }
    return NULL;
}

const int shell_execute_commands (const char * line) {
    // execute commands by getting command list
    // delimited by ;, then execute with operator (&, or nothing)
    if ( line == NULL ) {
        return __command_success;
    }
    _Token_List * token_groups = _get_token_list(line, __token_group_delim);
    for ( int n = 0; n < token_groups->size; n++ ) {
        const char * _group = token_groups->tokens[n];
        _Token_List * commands = _get_token_list(_group, __command_delim);
        for ( int m = 0; m < commands->size; m++ ) {
            const char * _command = commands->tokens[m];
            if ( _command != NULL ) {
                const char _operator = _get_command_operator(_command, _group, m);
                if ( _execute_command(_command, _operator) == __command_quit) {
                    return __command_quit;
                }
            }
        }
        _destroy_token_list(commands);
    }
    _destroy_token_list(token_groups);
    return __command_success;
}
