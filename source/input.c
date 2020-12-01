#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#ifdef __linux__
#include <linux/input.h>
#include <errno.h>
#endif
#include "task.h"
#include "list.h"
#include "input.h"

#if 0
static const char *key_table[KEY_CNT] =
{
    [KEY_RESERVED             ] = "RESERVED",
    [KEY_ESC                  ] = "ESC",
    [KEY_1                    ] = "1",
    [KEY_2                    ] = "2",
    [KEY_3                    ] = "3",
    [KEY_4                    ] = "4",
    [KEY_5                    ] = "5",
    [KEY_6                    ] = "6",
    [KEY_7                    ] = "7",
    [KEY_8                    ] = "8",
    [KEY_9                    ] = "9",
    [KEY_0                    ] = "0",
    [KEY_MINUS                ] = "MINUS",
    [KEY_EQUAL                ] = "EQUAL",
    [KEY_BACKSPACE            ] = "BACKSPACE",
    [KEY_TAB                  ] = "TAB",
    [KEY_Q                    ] = "Q",
    [KEY_W                    ] = "W",
    [KEY_E                    ] = "E",
    [KEY_R                    ] = "R",
    [KEY_T                    ] = "T",
    [KEY_Y                    ] = "Y",
    [KEY_U                    ] = "U",
    [KEY_I                    ] = "I",
    [KEY_O                    ] = "O",
    [KEY_P                    ] = "P",
    [KEY_LEFTBRACE            ] = "LEFTBRACE",
    [KEY_RIGHTBRACE           ] = "RIGHTBRACE",
    [KEY_ENTER                ] = "ENTER",
    [KEY_LEFTCTRL             ] = "LEFTCTRL",
    [KEY_A                    ] = "A",
    [KEY_S                    ] = "S",
    [KEY_D                    ] = "D",
    [KEY_F                    ] = "F",
    [KEY_G                    ] = "G",
    [KEY_H                    ] = "H",
    [KEY_J                    ] = "J",
    [KEY_K                    ] = "K",
    [KEY_L                    ] = "L",
    [KEY_SEMICOLON            ] = "SEMICOLON",
    [KEY_APOSTROPHE           ] = "APOSTROPHE",
    [KEY_GRAVE                ] = "GRAVE",
    [KEY_LEFTSHIFT            ] = "LEFTSHIFT",
    [KEY_BACKSLASH            ] = "BACKSLASH",
    [KEY_Z                    ] = "Z",
    [KEY_X                    ] = "X",
    [KEY_C                    ] = "C",
    [KEY_V                    ] = "V",
    [KEY_B                    ] = "B",
    [KEY_N                    ] = "N",
    [KEY_M                    ] = "M",
    [KEY_COMMA                ] = "COMMA",
    [KEY_DOT                  ] = "DOT",
    [KEY_SLASH                ] = "SLASH",
    [KEY_RIGHTSHIFT           ] = "RIGHTSHIFT",
    [KEY_KPASTERISK           ] = "KPASTERISK",
    [KEY_LEFTALT              ] = "LEFTALT",
    [KEY_SPACE                ] = "SPACE",
    [KEY_CAPSLOCK             ] = "CAPSLOCK",
    [KEY_F1                   ] = "F1",
    [KEY_F2                   ] = "F2",
    [KEY_F3                   ] = "F3",
    [KEY_F4                   ] = "F4",
    [KEY_F5                   ] = "F5",
    [KEY_F6                   ] = "F6",
    [KEY_F7                   ] = "F7",
    [KEY_F8                   ] = "F8",
    [KEY_F9                   ] = "F9",
    [KEY_F10                  ] = "F10",
    [KEY_NUMLOCK              ] = "NUMLOCK",
    [KEY_SCROLLLOCK           ] = "SCROLLLOCK",
    [KEY_KP7                  ] = "KP7",
    [KEY_KP8                  ] = "KP8",
    [KEY_KP9                  ] = "KP9",
    [KEY_KPMINUS              ] = "KPMINUS",
    [KEY_KP4                  ] = "KP4",
    [KEY_KP5                  ] = "KP5",
    [KEY_KP6                  ] = "KP6",
    [KEY_KPPLUS               ] = "KPPLUS",
    [KEY_KP1                  ] = "KP1",
    [KEY_KP2                  ] = "KP2",
    [KEY_KP3                  ] = "KP3",
    [KEY_KP0                  ] = "KP0",
    [KEY_KPDOT                ] = "KPDOT",
    [KEY_ZENKAKUHANKAKU       ] = "ZENKAKUHANKAKU",
    [KEY_102ND                ] = "102ND",
    [KEY_F11                  ] = "F11",
    [KEY_F12                  ] = "F12",
    [KEY_RO                   ] = "RO",
    [KEY_KATAKANA             ] = "KATAKANA",
    [KEY_HIRAGANA             ] = "HIRAGANA",
    [KEY_HENKAN               ] = "HENKAN",
    [KEY_KATAKANAHIRAGANA     ] = "KATAKANAHIRAGANA",
    [KEY_MUHENKAN             ] = "MUHENKAN",
    [KEY_KPJPCOMMA            ] = "KPJPCOMMA",
    [KEY_KPENTER              ] = "KPENTER",
    [KEY_RIGHTCTRL            ] = "RIGHTCTRL",
    [KEY_KPSLASH              ] = "KPSLASH",
    [KEY_SYSRQ                ] = "SYSRQ",
    [KEY_RIGHTALT             ] = "RIGHTALT",
    [KEY_LINEFEED             ] = "LINEFEED",
    [KEY_HOME                 ] = "HOME",
    [KEY_UP                   ] = "UP",
    [KEY_PAGEUP               ] = "PAGEUP",
    [KEY_LEFT                 ] = "LEFT",
    [KEY_RIGHT                ] = "RIGHT",
    [KEY_END                  ] = "END",
    [KEY_DOWN                 ] = "DOWN",
    [KEY_PAGEDOWN             ] = "PAGEDOWN",
    [KEY_INSERT               ] = "INSERT",
    [KEY_DELETE               ] = "DELETE",
    [KEY_MACRO                ] = "MACRO",
    [KEY_MUTE                 ] = "MUTE",
    [KEY_VOLUMEDOWN           ] = "VOLUMEDOWN",
    [KEY_VOLUMEUP             ] = "VOLUMEUP",
    [KEY_POWER                ] = "POWER",
    [KEY_KPEQUAL              ] = "KPEQUAL",
    [KEY_KPPLUSMINUS          ] = "KPPLUSMINUS",
    [KEY_PAUSE                ] = "PAUSE",
    [KEY_SCALE                ] = "SCALE",
    [KEY_KPCOMMA              ] = "KPCOMMA",
    [KEY_HANGEUL              ] = "HANGEUL",
    [KEY_HANGUEL              ] = "HANGUEL",
    [KEY_HANJA                ] = "HANJA",
    [KEY_YEN                  ] = "YEN",
    [KEY_LEFTMETA             ] = "LEFTMETA",
    [KEY_RIGHTMETA            ] = "RIGHTMETA",
    [KEY_COMPOSE              ] = "COMPOSE",
    [KEY_STOP                 ] = "STOP",
    [KEY_AGAIN                ] = "AGAIN",
    [KEY_PROPS                ] = "PROPS",
    [KEY_UNDO                 ] = "UNDO",
    [KEY_FRONT                ] = "FRONT",
    [KEY_COPY                 ] = "COPY",
    [KEY_OPEN                 ] = "OPEN",
    [KEY_PASTE                ] = "PASTE",
    [KEY_FIND                 ] = "FIND",
    [KEY_CUT                  ] = "CUT",
    [KEY_HELP                 ] = "HELP",
    [KEY_MENU                 ] = "MENU",
    [KEY_CALC                 ] = "CALC",
    [KEY_SETUP                ] = "SETUP",
    [KEY_SLEEP                ] = "SLEEP",
    [KEY_WAKEUP               ] = "WAKEUP",
    [KEY_FILE                 ] = "FILE",
    [KEY_SENDFILE             ] = "SENDFILE",
    [KEY_DELETEFILE           ] = "DELETEFILE",
    [KEY_XFER                 ] = "XFER",
    [KEY_PROG1                ] = "PROG1",
    [KEY_PROG2                ] = "PROG2",
    [KEY_WWW                  ] = "WWW",
    [KEY_MSDOS                ] = "MSDOS",
    [KEY_COFFEE               ] = "COFFEE",
    [KEY_SCREENLOCK           ] = "SCREENLOCK",
    [KEY_DIRECTION            ] = "DIRECTION",
    [KEY_CYCLEWINDOWS         ] = "CYCLEWINDOWS",
    [KEY_MAIL                 ] = "MAIL",
    [KEY_BOOKMARKS            ] = "BOOKMARKS",
    [KEY_COMPUTER             ] = "COMPUTER",
    [KEY_BACK                 ] = "BACK",
    [KEY_FORWARD              ] = "FORWARD",
    [KEY_CLOSECD              ] = "CLOSECD",
    [KEY_EJECTCD              ] = "EJECTCD",
    [KEY_EJECTCLOSECD         ] = "EJECTCLOSECD",
    [KEY_NEXTSONG             ] = "NEXTSONG",
    [KEY_PLAYPAUSE            ] = "PLAYPAUSE",
    [KEY_PREVIOUSSONG         ] = "PREVIOUSSONG",
    [KEY_STOPCD               ] = "STOPCD",
    [KEY_RECORD               ] = "RECORD",
    [KEY_REWIND               ] = "REWIND",
    [KEY_PHONE                ] = "PHONE",
    [KEY_ISO                  ] = "ISO",
    [KEY_CONFIG               ] = "CONFIG",
    [KEY_HOMEPAGE             ] = "HOMEPAGE",
    [KEY_REFRESH              ] = "REFRESH",
    [KEY_EXIT                 ] = "EXIT",
    [KEY_MOVE                 ] = "MOVE",
    [KEY_EDIT                 ] = "EDIT",
    [KEY_SCROLLUP             ] = "SCROLLUP",
    [KEY_SCROLLDOWN           ] = "SCROLLDOWN",
    [KEY_KPLEFTPAREN          ] = "KPLEFTPAREN",
    [KEY_KPRIGHTPAREN         ] = "KPRIGHTPAREN",
    [KEY_NEW                  ] = "NEW",
    [KEY_REDO                 ] = "REDO",
    [KEY_F13                  ] = "F13",
    [KEY_F14                  ] = "F14",
    [KEY_F15                  ] = "F15",
    [KEY_F16                  ] = "F16",
    [KEY_F17                  ] = "F17",
    [KEY_F18                  ] = "F18",
    [KEY_F19                  ] = "F19",
    [KEY_F20                  ] = "F20",
    [KEY_F21                  ] = "F21",
    [KEY_F22                  ] = "F22",
    [KEY_F23                  ] = "F23",
    [KEY_F24                  ] = "F24",
    [KEY_PLAYCD               ] = "PLAYCD",
    [KEY_PAUSECD              ] = "PAUSECD",
    [KEY_PROG3                ] = "PROG3",
    [KEY_PROG4                ] = "PROG4",
    [KEY_DASHBOARD            ] = "DASHBOARD",
    [KEY_SUSPEND              ] = "SUSPEND",
    [KEY_CLOSE                ] = "CLOSE",
    [KEY_PLAY                 ] = "PLAY",
    [KEY_FASTFORWARD          ] = "FASTFORWARD",
    [KEY_BASSBOOST            ] = "BASSBOOST",
    [KEY_PRINT                ] = "PRINT",
    [KEY_HP                   ] = "HP",
    [KEY_CAMERA               ] = "CAMERA",
    [KEY_SOUND                ] = "SOUND",
    [KEY_QUESTION             ] = "QUESTION",
    [KEY_EMAIL                ] = "EMAIL",
    [KEY_CHAT                 ] = "CHAT",
    [KEY_SEARCH               ] = "SEARCH",
    [KEY_CONNECT              ] = "CONNECT",
    [KEY_FINANCE              ] = "FINANCE",
    [KEY_SPORT                ] = "SPORT",
    [KEY_SHOP                 ] = "SHOP",
    [KEY_ALTERASE             ] = "ALTERASE",
    [KEY_CANCEL               ] = "CANCEL",
    [KEY_BRIGHTNESSDOWN       ] = "BRIGHTNESSDOWN",
    [KEY_BRIGHTNESSUP         ] = "BRIGHTNESSUP",
    [KEY_MEDIA                ] = "MEDIA",
    [KEY_SWITCHVIDEOMODE      ] = "SWITCHVIDEOMODE",
    [KEY_KBDILLUMTOGGLE       ] = "KBDILLUMTOGGLE",
    [KEY_KBDILLUMDOWN         ] = "KBDILLUMDOWN",
    [KEY_KBDILLUMUP           ] = "KBDILLUMUP",
    [KEY_SEND                 ] = "SEND",
    [KEY_REPLY                ] = "REPLY",
    [KEY_FORWARDMAIL          ] = "FORWARDMAIL",
    [KEY_SAVE                 ] = "SAVE",
    [KEY_DOCUMENTS            ] = "DOCUMENTS",
    [KEY_BATTERY              ] = "BATTERY",
    [KEY_BLUETOOTH            ] = "BLUETOOTH",
    [KEY_WLAN                 ] = "WLAN",
    [KEY_UWB                  ] = "UWB",
    [KEY_UNKNOWN              ] = "UNKNOWN",
    [KEY_VIDEO_NEXT           ] = "VIDEO_NEXT",
    [KEY_VIDEO_PREV           ] = "VIDEO_PREV",
    [KEY_BRIGHTNESS_CYCLE     ] = "BRIGHTNESS_CYCLE",
    [KEY_BRIGHTNESS_ZERO      ] = "BRIGHTNESS_ZERO",
    [KEY_DISPLAY_OFF          ] = "DISPLAY_OFF",
    [KEY_WIMAX                ] = "WIMAX",
    [BTN_MISC                 ] = "BTN_MISC",
    [BTN_0                    ] = "BTN_0",
    [BTN_1                    ] = "BTN_1",
    [BTN_2                    ] = "BTN_2",
    [BTN_3                    ] = "BTN_3",
    [BTN_4                    ] = "BTN_4",
    [BTN_5                    ] = "BTN_5",
    [BTN_6                    ] = "BTN_6",
    [BTN_7                    ] = "BTN_7",
    [BTN_8                    ] = "BTN_8",
    [BTN_9                    ] = "BTN_9",
    [BTN_MOUSE                ] = "BTN_MOUSE",
    [BTN_LEFT                 ] = "BTN_LEFT",
    [BTN_RIGHT                ] = "BTN_RIGHT",
    [BTN_MIDDLE               ] = "BTN_MIDDLE",
    [BTN_SIDE                 ] = "BTN_SIDE",
    [BTN_EXTRA                ] = "BTN_EXTRA",
    [BTN_FORWARD              ] = "BTN_FORWARD",
    [BTN_BACK                 ] = "BTN_BACK",
    [BTN_TASK                 ] = "BTN_TASK",
    [BTN_JOYSTICK             ] = "BTN_JOYSTICK",
    [BTN_TRIGGER              ] = "BTN_TRIGGER",
    [BTN_THUMB                ] = "BTN_THUMB",
    [BTN_THUMB2               ] = "BTN_THUMB2",
    [BTN_TOP                  ] = "BTN_TOP",
    [BTN_TOP2                 ] = "BTN_TOP2",
    [BTN_PINKIE               ] = "BTN_PINKIE",
    [BTN_BASE                 ] = "BTN_BASE",
    [BTN_BASE2                ] = "BTN_BASE2",
    [BTN_BASE3                ] = "BTN_BASE3",
    [BTN_BASE4                ] = "BTN_BASE4",
    [BTN_BASE5                ] = "BTN_BASE5",
    [BTN_BASE6                ] = "BTN_BASE6",
    [BTN_DEAD                 ] = "BTN_DEAD",
    [BTN_GAMEPAD              ] = "BTN_GAMEPAD",
    [BTN_A                    ] = "BTN_A",
    [BTN_B                    ] = "BTN_B",
    [BTN_C                    ] = "BTN_C",
    [BTN_X                    ] = "BTN_X",
    [BTN_Y                    ] = "BTN_Y",
    [BTN_Z                    ] = "BTN_Z",
    [BTN_TL                   ] = "BTN_TL",
    [BTN_TR                   ] = "BTN_TR",
    [BTN_TL2                  ] = "BTN_TL2",
    [BTN_TR2                  ] = "BTN_TR2",
    [BTN_SELECT               ] = "BTN_SELECT",
    [BTN_START                ] = "BTN_START",
    [BTN_MODE                 ] = "BTN_MODE",
    [BTN_THUMBL               ] = "BTN_THUMBL",
    [BTN_THUMBR               ] = "BTN_THUMBR",
    [BTN_DIGI                 ] = "BTN_DIGI",
    [BTN_TOOL_PEN             ] = "BTN_TOOL_PEN",
    [BTN_TOOL_RUBBER          ] = "BTN_TOOL_RUBBER",
    [BTN_TOOL_BRUSH           ] = "BTN_TOOL_BRUSH",
    [BTN_TOOL_PENCIL          ] = "BTN_TOOL_PENCIL",
    [BTN_TOOL_AIRBRUSH        ] = "BTN_TOOL_AIRBRUSH",
    [BTN_TOOL_FINGER          ] = "BTN_TOOL_FINGER",
    [BTN_TOOL_MOUSE           ] = "BTN_TOOL_MOUSE",
    [BTN_TOOL_LENS            ] = "BTN_TOOL_LENS",
    [BTN_TOUCH                ] = "BTN_TOUCH",
    [BTN_STYLUS               ] = "BTN_STYLUS",
    [BTN_STYLUS2              ] = "BTN_STYLUS2",
    [BTN_TOOL_DOUBLETAP       ] = "BTN_TOOL_DOUBLETAP",
    [BTN_TOOL_TRIPLETAP       ] = "BTN_TOOL_TRIPLETAP",
    [BTN_TOOL_QUADTAP         ] = "BTN_TOOL_QUADTAP",
    [BTN_WHEEL                ] = "BTN_WHEEL",
    [BTN_GEAR_DOWN            ] = "BTN_GEAR_DOWN",
    [BTN_GEAR_UP              ] = "BTN_GEAR_UP",
    [KEY_OK                   ] = "OK",
    [KEY_SELECT               ] = "SELECT",
    [KEY_GOTO                 ] = "GOTO",
    [KEY_CLEAR                ] = "CLEAR",
    [KEY_POWER2               ] = "POWER2",
    [KEY_OPTION               ] = "OPTION",
    [KEY_INFO                 ] = "INFO",
    [KEY_TIME                 ] = "TIME",
    [KEY_VENDOR               ] = "VENDOR",
    [KEY_ARCHIVE              ] = "ARCHIVE",
    [KEY_PROGRAM              ] = "PROGRAM",
    [KEY_CHANNEL              ] = "CHANNEL",
    [KEY_FAVORITES            ] = "FAVORITES",
    [KEY_EPG                  ] = "EPG",
    [KEY_PVR                  ] = "PVR",
    [KEY_MHP                  ] = "MHP",
    [KEY_LANGUAGE             ] = "LANGUAGE",
    [KEY_TITLE                ] = "TITLE",
    [KEY_SUBTITLE             ] = "SUBTITLE",
    [KEY_ANGLE                ] = "ANGLE",
    [KEY_ZOOM                 ] = "ZOOM",
    [KEY_MODE                 ] = "MODE",
    [KEY_KEYBOARD             ] = "KEYBOARD",
    [KEY_SCREEN               ] = "SCREEN",
    [KEY_PC                   ] = "PC",
    [KEY_TV                   ] = "TV",
    [KEY_TV2                  ] = "TV2",
    [KEY_VCR                  ] = "VCR",
    [KEY_VCR2                 ] = "VCR2",
    [KEY_SAT                  ] = "SAT",
    [KEY_SAT2                 ] = "SAT2",
    [KEY_CD                   ] = "CD",
    [KEY_TAPE                 ] = "TAPE",
    [KEY_RADIO                ] = "RADIO",
    [KEY_TUNER                ] = "TUNER",
    [KEY_PLAYER               ] = "PLAYER",
    [KEY_TEXT                 ] = "TEXT",
    [KEY_DVD                  ] = "DVD",
    [KEY_AUX                  ] = "AUX",
    [KEY_MP3                  ] = "MP3",
    [KEY_AUDIO                ] = "AUDIO",
    [KEY_VIDEO                ] = "VIDEO",
    [KEY_DIRECTORY            ] = "DIRECTORY",
    [KEY_LIST                 ] = "LIST",
    [KEY_MEMO                 ] = "MEMO",
    [KEY_CALENDAR             ] = "CALENDAR",
    [KEY_RED                  ] = "RED",
    [KEY_GREEN                ] = "GREEN",
    [KEY_YELLOW               ] = "YELLOW",
    [KEY_BLUE                 ] = "BLUE",
    [KEY_CHANNELUP            ] = "CHANNELUP",
    [KEY_CHANNELDOWN          ] = "CHANNELDOWN",
    [KEY_FIRST                ] = "FIRST",
    [KEY_LAST                 ] = "LAST",
    [KEY_AB                   ] = "AB",
    [KEY_NEXT                 ] = "NEXT",
    [KEY_RESTART              ] = "RESTART",
    [KEY_SLOW                 ] = "SLOW",
    [KEY_SHUFFLE              ] = "SHUFFLE",
    [KEY_BREAK                ] = "BREAK",
    [KEY_PREVIOUS             ] = "PREVIOUS",
    [KEY_DIGITS               ] = "DIGITS",
    [KEY_TEEN                 ] = "TEEN",
    [KEY_TWEN                 ] = "TWEN",
    [KEY_VIDEOPHONE           ] = "VIDEOPHONE",
    [KEY_GAMES                ] = "GAMES",
    [KEY_ZOOMIN               ] = "ZOOMIN",
    [KEY_ZOOMOUT              ] = "ZOOMOUT",
    [KEY_ZOOMRESET            ] = "ZOOMRESET",
    [KEY_WORDPROCESSOR        ] = "WORDPROCESSOR",
    [KEY_EDITOR               ] = "EDITOR",
    [KEY_SPREADSHEET          ] = "SPREADSHEET",
    [KEY_GRAPHICSEDITOR       ] = "GRAPHICSEDITOR",
    [KEY_PRESENTATION         ] = "PRESENTATION",
    [KEY_DATABASE             ] = "DATABASE",
    [KEY_NEWS                 ] = "NEWS",
    [KEY_VOICEMAIL            ] = "VOICEMAIL",
    [KEY_ADDRESSBOOK          ] = "ADDRESSBOOK",
    [KEY_MESSENGER            ] = "MESSENGER",
    [KEY_DISPLAYTOGGLE        ] = "DISPLAYTOGGLE",
    [KEY_SPELLCHECK           ] = "SPELLCHECK",
    [KEY_LOGOFF               ] = "LOGOFF",
    [KEY_DOLLAR               ] = "DOLLAR",
    [KEY_EURO                 ] = "EURO",
    [KEY_FRAMEBACK            ] = "FRAMEBACK",
    [KEY_FRAMEFORWARD         ] = "FRAMEFORWARD",
    [KEY_CONTEXT_MENU         ] = "CONTEXT_MENU",
    [KEY_MEDIA_REPEAT         ] = "MEDIA_REPEAT",
    [KEY_DEL_EOL              ] = "DEL_EOL",
    [KEY_DEL_EOS              ] = "DEL_EOS",
    [KEY_INS_LINE             ] = "INS_LINE",
    [KEY_DEL_LINE             ] = "DEL_LINE",
    [KEY_FN                   ] = "FN",
    [KEY_FN_ESC               ] = "FN_ESC",
    [KEY_FN_F1                ] = "FN_F1",
    [KEY_FN_F2                ] = "FN_F2",
    [KEY_FN_F3                ] = "FN_F3",
    [KEY_FN_F4                ] = "FN_F4",
    [KEY_FN_F5                ] = "FN_F5",
    [KEY_FN_F6                ] = "FN_F6",
    [KEY_FN_F7                ] = "FN_F7",
    [KEY_FN_F8                ] = "FN_F8",
    [KEY_FN_F9                ] = "FN_F9",
    [KEY_FN_F10               ] = "FN_F10",
    [KEY_FN_F11               ] = "FN_F11",
    [KEY_FN_F12               ] = "FN_F12",
    [KEY_FN_1                 ] = "FN_1",
    [KEY_FN_2                 ] = "FN_2",
    [KEY_FN_D                 ] = "FN_D",
    [KEY_FN_E                 ] = "FN_E",
    [KEY_FN_F                 ] = "FN_F",
    [KEY_FN_S                 ] = "FN_S",
    [KEY_FN_B                 ] = "FN_B",
    [KEY_BRL_DOT1             ] = "BRL_DOT1",
    [KEY_BRL_DOT2             ] = "BRL_DOT2",
    [KEY_BRL_DOT3             ] = "BRL_DOT3",
    [KEY_BRL_DOT4             ] = "BRL_DOT4",
    [KEY_BRL_DOT5             ] = "BRL_DOT5",
    [KEY_BRL_DOT6             ] = "BRL_DOT6",
    [KEY_BRL_DOT7             ] = "BRL_DOT7",
    [KEY_BRL_DOT8             ] = "BRL_DOT8",
    [KEY_BRL_DOT9             ] = "BRL_DOT9",
    [KEY_BRL_DOT10            ] = "BRL_DOT10",
    [KEY_NUMERIC_0            ] = "NUMERIC_0",
    [KEY_NUMERIC_1            ] = "NUMERIC_1",
    [KEY_NUMERIC_2            ] = "NUMERIC_2",
    [KEY_NUMERIC_3            ] = "NUMERIC_3",
    [KEY_NUMERIC_4            ] = "NUMERIC_4",
    [KEY_NUMERIC_5            ] = "NUMERIC_5",
    [KEY_NUMERIC_6            ] = "NUMERIC_6",
    [KEY_NUMERIC_7            ] = "NUMERIC_7",
    [KEY_NUMERIC_8            ] = "NUMERIC_8",
    [KEY_NUMERIC_9            ] = "NUMERIC_9",
    [KEY_NUMERIC_STAR         ] = "NUMERIC_STAR",
    [KEY_NUMERIC_POUND        ] = "NUMERIC_POUND",
};

