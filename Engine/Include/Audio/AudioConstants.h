#pragma once

#if PLATFORM_WINDOWS
#define AUDIO_MAX_VOICES 8
#elif PLATFORM_LINUX
#define AUDIO_MAX_VOICES 8
#elif PLATFORM_DOLPHIN
#define AUDIO_MAX_VOICES 8
#elif PLATFORM_3DS
#define AUDIO_MAX_VOICES 8
#endif