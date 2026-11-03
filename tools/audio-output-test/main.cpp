#include <bass.h>
#include <cstdio>

int main()
{
    if (!BASS_SetConfig(BASS_CONFIG_DEV_DEFAULT, TRUE) || !BASS_SetConfig(BASS_CONFIG_UNICODE, TRUE)) return 1;
    BASS_DEVICEINFO device = {};
    for (DWORD index = 0; BASS_GetDeviceInfo(index, &device); ++index)
        std::printf("device=%lu name=%s enabled=%i\n", index, device.name, (device.flags & BASS_DEVICE_ENABLED) != 0);
    if (!BASS_Init(-1, 44100, BASS_DEVICE_NOSPEAKER, nullptr, nullptr)) return 2;
    const bool initial = BASS_IsStarted();
    const bool paused = BASS_Pause() && !BASS_IsStarted();
    const bool resumed = BASS_Start() && BASS_IsStarted();
    BASS_Free();
    std::printf("initial=%i paused=%i resumed=%i\n", initial, paused, resumed);
    return initial && paused && resumed ? 0 : 3;
}
