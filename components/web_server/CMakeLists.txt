idf_component_register(SRCS  "web_server.c" "root.c" "spotify.c" "api_screens.c" "api_upload.c" "api_custom_img.c"
                       INCLUDE_DIRS "include"
                       REQUIRES config esp_https_server esp_wifi esp_netif api_spotify ui cjson gfx
                       EMBED_TXTFILES "../../web_assets/index.html" "certs/cacert.pem" "certs/prvtkey.pem")