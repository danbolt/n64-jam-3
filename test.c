#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <stdint.h>
#include <libdragon.h>

#include "wren.h"

static resolution_t res = RESOLUTION_640x240;
static bitdepth_t bit = DEPTH_16_BPP;

void wrenWrite(WrenVM* vm, const char* text) {
  debugf( text );
}

void wrenErr(
      WrenVM* vm, 
      WrenErrorType type,
      const char* module,
      int line,
      const char* message) {
  switch (type)
  {
    case WREN_ERROR_COMPILE:
    {
      debugf("[%s line %d] [Error] %s\n", module, line, message);
    } break;
    case WREN_ERROR_STACK_TRACE:
    {
      debugf("[%s line %d] in %s\n", module, line, message);
    } break;
    case WREN_ERROR_RUNTIME:
    {
      debugf("[Runtime Error] %s\n", message);
    } break;
  }
}

int main(void)
{
    /* Initialize peripherals */
    display_init( res, bit, 2, GAMMA_NONE, ANTIALIAS_RESAMPLE );
    dfs_init( DFS_DEFAULT_LOCATION );
    controller_init();
    debug_init_isviewer();

    WrenConfiguration config;
    wrenInitConfiguration(&config);
    config.writeFn= &wrenWrite;
    config.errorFn = &wrenErr;

    // Really agressive heap stuff ; we don't have a lot of space!
    config.initialHeapSize = 1024 * 500;
    config.minHeapSize = 1024 * 10;
    config.heapGrowthPercent = 10;

    WrenVM* vm = wrenNewVM(&config);

    int fp = dfs_open("test.wren");
    void *script = malloc( dfs_size( fp ) );
    dfs_read( script, 1, dfs_size( fp ), fp );
    dfs_close( fp );

    WrenInterpretResult result = wrenInterpret(
            vm,
            "my_module",
            (const char*)script);
        result = result;



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
        sprintf(tStr, "memory size: %d\n", get_memory_size());
        graphics_draw_text( disp, 20, 30, tStr );

        display_show(disp);

        /* Do we need to switch video displays? */
        controller_scan();
        struct controller_data keys = get_keys_down();

        if( keys.c[0].C_up )
        {
            malloc_stats();
        }
    }
}
