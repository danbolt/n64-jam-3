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

static unsigned long lastTimeInTicks;

static int bippingText;
static int currentTextIndex;
static unsigned long bipDelta;

#define ONSCREEN_TEXT_BUFFER_SIZE 512
static char onscreenText[ONSCREEN_TEXT_BUFFER_SIZE];

static int inTalkModal;

static int inExitModal;
#define MAX_NUMBER_OF_ONSCREEN_EXITS 4
#define ONSCREEN_EXIT_NAME_SIZE 64
static char onscreenExitText[MAX_NUMBER_OF_ONSCREEN_EXITS][ONSCREEN_EXIT_NAME_SIZE];
static int selectedExitIndex;
static int numberOfOnscreenExits;

static WrenVM* vm = NULL;
static WrenHandle* gameStateHandle = NULL;
static WrenHandle* hasNextLineHandle = NULL;
static WrenHandle* getNextLineHandle = NULL;
static WrenHandle* getExitsHandle = NULL;
static WrenHandle* getPeopleHandle = NULL;
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

    bippingText = 1;
    currentTextIndex = 0;
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

    inTalkModal = 0;

    inExitModal = 0;
    for (int i = 0; i < MAX_NUMBER_OF_ONSCREEN_EXITS; i++) {
        onscreenExitText[i][0] = '\0';
    }
    selectedExitIndex = 0;

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
    config.heapGrowthPercent = 10; // This should probably be zero at some point so we can stay within 4MB

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
    getExitsHandle = wrenMakeCallHandle(vm, "getExits()");
    getPeopleHandle = wrenMakeCallHandle(vm, "getPeople()");
    lookHandle = wrenMakeCallHandle(vm, "look()");
    investigateHandle = wrenMakeCallHandle(vm, "investigate()");
    talkHandle = wrenMakeCallHandle(vm, "talk(_)");
    itemHandle = wrenMakeCallHandle(vm, "item()");
    moveHandle = wrenMakeCallHandle(vm, "move(_)");

    bippingText = 0;
    currentTextIndex = 0;
    showNextLine();

    lastTimeInTicks = get_ticks();
}

