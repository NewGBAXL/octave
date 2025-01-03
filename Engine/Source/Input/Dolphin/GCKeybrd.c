#if PLATFORM_DOLPHIN

#include <gctypes.h>
//#include "GCKeymap.h"
#include "GCKeybrd.h"

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
static u32 __kybd_keysbuffer[PAD_CHANMAX];

static void SI_AwaitPendingCommands(void) {
    while(*SICOMCSR & 0x1);
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

//use this instead of GCKB_Detect
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

u32 GCKB_Read(KYBDStatus *status) {
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

	u32 buffer[2] = { 0, 0 };
    if (SI_GetResponse(pad, buffer)) {
        return (buffer[1] & 0xFFFFFF00) >> 8;
    }
    return 0;
}

/*char GCKB_GetMap(u8 key, int isShiftHeld) {
    char map[] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l',
        'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
        '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^', '\\', '@', '[', ';', ':', ']', ',', '.', '/', '\\' };
    
    char mapShift[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',
		'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
		'!', '"','#','$','%','&','\'','(',')','~','=','|', '`','{', '+', '*', '}', '<', '>', '?', '_' };
	
    if (key == KEY_SPACE) {
		return ' ';
	}
	else if (key >= KEY_A && key <= KEY_BACKSLASH) {
		return isShiftHeld ? mapShift[key - KEY_A] : map[key - KEY_A];
	}
	else {
		return '\0';
	}
}*/

#endif
