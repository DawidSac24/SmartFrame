#include "screens_init.h"

#include "screen_scheduler.h"
#include "clock_screen.h"
#include "spotify_screen.h"
#include "weather_screen.h"
#include "custom_img_screen.h"

#include <string.h>
#include <stddef.h>

void screens_init_defaults(void)
{
    // 1. Add the screens you DO want to see on boot
    sched_add_screen(screen_clock_init(5000));
    sched_add_screen(screen_weather_init(5000));

    // 2. Initialize the custom image screen so it exists in memory
    struct screen *custom = screen_custom_img_init(5000);

    // 3. Add it to register it in the master list, then instantly remove it
    // from the active rotation!
    sched_add_screen(custom);
    sched_remove_screen(custom);
}

struct screen *screens_create_by_name(const char *name, uint32_t duration_ms)
{
    // This acts as a factory for the `ui add <target>` command
    if (strcmp(name, "clock") == 0)
    {
        return screen_clock_init(duration_ms);
    }
    else if (strcmp(name, "spotify") == 0)
    {
        return screen_spotify_init(duration_ms);
    }
    else if (strcmp(name, "weather") == 0)
    {
        return screen_weather_init(duration_ms);
    }
    else if (strcmp(name, "custom_img") == 0)
    {
        return screen_custom_img_init(duration_ms);
    }

    return NULL; // Return NULL if the string doesn't match any known screen
}