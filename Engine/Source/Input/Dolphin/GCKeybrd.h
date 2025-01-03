#if PLATFORM_DOLPHIN

#include <gccore.h>
#include <gctypes.h>

//#include "GCKeymap.h"

typedef struct _gckbstatus {
	u32 key;
	s8 err;
} KYBDStatus;

/**
 * Initializes the keyboard controller, previously detected on the given SI channel. This must be called before
 * key press information can be read. Returns 1 on success, 0 on failure.
 */
u32 GCKB_Init(void);

/**
* Scans all controller ports for a keyboard controller, and sets .err to PAD_ERR_NONE to all present controllers.
* Don't bother reading return value. Included for compatibility with libogc/pad.c.
*/
u32 GCKB_Read(KYBDStatus* status);

/**
 * Attempts to detect the presence of a GC Keyboard Controller connected to any of the controller ports.
 * Returns all SI channels of present keyboard controllers, or 0 if one could not be found.
 * Additionally, updates keypress information for all present controllers.
 * GCKB_ScanPads() also finds present standard controllers and values, and includes proper error handling.
 */
u32 GCKB_ScanKybd(void);
u32 GCKB_ScanPads(void);

/**
 * Reads current key press information from the previously initialized keyboard controller, located on the given
 * SI channel. Returns 1 if key press data has been returned, or 0 on failure.
 *
 * The pressedKeys buffer passed in should be large enough to hold 3 bytes. Each byte will correspond to one key
 * pressed. The keyboard controller can only recognize 3 simultaneous key presses at a time (and, depending on the
 * specific keys, probably only 2 at a time). If too many keys are held down, all of the values returned will be 0x01
 * or 0x02. A value of 0x00 indicates no key press.
 */
u32 GCKB_ReadKeys(int chan);

/**
 * Returns the character corresponding to the given key press, with the given shift state. If the key press is not
 * recognized, or if the shift state is not valid, a null character is returned.
 */
//char GCKB_GetMap(u8 key, int isShiftHeld);
//DEPRECATED for octave

#endif
