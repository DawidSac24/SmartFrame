idf_component_register(SRCS "ui_cmd.c" "ui_task.c" "screen_scheduler.c"
                       INCLUDE_DIRS "include" 
                       REQUIRES memory collections esp_timer gfx api_spotify console)