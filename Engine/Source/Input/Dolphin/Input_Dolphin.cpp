#if PLATFORM_DOLPHIN

#include "Input/Input.h"
#include "Input/InputUtils.h"

#include "Engine.h"
#include "Log.h"
#include "Maths.h"

#if PLATFORM_WII
#include <wiiuse/wpad.h>
#endif

#include <gctypes.h>

typedef struct _gckbstatus {
    u32 key;
    s8 err;
} KYBDStatus;

u32 GCKB_Init(void);
u32 GCKB_Init(int chan);
u32 GCKB_ScanKybd(void);
u32 GCKB_ScanPads(void);
u32 GCKB_Read(KYBDStatus* status);
u32 GCKB_ReadKeys(int pad);

void INP_Initialize()
{
#if PLATFORM_WII
    PAD_Init();
    WPAD_Init();

    WPAD_SetVRes(0, 640, 480);
    
    for (uint32_t i = 0; i < INPUT_MAX_GAMEPADS; ++i)
    {
        WPAD_SetDataFormat(WPAD_CHAN_0 + i, WPAD_FMT_BTNS_ACC_IR);

        InputState& input = GetEngineState()->mInput;
        input.mGamepads[i].mType = GamepadType::Wiimote;
    }
#else
    PAD_Init();
    //GCKB_Init();
#endif
}

void INP_Shutdown()
{
    
}

