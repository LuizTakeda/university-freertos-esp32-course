#pragma once

#include "digital_input.h"

//**************************************************
// Typedefs
//**************************************************

typedef struct digital_input_internal_handler_node_t
{
  digital_input_event_handler_t event_handler;
  struct digital_input_internal_handler_node_t *next;
} digital_input_internal_handler_node_t;
