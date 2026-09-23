#include "screens_init.h"

#include "screen_scheduler.h"
#include "clock_screen.h"
#include "spotify_screen.h"
#include "weather_screen.h"

#include <string.h>
#include <stddef.h>

void screens_init_defaults(void)
{
    sched_add_screen(screen_clock_init(5000));
    sched_add_screen(screen_weather_init(5000));
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
    /*
    else if (strcmp(name, "weather") == 0) {
        return screen_weather_init(duration_ms);
    }
    */

    return NULL; // Return NULL if the string doesn't match any known screen
}