static const char *switch_table[SW_CNT] = {
    [SW_LID                   ] = "LID",
    [SW_TABLET_MODE           ] = "TABLET_MODE",
    [SW_HEADPHONE_INSERT      ] = "HEADPHONE_INSERT",
    [SW_RFKILL_ALL            ] = "RFKILL_ALL",
    [SW_RADIO                 ] = "RADIO",
    [SW_MICROPHONE_INSERT     ] = "MICROPHONE_INSERT",
    [SW_DOCK                  ] = "DOCK",
    [SW_LINEOUT_INSERT        ] = "LINEOUT_INSERT",
    [SW_JACK_PHYSICAL_INSERT  ] = "JACK_PHYSICAL_INSERT",
    [SW_VIDEOOUT_INSERT       ] = "VIDEOOUT_INSERT",
};
#endif

typedef struct
{
    int fd;
    cb_input_rxdata_func cbRxData;
    void *ctx;
    struct input_event msgbuffer;
    struct dl_list list;
}input_t;

typedef struct
{
    hMutex mtx;
    hThread thread;
    int ref;
    int first;
    struct dl_list input_head;
}input_priv_t;
static input_priv_t input_priv;

hInput input_open(const char *name)
{
    if(!name)
        return NULL;
    input_t *input = (input_t*)calloc(1,sizeof(input_t));
    if(!input)
        return NULL;

    input->fd = open(name,O_RDONLY);
    if(input->fd < 0)
    {
        free(input);
        return NULL;
    }
    char name_[256] = "";
    char phys_[64] = "";
    unsigned char evmask[EV_MAX/8 + 1];

#define test_bit(array,bit) ((array)[(bit)/8] & (1 << ((bit)%8)))

    ioctl(input->fd,EVIOCGNAME(sizeof(name_)),name_);
    ioctl(input->fd,EVIOCGPHYS(sizeof(phys_)),phys_);
    ioctl(input->fd,EVIOCGBIT(0, sizeof(evmask)),evmask);

    printf("name    :%s\n",name_);
    printf("phys    :%s\n",phys_);
    printf("features :");
    int e;
    for(e = 0; e < EV_MAX;e++)
    {
        if(test_bit(evmask,e))
        {
            switch(e)
            {
            case EV_KEY:
                printf("keys\n");
                break;
            default:
                break;
            }
        }
    }

    return (hInput)input;
}