void INP_Update()
{
    InputAdvanceFrame();

    InputState& input = GetEngineState()->mInput;

#if PLATFORM_WII
    WPAD_ScanPads();

    for (uint32_t i = 0; i < INPUT_MAX_GAMEPADS; ++i)
    {
        WPADData* data = WPAD_Data(i);

        input.mGamepads[i].mConnected = (data->err == WPAD_ERR_NONE);

        struct expansion_t expData;
        WPAD_Expansion(i, &expData); // Get expansion info from the first wiimote

        if (expData.type == WPAD_EXP_CLASSIC)
        {
            int32_t held = expData.classic.btns_held;

            if (held != 0)
            {
                input.mGamepads[i].mType = GamepadType::WiiClassic;
            }

            if (input.mGamepads[i].mType != GamepadType::WiiClassic)
            {
                continue;
            }

            // Buttons
            input.mGamepads[i].mButtons[GAMEPAD_A] = held & CLASSIC_CTRL_BUTTON_A;
            input.mGamepads[i].mButtons[GAMEPAD_B] = held & CLASSIC_CTRL_BUTTON_B;
            input.mGamepads[i].mButtons[GAMEPAD_X] = held & CLASSIC_CTRL_BUTTON_X;
            input.mGamepads[i].mButtons[GAMEPAD_Y] = held & CLASSIC_CTRL_BUTTON_Y;
            input.mGamepads[i].mButtons[GAMEPAD_Z] = held & CLASSIC_CTRL_BUTTON_ZL;
            input.mGamepads[i].mButtons[GAMEPAD_R1] = held & CLASSIC_CTRL_BUTTON_FULL_R;
            input.mGamepads[i].mButtons[GAMEPAD_L1] = held & CLASSIC_CTRL_BUTTON_FULL_L;
            input.mGamepads[i].mButtons[GAMEPAD_LEFT] = held & CLASSIC_CTRL_BUTTON_LEFT;
            input.mGamepads[i].mButtons[GAMEPAD_RIGHT] = held & CLASSIC_CTRL_BUTTON_RIGHT;
            input.mGamepads[i].mButtons[GAMEPAD_DOWN] = held & CLASSIC_CTRL_BUTTON_DOWN;
            input.mGamepads[i].mButtons[GAMEPAD_UP] = held & CLASSIC_CTRL_BUTTON_UP;
            input.mGamepads[i].mButtons[GAMEPAD_START] = held & CLASSIC_CTRL_BUTTON_PLUS;
            input.mGamepads[i].mButtons[GAMEPAD_SELECT] = held & CLASSIC_CTRL_BUTTON_MINUS;
            input.mGamepads[i].mButtons[GAMEPAD_HOME] = held & CLASSIC_CTRL_BUTTON_HOME;

            // Axes
            int32_t leftStickX = int32_t(expData.classic.ljs.pos.x - expData.classic.ljs.center.x);
            int32_t leftStickY = int32_t(expData.classic.ljs.pos.y - expData.classic.ljs.center.y);
            int32_t rightStickX = int32_t(expData.classic.rjs.pos.x - expData.classic.rjs.center.x);
            int32_t rightStickY = int32_t(expData.classic.rjs.pos.y - expData.classic.rjs.center.y);
            float leftTrigger = expData.classic.l_shoulder;
            float rightTrigger = expData.classic.r_shoulder;

            input.mGamepads[i].mAxes[GAMEPAD_AXIS_LTHUMB_X] = glm::clamp(leftStickX / 31.0f, -1.0f, 1.0f);
            input.mGamepads[i].mAxes[GAMEPAD_AXIS_LTHUMB_Y] = glm::clamp(leftStickY / 31.0f, -1.0f, 1.0f);
            input.mGamepads[i].mAxes[GAMEPAD_AXIS_RTHUMB_X] = glm::clamp(rightStickX / 15.0f, -1.0f, 1.0f);
            input.mGamepads[i].mAxes[GAMEPAD_AXIS_RTHUMB_Y] = glm::clamp(rightStickY / 15.0f, -1.0f, 1.0f);
            input.mGamepads[i].mAxes[GAMEPAD_AXIS_LTRIGGER] = leftTrigger;
            input.mGamepads[i].mAxes[GAMEPAD_AXIS_RTRIGGER] = rightTrigger;
        }
        else
        {
            int32_t held = WPAD_ButtonsHeld(i);

            if (held != 0)
            {
                input.mGamepads[i].mType = GamepadType::Wiimote;
            }

            if (input.mGamepads[i].mType != GamepadType::Wiimote)
            {
                continue;
            }

            // Buttons
            input.mGamepads[i].mButtons[GAMEPAD_A] = held & WPAD_BUTTON_A;
            input.mGamepads[i].mButtons[GAMEPAD_B] = held & WPAD_BUTTON_B;
            input.mGamepads[i].mButtons[GAMEPAD_X] = held & WPAD_BUTTON_1;
            input.mGamepads[i].mButtons[GAMEPAD_Y] = held & WPAD_BUTTON_2;
            input.mGamepads[i].mButtons[GAMEPAD_Z] = held & WPAD_NUNCHUK_BUTTON_Z;
            input.mGamepads[i].mButtons[GAMEPAD_C] = held & WPAD_NUNCHUK_BUTTON_C;
            input.mGamepads[i].mButtons[GAMEPAD_LEFT] = held & WPAD_BUTTON_LEFT;
            input.mGamepads[i].mButtons[GAMEPAD_RIGHT] = held & WPAD_BUTTON_RIGHT;
            input.mGamepads[i].mButtons[GAMEPAD_DOWN] = held & WPAD_BUTTON_DOWN;
            input.mGamepads[i].mButtons[GAMEPAD_UP] = held & WPAD_BUTTON_UP;
            input.mGamepads[i].mButtons[GAMEPAD_START] = held & WPAD_BUTTON_PLUS;
            input.mGamepads[i].mButtons[GAMEPAD_SELECT] = held & WPAD_BUTTON_MINUS;
            input.mGamepads[i].mButtons[GAMEPAD_HOME] = held & WPAD_BUTTON_HOME;

            struct expansion_t expData;
            WPAD_Expansion(i, &expData); // Get expansion info from the first wiimote

            if (expData.type == WPAD_EXP_NUNCHUK)
            {
                int32_t stickX = int32_t(expData.nunchuk.js.pos.x - expData.nunchuk.js.center.x);
                int32_t stickY = int32_t(expData.nunchuk.js.pos.y - expData.nunchuk.js.center.y);

                // Axes
                input.mGamepads[i].mAxes[GAMEPAD_AXIS_LTHUMB_X] = glm::clamp(stickX / 127.0f, -1.0f, 1.0f);
                input.mGamepads[i].mAxes[GAMEPAD_AXIS_LTHUMB_Y] = glm::clamp(stickY / 127.0f, -1.0f, 1.0f);
            }

            // IR support
            struct ir_t irData;
            WPAD_IR(i, &irData);
            INP_SetTouchPosition((int32_t)irData.x, (int32_t)irData.y, i);
        }
    }

#endif

    PADStatus pads[4];
    //PAD_ScanPads();
    PAD_Read(pads);

    for (uint32_t i = 0; i < INPUT_MAX_GAMEPADS; ++i)
    {
        //input.mGamepads[i].mConnected = (pads[i].err == PAD_ERR_NONE);
        input.mGamepads[i].mConnected = true;

        switch (SI_GetType(i)) {
            case SI_GC_CONTROLLER:
            case SI_GC_RECEIVER:
            case SI_GC_WAVEBIRD:
                input.mGamepads[i].mType = GamepadType::GameCube;
                break;
            //can't find a way to find out if it's a bongo or a controller
            /*case SI_GC_CONTROLLER:
                input.mGamepads[i].mType = GamepadType::GCNBongos;*/
            case (SI_GC_CONTROLLER | 0x00000300):
                //input.mGamepads[i].mType = GamepadType::GCNMat;
                input.mGamepads[i].mType = GamepadType::GameCube;
                break;
            case SI_GC_KEYBOARD:
                input.mGamepads[i].mType = GamepadType::GCNKeyboard;
                input.mGamepads[i].mConnected = false;
                break;
            case SI_GC_STEERING:
                input.mGamepads[i].mType = GamepadType::GCNSteering;
                break;
            case SI_GBA:
                input.mGamepads[i].mType = GamepadType::GCNGBA;
                break;
            default:
                input.mGamepads[i].mConnected = false;
                break;
        }

#if PLATFORM_WII
        if (pads[i].button != 0)
        {
            input.mGamepads[i].mType = GamepadType::GameCube;
        }

        if (input.mGamepads[i].mType != GamepadType::GameCube)
        {
            continue;
        }
#else
        if (input.mGamepads[i].mType == GamepadType::GCNKeyboard)
        {
            u32 keys = 0;
            u8* key = (u8*)&keys;

            keys = GCKB_ReadKeys(i);
            INP_ClearAllKeys();
            if (keys == 0)
            {
                continue;
            }
            INP_SetKey(key[0]);
            INP_SetKey(key[1]);
            INP_SetKey(key[2]);
            continue;
        }
#endif

        // Buttons
        input.mGamepads[i].mButtons[GAMEPAD_A] = pads[i].button & PAD_BUTTON_A;
        input.mGamepads[i].mButtons[GAMEPAD_B] = pads[i].button & PAD_BUTTON_B;
        input.mGamepads[i].mButtons[GAMEPAD_X] = pads[i].button & PAD_BUTTON_X;
        input.mGamepads[i].mButtons[GAMEPAD_Y] = pads[i].button & PAD_BUTTON_Y;
        input.mGamepads[i].mButtons[GAMEPAD_Z] = pads[i].button & PAD_TRIGGER_Z;
        input.mGamepads[i].mButtons[GAMEPAD_R1] = pads[i].button & PAD_TRIGGER_R;
        input.mGamepads[i].mButtons[GAMEPAD_L1] = pads[i].button & PAD_TRIGGER_L;
        input.mGamepads[i].mButtons[GAMEPAD_LEFT] = pads[i].button & PAD_BUTTON_LEFT;
        input.mGamepads[i].mButtons[GAMEPAD_RIGHT] = pads[i].button & PAD_BUTTON_RIGHT;
        input.mGamepads[i].mButtons[GAMEPAD_DOWN] = pads[i].button & PAD_BUTTON_DOWN;
        input.mGamepads[i].mButtons[GAMEPAD_UP] = pads[i].button & PAD_BUTTON_UP;
        input.mGamepads[i].mButtons[GAMEPAD_START] = pads[i].button & PAD_BUTTON_START;
        input.mGamepads[i].mButtons[GAMEPAD_HOME] = 0;
        input.mGamepads[i].mButtons[GAMEPAD_PEDALCONNECT] = pads[i].button & 0x0080; //(ZL)

        // Axes
        input.mGamepads[i].mAxes[GAMEPAD_AXIS_LTHUMB_X] = glm::clamp(pads[i].stickX / 127.0f, -1.0f, 1.0f);
        input.mGamepads[i].mAxes[GAMEPAD_AXIS_LTHUMB_Y] = glm::clamp(pads[i].stickY / 127.0f, -1.0f, 1.0f);
        input.mGamepads[i].mAxes[GAMEPAD_AXIS_RTHUMB_X] = glm::clamp(pads[i].substickX / 127.0f, -1.0f, 1.0f);
        input.mGamepads[i].mAxes[GAMEPAD_AXIS_RTHUMB_Y] = glm::clamp(pads[i].substickY / 127.0f, -1.0f, 1.0f);
        input.mGamepads[i].mAxes[GAMEPAD_AXIS_LTRIGGER] = pads[i].triggerL / 255.0f;
        input.mGamepads[i].mAxes[GAMEPAD_AXIS_RTRIGGER] = pads[i].triggerR / 255.0f;


    }
}

