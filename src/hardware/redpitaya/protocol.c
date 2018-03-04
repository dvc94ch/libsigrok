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

#include "protocol.h"

static int read_data(struct sr_dev_inst *sdi,
                     struct sr_scpi_dev_inst *scpi,
                     struct dev_context *devc,
                     int buffer_size)
{
	int bytes_read;

  bytes_read = sr_scpi_read_data(scpi, devc->rcv_buffer, buffer_size);

	if (bytes_read < 0) {
		sr_err("Read data error.");
		sr_dev_acquisition_stop(sdi);
    devc->cur_rcv_buffer_position = 0;
		return SR_ERR;
	}

	devc->cur_rcv_buffer_position = bytes_read;

  return SR_OK;
}

SR_PRIV int redpitaya_receive_data(int fd, int revents, void *cb_data) {
  sr_info("redpitaya_receive_data");
	struct sr_dev_inst *sdi;
	struct sr_scpi_dev_inst *scpi;
	struct dev_context *devc;
	struct sr_datafeed_packet packet;
  struct sr_datafeed_logic payload;

	(void)fd;
  (void)revents;

	if (!(sdi = cb_data))
		return TRUE;

	if (!(devc = sdi->priv))
		return TRUE;

	scpi = sdi->conn;

	if (!(revents == G_IO_IN || revents == 0))
		return TRUE;


  if (sr_scpi_send(scpi, "\x3") != SR_OK) {
    sr_err("Failed to start data acquisition");
    sr_dev_acquisition_stop(sdi);
    return TRUE;
  }

  if (read_data(sdi, scpi, devc, MAX_RCV_BUFFER_SIZE) < 0) {
    sr_err("Error reading buffer");
    sr_dev_acquisition_stop(sdi);
    return TRUE;
  }

  payload.unitsize = 1;
  payload.data = devc->rcv_buffer;
  payload.length = devc->cur_rcv_buffer_position;

  packet.type = SR_DF_LOGIC;
  packet.payload = &payload;
  sr_session_send(sdi, &packet);

  return TRUE;
}
