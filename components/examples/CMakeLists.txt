idf_component_register(
  SRCS "task.c" "software_timer.c" "examples.c"
  INCLUDE_DIRS "include"
  PRIV_REQUIRES esp_driver_uart esp_driver_gpio
)
