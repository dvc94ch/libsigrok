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

#include <config.h>
#include <stdlib.h>
#include <string.h>
#include "protocol.h"
#include "scpi.h"

static struct sr_dev_driver redpitaya_driver_info;

static const uint32_t scanopts[] = {
	SR_CONF_CONN,
};

static const uint32_t drvopts[] = {
	SR_CONF_LOGIC_ANALYZER,
};

static const uint32_t devopts[] = {
	SR_CONF_CONTINUOUS,
	SR_CONF_SAMPLERATE | SR_CONF_GET,
};

static int config_list(uint32_t key, GVariant **data,
                       const struct sr_dev_inst *sdi,
                       const struct sr_channel_group *cg)
{
  switch (key) {
  case SR_CONF_SCAN_OPTIONS:
  case SR_CONF_DEVICE_OPTIONS:
    return STD_CONFIG_LIST(key, data, sdi, cg, scanopts, drvopts, devopts);
  default:
    sr_err("%d", key);
    return SR_ERR_NA;
  }

  return SR_OK;
}

static int config_get(uint32_t key, GVariant **data,
                      const struct sr_dev_inst *sdi,
                      const struct sr_channel_group *cg)
{
	struct dev_context *devc;

	(void)cg;

	if (!sdi)
		return SR_ERR_ARG;

	devc = sdi->priv;

	switch (key) {
	case SR_CONF_SAMPLERATE:
		*data = g_variant_new_uint64(devc->sample_rate);
		break;
	default:
		return SR_ERR_NA;
	}

	return SR_OK;
}

static int config_set(uint32_t key, GVariant *data,
                      const struct sr_dev_inst *sdi,
                      const struct sr_channel_group *cg)
{
  (void)key;
  (void)data;
  (void)sdi;
  (void)cg;
  return SR_OK;
}

static struct sr_dev_inst *probe_device(struct sr_scpi_dev_inst *scpi)
{
	struct dev_context *devc;
	struct sr_dev_inst *sdi;
	struct sr_scpi_hw_info *hw_info;
	struct sr_channel_group *cg;

	if (sr_scpi_get_hw_id(scpi, &hw_info) != SR_OK) {
		sr_info("Couldn't get IDN response.");
		return NULL;
	}

	if (strcmp(hw_info->manufacturer, "RedPitaya") != 0 ||
	    strcmp(hw_info->model, "RedPitaya") != 0) {
		sr_scpi_hw_info_free(hw_info);
		return NULL;
	}

	sdi = g_malloc0(sizeof(struct sr_dev_inst));
	sdi->vendor = g_strdup(hw_info->manufacturer);
	sdi->model = g_strdup(hw_info->model);
	sdi->version = g_strdup(hw_info->firmware_version);
	sdi->conn = scpi;
	sdi->driver = &redpitaya_driver_info;
	sdi->inst_type = SR_INST_SCPI;
	sdi->serial_num = g_strdup(hw_info->serial_number);
	sdi->channels = NULL;
	sdi->channel_groups = NULL;

	sr_scpi_hw_info_free(hw_info);

	devc = g_malloc0(sizeof(struct dev_context));
	devc->sample_rate = 12500.0;
	sdi->priv = devc;

	sr_channel_new(sdi, 0, SR_CHANNEL_LOGIC, TRUE, "DIN0");
	sr_channel_new(sdi, 1, SR_CHANNEL_LOGIC, TRUE, "DIN1");
	sr_channel_new(sdi, 2, SR_CHANNEL_LOGIC, TRUE, "DIN2");
	sr_channel_new(sdi, 3, SR_CHANNEL_LOGIC, TRUE, "DIN3");
	sr_channel_new(sdi, 4, SR_CHANNEL_LOGIC, TRUE, "DIN4");
	sr_channel_new(sdi, 5, SR_CHANNEL_LOGIC, TRUE, "DIN5");
	sr_channel_new(sdi, 6, SR_CHANNEL_LOGIC, TRUE, "DIN6");
	sr_channel_new(sdi, 7, SR_CHANNEL_LOGIC, TRUE, "DIN7");

	cg = g_malloc0(sizeof(struct sr_channel_group));
	cg->name = g_strdup("");
	cg->channels = g_slist_append(cg->channels, g_slist_nth_data(sdi->channels, 0));
	cg->channels = g_slist_append(cg->channels, g_slist_nth_data(sdi->channels, 1));
	cg->channels = g_slist_append(cg->channels, g_slist_nth_data(sdi->channels, 2));
	cg->channels = g_slist_append(cg->channels, g_slist_nth_data(sdi->channels, 3));
	cg->channels = g_slist_append(cg->channels, g_slist_nth_data(sdi->channels, 4));
	cg->channels = g_slist_append(cg->channels, g_slist_nth_data(sdi->channels, 5));
	cg->channels = g_slist_append(cg->channels, g_slist_nth_data(sdi->channels, 6));
	cg->channels = g_slist_append(cg->channels, g_slist_nth_data(sdi->channels, 7));
	cg->priv = NULL;
	sdi->channel_groups = g_slist_append(NULL, cg);

	return sdi;
}

static GSList *scan(struct sr_dev_driver *di, GSList *options)
{
  return sr_scpi_scan(di->context, options, probe_device);
}

static int dev_open(struct sr_dev_inst *sdi)
{
  if (sr_scpi_open(sdi->conn) != SR_OK)
    return SR_ERR;

  return SR_OK;
}

static int dev_close(struct sr_dev_inst *sdi)
{
  return sr_scpi_close(sdi->conn);
}

static int dev_acquisition_start(const struct sr_dev_inst *sdi)
{
	struct sr_scpi_dev_inst *scpi;

	scpi = sdi->conn;

	sr_scpi_source_add(sdi->session, scpi, G_IO_IN, 50,
                     redpitaya_receive_data, (void *)sdi);
  std_session_send_df_header(sdi);

	return SR_OK;
}

static int dev_acquisition_stop(struct sr_dev_inst *sdi)
{
	struct sr_scpi_dev_inst *scpi;

	scpi = sdi->conn;

  std_session_send_df_end(sdi);
	sr_scpi_source_remove(sdi->session, scpi);

	return SR_OK;
}

static struct sr_dev_driver redpitaya_driver_info = {
	.name = "redpitaya",
	.longname = "Red Pitaya",
	.api_version = 1,
	.init = std_init,
	.cleanup = std_cleanup,
	.scan = scan,
	.dev_list = std_dev_list,
	.dev_clear = std_dev_clear,
	.config_get = config_get,
	.config_set = config_set,
	.config_list = config_list,
	.dev_open = dev_open,
	.dev_close = dev_close,
	.dev_acquisition_start = dev_acquisition_start,
	.dev_acquisition_stop = dev_acquisition_stop,
	.context = NULL,
};
SR_REGISTER_DEV_DRIVER(redpitaya_driver_info);
