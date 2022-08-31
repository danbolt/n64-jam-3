#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <stdint.h>
#include <libdragon.h>

#include "wren.h"

static resolution_t res = RESOLUTION_640x240;
static bitdepth_t bit = DEPTH_16_BPP;

static color_t screenColor;
static color_t targetScreenColor;

static sprite_t* buttonsSprite;

#define ONSCREEN_TEXT_BUFFER_SIZE 512
static char onscreenText[ONSCREEN_TEXT_BUFFER_SIZE];

static WrenVM* vm = NULL;
static WrenHandle* gameStateHandle = NULL;
static WrenHandle* hasNextLineHandle = NULL;
static WrenHandle* getNextLineHandle = NULL;
static WrenHandle* lookHandle = NULL;
static WrenHandle* investigateHandle = NULL;
static WrenHandle* talkHandle = NULL;
static WrenHandle* itemHandle = NULL;
static WrenHandle* moveHandle = NULL;

typedef enum {
    Look = 0,
    Check,
    Talk,
    Item,
    Move,

    ActionCount
} Action;
static Action selectedActionIndex;

void finishedLoadingModule(WrenVM* vm, const char* name, struct WrenLoadModuleResult result) {
    if (result.source) {
        free((void*)(result.source));

        // debugf("Unloaded %s\n", name);
    }
}

WrenLoadModuleResult loadModule(WrenVM* vm, const char* name) {
    WrenLoadModuleResult result = {0};
    result.onComplete = finishedLoadingModule;

    // debugf("Attempting to load %s\n", name);

    int fp = dfs_open(name);
    if (fp < 0) {
        // debugf("Could not load %s; error: %d\n", name, fp);
        result.source = NULL;
    } else {
        // debugf("%s loaded!\n", name);

        // Note that we're allocating an extra byte to terminate our script. Otherwise the wren interpreter will overrun into system memory.
        int fileSize = dfs_size( fp );
        void *script = malloc( fileSize + 1 );
        dfs_read( script, 1, dfs_size( fp ), fp );
        ((char*)script)[fileSize] = '\0';
        dfs_close( fp );

        result.source = script;
    }
    
    return result;
}

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

void setScreenColor(WrenVM* vm) {
  targetScreenColor.r = (uint8_t)wrenGetSlotDouble(vm, 1);
  targetScreenColor.g = (uint8_t)wrenGetSlotDouble(vm, 2);
  targetScreenColor.b = (uint8_t)wrenGetSlotDouble(vm, 3);
}

WrenForeignMethodFn bindForeignMethodToWren(WrenVM* vm, const char* module, const char* className, bool isStatic, const char* signature) {
    if ((strcmp(module, "view.wren") == 0) && (strcmp(className, "View") == 0) && isStatic && (strcmp(signature, "setScreenColor(_,_,_)") == 0)) {
        return setScreenColor;
    }

    return NULL;
}

void initHUDSprites() {
    int fp = dfs_open("buttons.sprite");
    buttonsSprite = malloc( dfs_size( fp ) );
    dfs_read( buttonsSprite, 1, dfs_size( fp ), fp );
    dfs_close( fp );
}

void initGame() {
    screenColor = (color_t){ 0, 0, 0, 255 };
    targetScreenColor = (color_t){ 0, 0, 0, 255 };

    onscreenText[0] = '\0';

    initHUDSprites();
    selectedActionIndex = Look;

    WrenConfiguration config;
    wrenInitConfiguration(&config);
    config.loadModuleFn = loadModule;
    config.bindForeignMethodFn = &bindForeignMethodToWren;
    config.writeFn= &wrenWrite;
    config.errorFn = &wrenErr;
    config.initialHeapSize = 1024 * 500; // Really agressive heap stuff ; we don't have a lot of space!
    config.minHeapSize = 1024 * 10;
    config.heapGrowthPercent = 10;

    vm = wrenNewVM(&config);
    WrenInterpretResult result = wrenInterpret(
            vm,
            "my_module",
            "import \"main_game.wren\" for Gameplay\nGameplay.execute()");
    result = result;

    wrenEnsureSlots(vm, 1);
    wrenGetVariable(vm, "main_game.wren", "GameState", 0);
    gameStateHandle = wrenGetSlotHandle(vm, 0);
    hasNextLineHandle = wrenMakeCallHandle(vm, "hasNextLine()");
    getNextLineHandle = wrenMakeCallHandle(vm, "getNextLine()");
    lookHandle = wrenMakeCallHandle(vm, "look()");
    investigateHandle = wrenMakeCallHandle(vm, "investigate()");
    talkHandle = wrenMakeCallHandle(vm, "talk()");
    itemHandle = wrenMakeCallHandle(vm, "item()");
    moveHandle = wrenMakeCallHandle(vm, "move()");
}

