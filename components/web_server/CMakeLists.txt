idf_component_register(SRCS  "web_server.c" "root.c" "spotify.c"
                       INCLUDE_DIRS "include"
                       REQUIRES config esp_https_server esp_wifi esp_netif api_spotify
                       EMBED_TXTFILES "../../web_assets/index.html" "../../web_assets/spotify.html" "certs/cacert.pem" "certs/prvtkey.pem")