void INP_SetCursorPos(int32_t x, int32_t y)
{
    
}

void INP_ShowCursor(bool show)
{

}

void INP_LockCursor(bool lock)
{

}

void INP_TrapCursor(bool trap)
{

}

void INP_ShowSoftKeyboard(bool show)
{

}

bool INP_IsSoftKeyboardShown()
{
    return false;
}

//GCKeybrd.c
//#include <gctypes.h>
//#include "GCKeymap.h"
//#include "GCKeybrd.h"

#if defined(HW_DOL)
#define SI_REG_BASE 0xCC006400
#elif defined(HW_RVL)
#define SI_REG_BASE 0xCD006400
#else
#error Hardware model unknown? Missing a preprocessor definition somewhere...
#endif

#define SIREG(n)               ((vu32*)(SI_REG_BASE + (n)))

#define SICOUTBUF(n)           (SIREG(0x00 + (n)*12))               /* SI Channel n Output Buffer (Joy-channel n Command) (4 bytes) */
#define SICINBUFH(i)           (SIREG(0x04 + (i)*12))               /* SI Channel n Input Buffer High (Joy-channel n Buttons 1) (4 bytes) */
#define SICINBUFL(i)           (SIREG(0x08 + (i)*12))               /* SI Channel n Input Buffer Low (Joy-channel n Buttons 2) (4 bytes) */
#define SIPOLL                 (SIREG(0x30))                        /* SI Poll Register (4 bytes) */
#define SICOMCSR               (SIREG(0x34))                        /* SI Communication Control Status Register (command) (4 bytes) */
#define SISR                   (SIREG(0x38))                        /* SI Status Register (4 bytes) */
#define SIIOBUF                (SIREG(0x80))                        /* SI I/O buffer (access by word) (128 bytes) */