// TODO: Use a math library or move this
float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

bool hasNextLine() {
    wrenEnsureSlots(vm, 1);
    wrenSetSlotHandle(vm, 0, gameStateHandle);
    wrenCall(vm, hasNextLineHandle);
    return wrenGetSlotBool(vm, 0);;
}

void showNextLine() {
    if (!hasNextLine()) {
        return;
    }

    wrenSetSlotHandle(vm, 0, gameStateHandle);
    wrenCall(vm, getNextLineHandle);
    const char* nextLineText = wrenGetSlotString(vm, 0);
    strncpy(onscreenText, nextLineText, ONSCREEN_TEXT_BUFFER_SIZE - 1);
}

void tickLogic() {
    controller_scan();
    struct controller_data keys = get_keys_down();

    if( keys.c[0].C_up )
    {
        malloc_stats();
    }

    if (!hasNextLine()) {
        if (keys.c[0].left) {
            selectedActionIndex = (Action)(((int)selectedActionIndex - 1 + (int)ActionCount) % (int)ActionCount);
        } else if (keys.c[0].right) {
            selectedActionIndex = (Action)((((int)selectedActionIndex + 1) % (int)ActionCount));
        }

        if (keys.c[0].A) {
            wrenSetSlotHandle(vm, 0, gameStateHandle);
            switch(selectedActionIndex) {
                case Look:
                    wrenCall(vm, lookHandle);
                    break;
                case Check:
                    wrenCall(vm, investigateHandle);
                    break;
                case Talk:
                    wrenCall(vm, talkHandle);
                    break;
                case Item:
                    wrenCall(vm, itemHandle);
                    break;
                case Move:
                    wrenCall(vm, moveHandle);
                    break;
                default:
                    break;
            }

            if (hasNextLine()) {
                showNextLine();
            }
        }
    } else {
        if (keys.c[0].A) {
            showNextLine();
        }
    }

    screenColor.r = (uint8_t)lerp((float)(screenColor.r), (float)(targetScreenColor.r), 0.13f);
    screenColor.g = (uint8_t)lerp((float)(screenColor.g), (float)(targetScreenColor.g), 0.13f);
    screenColor.b = (uint8_t)lerp((float)(screenColor.b), (float)(targetScreenColor.b), 0.13f);
}

void tickDisplay() {
    static display_context_t disp = 0;

    while( !(disp = display_lock()) );
   
    graphics_fill_screen( disp, graphics_convert_color(screenColor) );

    graphics_draw_text(disp, 22, 16, onscreenText);

    if (!hasNextLine()) {
        for (int i = 0; i < (int)(ActionCount); i++) {
            graphics_draw_sprite_stride(disp, 16 + (32 * i), 240 - 22, buttonsSprite, i);

            if (i == (Action)selectedActionIndex) {
                graphics_draw_sprite_trans_stride(disp, 16 + (32 * i), 240 - 22, buttonsSprite, 7);
            }
        }
    }

    display_show(disp);
}

void tick() {
    tickDisplay();
    tickLogic();
}

int main(void)
{
    /* Initialize peripherals */
    display_init( res, bit, 2, GAMMA_NONE, ANTIALIAS_RESAMPLE );
    dfs_init( DFS_DEFAULT_LOCATION );
    controller_init();
    debug_init_isviewer();

    initGame();

    /* Main loop test */
    while(1) 
    {
        tick();
    }
}
