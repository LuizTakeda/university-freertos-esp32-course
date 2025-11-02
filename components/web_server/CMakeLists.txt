idf_component_register(
  SRCS "web_server.c"
  INCLUDE_DIRS "include"
  PRIV_REQUIRES esp_http_server
  EMBED_FILES ./frontend/dist/index.html ./frontend/dist/bundle.js
)
