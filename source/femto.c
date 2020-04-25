#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>

#include <terminal.h>
#include <screen.h>
#include <helper.h>

#ifndef BUILD_DATE
#define BUILD_DATE "fix your makefile"
#endif

#ifndef BUILD_VERSION
#define BUILD_VERSION "fix your makefile"
#endif

int main(int argc, char *argv[])
{

    char *filename = NULL;

    /* Argument parsing */
    if( argc > 1 )
    {
        if( strcmp( argv[1], "-h") == 0 ||
            strcmp( argv[1], "--help") == 0)
        {
            printf("Text-Editor\n\n"
                   "Usage:\n"
                   "femto                    New file\n"
                   "femto [file]             Open file\n"
                   "femto -h | --help        Show help\n"
                   "femto -v | --version     Show version\n"
            );
            return EXIT_SUCCESS;
        }

        if( strcmp( argv[1], "-v" ) == 0 ||
            strcmp( argv[1], "--version" ) == 0)
        {
            printf( "BuildDate: %s\n", BUILD_DATE );
            printf( "BuildVersion: %s\n", BUILD_VERSION );
            return EXIT_SUCCESS;
        }

        /* Argument has to be the filename */
        filename = strdup(argv[1]);
        if( ! filename ) {
            lprintf(LOG_ERROR, "Error allocating memory for filename");
        }
    }


    lopen( "femto.log", LOG_DEBUG );

    /* Create session with delivered file */
    Session *session= fe_init_session( filename );

    /* 
     * Set terminal mode to raw in order to disable 
     * unwanted behaviour
     */
    fe_enable_raw_mode();

    /*
     * CYCLING EXECUTIVE: Read user input, interpret them, render buffer 
     */
    int c;
    int exit_femto = 0;
    while( ! exit_femto )
    {
        /* Render the current buffer to the terminal screen*/
        fe_refresh_screen( session );

        c = fe_get_user_input();
        lprintf( LOG_INFO, "read user input '%c' (%d)", c, c );

        /*
         * Move cursor when the user types a direct navigation
         * key. All buffer manipulating functions have to
         * correct the cursor by themselves.
         *
         * As a rule of thumb:
         * - Only one fe_ call per case
         * - No direct session r/w access
         * - No further logic
         * - You manipulate the buffer and make the cursor
         *   position incorrect? Fine, than fix that.
         */
        switch( c )
        {
            case UP:
                fe_move( session, 0, -1 );
                break;
            case LEFT:
                fe_move( session, -1, 0 );
                break;
            case DOWN:
                fe_move( session, 0, 1 );
                break;
            case RIGHT:
                fe_move( session, 1, 0 );
                break;
            case ESCAPE:
                exit_femto = 1;
                lprintf( LOG_INFO, "exited by user" );
                break;
            case ENTER_MAC:
            case ENTER:
                fe_insert_line( session );
                break;
            case BACKSPACE:
                fe_remove_char_at_cursor( session );
                break;
            default:
                if( isprint( c ) )
                    fe_insert_char( session, c );
        }

    }

    /* Restore old terminal mode */
    fe_disable_raw_mode();

    /* Free resources aka make valgrind happy */
    fe_free_session( session );

    lclose();
    return EXIT_SUCCESS;
}


