idf_component_register(
  SRCS "main.c"
  INCLUDE_DIRS "."
  PRIV_REQUIRES nvs_flash wifi web_server
)