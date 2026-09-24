#include "api_custom_img.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

static enum custom_img_state s_img_state = CUSTOM_IMG_NONE;
static SemaphoreHandle_t s_mutex = NULL;

void custom_img_set_state(enum custom_img_state state)
{
    if (!s_mutex)
        s_mutex = xSemaphoreCreateMutex();
    if (xSemaphoreTake(s_mutex, portMAX_DELAY))
    {
        s_img_state = state;
        xSemaphoreGive(s_mutex);
    }
}
enum custom_img_state custom_img_get_state(void)
{
    if (!s_mutex)
        s_mutex = xSemaphoreCreateMutex();
    enum custom_img_state state = CUSTOM_IMG_NONE;
    if (xSemaphoreTake(s_mutex, portMAX_DELAY))
    {
        state = s_img_state;
        xSemaphoreGive(s_mutex);
    }
    return state;
}