#pragma once
enum custom_img_state
{
    CUSTOM_IMG_NONE = 0,
    CUSTOM_IMG_NEW_FILE,
    CUSTOM_IMG_DECODED,
    CUSTOM_IMG_FAILED
};
void custom_img_set_state(enum custom_img_state state);
enum custom_img_state custom_img_get_state(void);