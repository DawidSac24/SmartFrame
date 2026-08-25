#include "cli.h"

#include "esp_console.h"
#include "esp_log.h"

void cli_init(void)
{
    esp_console_repl_t *repl = NULL;
    esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
    repl_config.prompt = "SmartFrame >";

    // Because you enabled USB in menuconfig, the compiler now knows what this is!
    esp_console_dev_usb_serial_jtag_config_t hw_config = ESP_CONSOLE_DEV_USB_SERIAL_JTAG_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_console_new_repl_usb_serial_jtag(&hw_config, &repl_config, &repl));

    ESP_ERROR_CHECK(esp_console_start_repl(repl));

    ESP_LOGI("CLI", "Terminal Engine Started on Native USB!");
}