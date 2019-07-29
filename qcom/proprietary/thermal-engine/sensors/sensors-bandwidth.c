/*===========================================================================
  copyright (c) 2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sensors_manager_internal.h"
#include "thermal.h"
#include "sensors-bandwidth.h"
#include "thermal_server.h"

struct bw_data {
	pthread_mutex_t bw_mutex;
	pthread_cond_t bw_condition;
	int threshold_reached;
	int value;
	struct sensor_info *sensor;
};

static int camera_hal_callback(int val, void *data, void *reserved)
{
	struct bw_data *bw = (struct bw_data *)data;

	if (bw == NULL) {
		msg("%s: unexpected NULL", __func__);
		return -1;
	}

	dbgmsg("%s: received %d value from client\n", __func__, val);

	/* notify the waiting threads */
	pthread_mutex_lock(&(bw->bw_mutex));
	if(bw->value == val) {
		pthread_mutex_unlock(&(bw->bw_mutex));
		return 0;
	}
	bw->value = val;
	bw->threshold_reached = 1;
	pthread_cond_broadcast(&(bw->bw_condition));
	pthread_mutex_unlock(&(bw->bw_mutex));

	return 0;
}

int bw_sensors_setup(struct sensor_info *sensor)
{
	struct bw_data *bw = NULL;

	if (sensor == NULL) {
		msg("%s: Invalid sensor pointer\n", __func__);
		return 0;
	}

	bw = (struct bw_data *) malloc(sizeof(struct bw_data));
	if (bw == NULL) {
		msg("%s: malloc failed", __func__);
		return 0;
	}

	memset(bw, 0, sizeof(struct bw_data));
	sensor->data = (void *) bw;
	pthread_mutex_init(&(bw->bw_mutex), NULL);
	pthread_cond_init(&(bw->bw_condition), NULL);
	bw->threshold_reached = 0;
	bw->sensor = sensor;

#ifdef ENABLE_CAMERA_REG_BW_CALLBACK
	thermal_server_register_client_req_handler("camera", camera_hal_callback, bw);
#else
	thermal_server_register_client_req_handler(sensor->name, camera_hal_callback, bw);
#endif

	return 1;
}

void bw_sensors_shutdown(struct sensor_info *sensor)
{
	struct bw_data *bw;

	if (NULL == sensor ||
	    NULL == sensor->data) {
		msg("%s: unexpected NULL", __func__);
		return;
	}
	bw = (struct bw_data *) sensor->data;
	sensor->data = NULL;
	free(bw);
}

int bw_sensor_get_temperature(struct sensor_info *sensor)
{
	int temp = 0;
	struct bw_data *bw;

	if (NULL == sensor) {
		msg("%s: unexpected NULL", __func__);
		return 0;
	}
	pthread_mutex_lock(&(sensor->read_mtx));
	bw = (struct bw_data *) sensor->data;
	temp = bw->value;
	pthread_mutex_unlock(&(sensor->read_mtx));

	return temp;
}

void bw_sensor_interrupt_wait(struct sensor_info *sensor)
{
	struct bw_data *bw;

	if (NULL == sensor ||
	    NULL == sensor->data) {
		msg("%s: unexpected NULL", __func__);
		return;
	}
	if (sensor->interrupt_enable) {
		bw = (struct bw_data *) sensor->data;
		/* Wait for sensor threshold condition */
		pthread_mutex_lock(&(bw->bw_mutex));
		while (!bw->threshold_reached) {
			pthread_cond_wait(&(bw->bw_condition),
					&(bw->bw_mutex));
		}
		bw->threshold_reached = 0;
		pthread_mutex_unlock(&(bw->bw_mutex));
	}
}
