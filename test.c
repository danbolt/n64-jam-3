#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <stdint.h>
#include <libdragon.h>

static resolution_t res = RESOLUTION_640x240;
static bitdepth_t bit = DEPTH_32_BPP;

int filesize( FILE *pFile )
{
    fseek( pFile, 0, SEEK_END );
    int lSize = ftell( pFile );
    rewind( pFile );

    return lSize;
}

sprite_t *read_sprite( const char * const spritename )
{
    FILE *fp = fopen( spritename, "r" );

    if( fp )
    {
        sprite_t *sp = malloc( filesize( fp ) );
        fread( sp, 1, filesize( fp ), fp );
        fclose( fp );

        return sp;
    }
    else
    {
        return 0;
    }
}

int main(void)
{
    /* Initialize peripherals */
    display_init( res, bit, 2, GAMMA_NONE, ANTIALIAS_RESAMPLE );
    dfs_init( DFS_DEFAULT_LOCATION );
    controller_init();

    /* Main loop test */
    while(1) 
    {
        char tStr[256];
        static display_context_t disp = 0;

        /* Grab a render buffer */
        while( !(disp = display_lock()) );
       
        /*Fill the screen */
        graphics_fill_screen( disp, 0 );

        sprintf(tStr, "Video mode: %lu\n", *((uint32_t *)0x80000300));
        graphics_draw_text( disp, 20, 20, tStr );
        sprintf(tStr, "Status: %08lX\n", *((uint32_t *)0xa4400000));
        graphics_draw_text( disp, 20, 30, tStr );

        display_show(disp);

        /* Do we need to switch video displays? */
        controller_scan();
        struct controller_data keys = get_keys_down();

        if( keys.c[0].up )
        {
            //
        }
    }
}
