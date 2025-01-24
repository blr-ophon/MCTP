#ifndef MCTP_TASK_H
#define MCTP_TASK_H

#include <stdint.h>
#include <string.h>
#include "mctp_parser.h"
#include "mctp_api.h"

/*
 * Update MCTP communication task finite state machine.
 *
 * Returns 0 on success and -1 on error.
 */
int MCTP_updateTask(MCTP_Handle *hmctp, E_MCTP_TaskEvent event);

#endif