void tickLogic() {
    controller_scan();
    struct controller_data keys = get_keys_down();

    unsigned long currentTimeInTicks = get_ticks();

    if( keys.c[0].C_up )
    {
        malloc_stats();
    }

     if (inTalkModal) {
        if (keys.c[0].B) {
            inTalkModal = 0;
        } else if (keys.c[0].A) {

            wrenEnsureSlots(vm, 2);
            wrenSetSlotHandle(vm, 0, gameStateHandle);
            wrenSetSlotString(vm, 1, onscreenExitText[selectedExitIndex]);
            wrenCall(vm, talkHandle);

            if (hasNextLine()) {
                showNextLine();
            }
            inTalkModal = 0;
        } else if (keys.c[0].down) {
            selectedExitIndex = (selectedExitIndex + 1) % numberOfOnscreenExits;
        } else if (keys.c[0].up) {
            selectedExitIndex = (selectedExitIndex - 1 + numberOfOnscreenExits) % numberOfOnscreenExits;
        }
     } else if (inExitModal) {
        if (keys.c[0].B) {
            inExitModal = 0;
        } else if (keys.c[0].A) {

            wrenEnsureSlots(vm, 2);
            wrenSetSlotHandle(vm, 0, gameStateHandle);
            wrenSetSlotString(vm, 1, onscreenExitText[selectedExitIndex]);
            wrenCall(vm, moveHandle);

            if (hasNextLine()) {
                showNextLine();
            }
            inExitModal = 0;
        } else if (keys.c[0].down) {
            selectedExitIndex = (selectedExitIndex + 1) % numberOfOnscreenExits;
        } else if (keys.c[0].up) {
            selectedExitIndex = (selectedExitIndex - 1 + numberOfOnscreenExits) % numberOfOnscreenExits;
        }
    } else if (bippingText) {
        if (onscreenText[currentTextIndex] == '\0') {
            bippingText = 0;
        } else {
            bipDelta += currentTimeInTicks - lastTimeInTicks;
            if (bipDelta > TICKS_FROM_MS(115)) {
                bipDelta = 0;
                currentTextIndex++;
            }
        }

        if (keys.c[0].A) {
            bippingText = 0;
        }
    } else if (!hasNextLine()) {
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
                    wrenCall(vm, getPeopleHandle);
                    numberOfOnscreenExits = wrenGetListCount(vm, 0);
                    if (numberOfOnscreenExits > 0) {
                        if (numberOfOnscreenExits > MAX_NUMBER_OF_ONSCREEN_EXITS) {
                            numberOfOnscreenExits = MAX_NUMBER_OF_ONSCREEN_EXITS;
                        }
                        for (int i = 0; i < numberOfOnscreenExits; i++) {
                            wrenGetListElement(vm, 0, i, 1);
                            const char* personName = wrenGetSlotString(vm, 1);
                            strncpy(onscreenExitText[i], personName, ONSCREEN_EXIT_NAME_SIZE);
                            onscreenExitText[i][ONSCREEN_EXIT_NAME_SIZE - 1] = '\0';
                        }
                        selectedExitIndex = 0;
                        inTalkModal = 1;
                    }
                    break;
                case Item:
                    wrenCall(vm, itemHandle);
                    break;
                case Move:
                    wrenCall(vm, getExitsHandle);
                    numberOfOnscreenExits = wrenGetListCount(vm, 0);
                    if (numberOfOnscreenExits > MAX_NUMBER_OF_ONSCREEN_EXITS) {
                        numberOfOnscreenExits = MAX_NUMBER_OF_ONSCREEN_EXITS;
                    }
                    for (int i = 0; i < numberOfOnscreenExits; i++) {
                        wrenGetListElement(vm, 0, i, 1);
                        const char* exitName = wrenGetSlotString(vm, 1);
                        strncpy(onscreenExitText[i], exitName, ONSCREEN_EXIT_NAME_SIZE);
                        onscreenExitText[i][ONSCREEN_EXIT_NAME_SIZE - 1] = '\0';
                    }
                    selectedExitIndex = 0;
                    inExitModal = 1;
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

    lastTimeInTicks = currentTimeInTicks;
}

void tickDisplay() {
    static display_context_t disp = 0;

    while( !(disp = display_lock()) );
   
    graphics_fill_screen( disp, graphics_convert_color(screenColor) );

    if (inTalkModal) {
        graphics_draw_text(disp, 22, 16, "Talk to...");

        for (int i = 0; i < numberOfOnscreenExits; i++) {
            graphics_draw_text(disp, 22 + 32, 16 + 16 + (16 * i), onscreenExitText[i]);

            if (i == selectedExitIndex) {
                graphics_draw_text(disp, 22 + 16, 16 + 16 + (16 * i), ">");
            }
        }
    } else if (inExitModal) {
        graphics_draw_text(disp, 22, 16, "Move to...");

        for (int i = 0; i < numberOfOnscreenExits; i++) {
            graphics_draw_text(disp, 22 + 32, 16 + 16 + (16 * i), onscreenExitText[i]);

            if (i == selectedExitIndex) {
                graphics_draw_text(disp, 22 + 16, 16 + 16 + (16 * i), ">");
            }
        }
    } else {
        if (bippingText) {
            int tx = 22;
            int ty = 16;
            
            // HACK: use a custom font for this
            for (int i = 0; i < currentTextIndex; i++) {
                if (onscreenText[i] == ' ') {
                    tx += 8;
                } else if (onscreenText[i] == '\n') {
                    tx = 22;

                    ty += 8;
                } else {
                    graphics_draw_character(disp, tx, ty, onscreenText[i]);
                    tx += 8;
                }

            }
        } else {
            graphics_draw_text(disp, 22, 16, onscreenText);
        }

        if (!hasNextLine() && !bippingText) {
            for (int i = 0; i < (int)(ActionCount); i++) {
                graphics_draw_sprite_stride(disp, 16 + (32 * i), 240 - 22, buttonsSprite, i);

                if (i == (Action)selectedActionIndex) {
                    graphics_draw_sprite_trans_stride(disp, 16 + (32 * i), 240 - 22, buttonsSprite, 7);
                }
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