#define PAD_ENABLEDMASK(chan)  (0x80000000 >> chan)

#define PAD_CHAN0 0
#define PAD_CHAN1 1
#define PAD_CHAN2 2
#define PAD_CHAN3 3
#define PAD_CHANMAX	4

#define PAD_ERR_NONE 0
#define PAD_ERR_NO_CONTROLLER -1

static u32 __kybd_initialized = 0;
static u32 __kybd_initialized_chan[4] = { 0, 0, 0, 0 };
static u32 __kybd_keysbuffer[PAD_CHANMAX];

static void SI_AwaitPendingCommands(void) {
    while (*SICOMCSR & 0x1);
}

u32 GCKB_Init(void) {
    u32 chan;
    u32 buf[2];

    if (__kybd_initialized) return 1;

    chan = 0;
    while (chan < 4) {
        SI_GetResponse(chan, buf);
        SI_SetCommand(chan, 0x00540000);
        SI_EnablePolling(PAD_ENABLEDMASK(chan));
        SI_TransferCommands();
        SI_AwaitPendingCommands();
        ++chan;
    }

    __kybd_initialized = 1;
    return 1;
}

u32 GCKB_Init(int chan) {
    u32 buf[2];

    if (__kybd_initialized_chan[chan]) return 1;

    SI_GetResponse(chan, buf);
    SI_SetCommand(chan, 0x00540000);
    SI_EnablePolling(PAD_ENABLEDMASK(chan));
    SI_TransferCommands();
    SI_AwaitPendingCommands();


	__kybd_initialized_chan[chan] = 1;
    return 1;
}

u32 GCKB_ScanKybd(void) {
    u32 connected = 0;
    KYBDStatus kybdstatus[PAD_CHANMAX];

    SI_AwaitPendingCommands();

    u32 buf[2];
    for (int i = 0; i < PAD_CHANMAX; ++i) {
        SI_GetResponse(i, buf);
        SI_SetCommand(i, 0x00400300);
        SI_EnablePolling(PAD_ENABLEDMASK(i));
        SI_TransferCommands();
    }
    SI_AwaitPendingCommands();

    GCKB_Read(kybdstatus);
    for (int i = 0; i < PAD_CHANMAX; ++i) {
        if (kybdstatus[i].err == PAD_ERR_NONE) {
            //kybdstatus[i].key = GCKB_ReadKeys(i);
            __kybd_keysbuffer[i] = GCKB_ReadKeys(i);
            connected |= (1 << i);
        }
    }

    /*for (int i = 0; i < 4; ++i) {
        u32 type = SI_DecodeType(SI_GetType(i));
        if (type == SI_GC_KEYBOARD)
            connected |= (1 << i);
    }*/

    //todo: deal with error handling
    //GCKB_ScanPads() already deals with error handling,
    //since it calls PAD_ScanPads() internally

    return connected;
}

u32 GCKB_ScanPads(void) {
    return PAD_ScanPads() | GCKB_ScanKybd();
}

u32 GCKB_Read(KYBDStatus* status) {
    GCKB_Init();
    u32 connected = GCKB_ScanKybd(); //which one do we want to use?
    for (int chan = 0; chan < 4; ++chan) {
        if (connected & (1 << chan)) {
            status[chan].err = PAD_ERR_NONE;
        }
        else {
            status[chan].err = PAD_ERR_NO_CONTROLLER;
        }
    }
    return 1;
}

u32 GCKB_ReadKeys(int pad) {
    if (pad<PAD_CHAN0 || pad>PAD_CHAN3) return 0;
	GCKB_Init(pad);

    u32 buffer[2] = { 0, 0 };
    if (SI_GetResponse(pad, buffer)) {
        return (buffer[1] & 0xFFFFFF00) >> 8;
    }
    return 0;
}

#endif