int input_close(hInput h)
{
    input_t *input = (input_t*)h;
    if(!input)
        return -1;

    close(input->fd);
    free(input);

    return 0;
}

int input_setRxDataFunc(hInput h, cb_input_rxdata_func func, void *ctx)
{
    input_t *comm = (input_t *)h;
    if(!comm)
        return -1;
    comm->cbRxData = func;
    comm->ctx = ctx;

    return 0;
}

#if 0
static const char *key_event_name(unsigned int code)
{
    if(code < KEY_MAX && key_table[code] != NULL)
    {
        return key_table[code];
    }
    else
    {
        return "UNKNOWN";
    }
}
#endif
/**
struct input_event {

struct timeval time; //按键时间

__u16 type; //类型，在下面有定义

__u16 code; //要模拟成什么按键

__s32 value;//是按下还是释放

};

EV_KEY,键盘

EV_REL,相对坐标

EV_ABS,绝对坐标
*/

void *input_recv_task(hThread h,void *p)
{
    int ret,len;
    fd_set readfds;
    //int timeout = 1000;/* 1s */
    input_t *comm = NULL;
    int maxfd = 0;
   // struct timeval tm = {30,0};
    while(lapi_thread_isrunning(h))
    {
        FD_ZERO(&readfds);
        lapi_mutex_lock(input_priv.mtx);
        dl_list_for_each(comm,&input_priv.input_head,input_t,list)
        {
            if(comm)
            {
                FD_SET(comm->fd,&readfds);
                maxfd = MAX(maxfd,comm->fd);
            }
        }
        lapi_mutex_unlock(input_priv.mtx);

        //ret = select(maxfd + 1,&readfds,NULL,NULL,&tm);
        ret = select(maxfd + 1,&readfds,NULL,NULL,NULL);
        if(ret < 0)
        {
            if(errno == EINTR) /* 信号中断*/
            {
                continue;
            }
            break;
        }
        if(ret == 0)
        {
            /* timeout */
            continue;
        }

        lapi_mutex_lock(input_priv.mtx);

        dl_list_for_each(comm,&input_priv.input_head,input_t,list)
        {
            if(comm && FD_ISSET(comm->fd,&readfds))
            {

                len = read(comm->fd, &comm->msgbuffer, sizeof(comm->msgbuffer));
                if(len > 0)
                {
                    /* parser input event */
                    switch(comm->msgbuffer.type)
                    {
                    case EV_KEY:
                    {
                        if(comm->cbRxData)
                            comm->cbRxData((hInput)comm,comm->msgbuffer.code,comm->msgbuffer.value,comm->ctx);
                    }
                        break;
                    case EV_SW:
                        break;
                    case EV_ABS:
                    {
                        //if(comm->cbRxData)
                         //       comm->cbRxData((hInput)comm,comm->msgbuffer.code,comm->msgbuffer.value,comm->ctx);
                    }
                    default:
                        break;
                    }
                }
            }
        }

        lapi_mutex_unlock(input_priv.mtx);
    }

    return NULL;
}

int input_start(hInput h)
{
    input_t *comm = (input_t*)h;
    if(!comm)
        return -1;


    if(input_priv.first == 0)
    {
        dl_list_init(&input_priv.input_head);
        input_priv.mtx = lapi_mutex_create();
    }

    lapi_mutex_lock(input_priv.mtx);
    dl_list_add_tail(&input_priv.input_head,&comm->list);
    input_priv.ref++;
    lapi_mutex_unlock(input_priv.mtx);

    if(input_priv.first == 0)
    {
        input_priv.first = 1;
        input_priv.thread = lapi_thread_create(input_recv_task,NULL,1 << 10 <<10);
    }

    return 0;
}

int input_stop(hInput h)
{
    input_t *comm = (input_t*)h;
    if(!comm)
        return -1;


    lapi_mutex_lock(input_priv.mtx);

    dl_list_del(&comm->list);
    input_priv.ref--;
    if(input_priv.ref == 0)
    {
        lapi_thread_destroy(input_priv.thread);
        input_priv.thread = NULL;
        input_priv.first = 0;
    }

    lapi_mutex_unlock(input_priv.mtx);

    if(input_priv.ref == 0)
    {
        lapi_mutex_destroy(input_priv.mtx);
    }

    return 0;
}
