/*
 * This file is part of the libsigrok project.
 *
 * Copyright (C) 2018 dvc94ch (David Craven) <david@craven.ch>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LIBSIGROK_HARDWARE_REDPITAYA_PROTOCOL_H
#define LIBSIGROK_HARDWARE_REDPITAYA_PROTOCOL_H

#include <libsigrok/libsigrok.h>
#include "libsigrok-internal.h"
#include "scpi.h"

#define LOG_PREFIX "redpitaya"

#define MAX_RCV_BUFFER_SIZE 0x4000

struct dev_context {
  float sample_rate;
	gboolean df_started;
  int cur_rcv_buffer_position;
	char rcv_buffer[MAX_RCV_BUFFER_SIZE];
};

SR_PRIV int redpitaya_receive_data(int fd, int revents, void *cb_data);

#endif
