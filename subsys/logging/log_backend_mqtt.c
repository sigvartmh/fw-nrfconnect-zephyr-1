/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <logging/log.h>
LOG_MODULE_REGISTER(log_backend_mqtt, CONFIG_LOG_DEFAULT_LEVEL);

#include <logging/log_backend.h>
#include <logging/log_core.h>
#include <logging/log_output.h>
#include <logging/log_msg.h>

#include <net/mqtt.h>
#include <kernel.h>

/* Set this to 1 if you want to see what is being sent to server */
#define DEBUG_PRINTING 0
#define LOG_TOPIC "$aws/things/aws_fota/log/output"
static bool panic_mode;
static bool connected;

static u8_t send_buf[CONFIG_LOG_BACKEND_MQTT_MAX_BUF_SIZE];
const struct log_backend *log_backend_mqtt_get(void);

static int line_out(u8_t *data, size_t length, void *output_ctx)
{
	struct mqtt_client *ctx = (struct mqtt_client *)output_ctx;
	int ret = -ENOMEM;

	if (ctx == NULL) {
		return length;
	}


	struct mqtt_topic topic = {
		.topic.size = sizeof(LOG_TOPIC),
		.topic.utf8 = LOG_TOPIC,
		.qos = MQTT_QOS_1_AT_LEAST_ONCE,
	};

	struct mqtt_publish_param param = {
		.message.topic = topic,
		.message.payload.data = data,
		.message.payload.len = length,
		.message_id = sys_rand32_get(),
		.dup_flag = 0,
		.retain_flag = 0,
	};

	ret = mqtt_publish(ctx, &param);
	if (ret < 0) {
		goto fail;
	}

fail:
	return length;
}

LOG_OUTPUT_DEFINE(log_output, line_out,  send_buf, sizeof(send_buf));
static void send_output(const struct log_backend *const backend,
			struct log_msg *msg)
{
	if (panic_mode | !connected) {
		return;
	}
	/* Add json formatting */

	log_msg_get(msg);
	u32_t level = log_msg_level_get(msg);
	u32_t flags = IS_ENABLED(CONFIG_LOG_BACKEND_MQTT_SYST_ENABLE) ?
			LOG_OUTPUT_FLAG_FORMAT_SYST : 0;
	log_output_msg_process(&log_output, msg, flags);
	log_msg_put(msg);
}

void log_backend_mqtt_set_ctx(struct mqtt_client *const client){
	log_output_ctx_set(&log_output, client);
}

void log_backend_mqtt_connected(void)
{
	connected = true;
}

static void sync_string(const struct log_backend *const backend,
		     struct log_msg_ids src_level, u32_t timestamp,
		     const char *fmt, va_list ap)
{
	u32_t flags = IS_ENABLED(CONFIG_LOG_BACKEND_MQTT_SYST_ENABLE) ?
			LOG_OUTPUT_FLAG_FORMAT_SYST : 0;
	u32_t key;
	if (!connected) {
		return;
	}

	key = irq_lock();
	log_output_string(&log_output, src_level, timestamp,fmt, ap, flags);
	irq_unlock(key);
}

static void panic(struct log_backend const *const backend)
{
	panic_mode = true;
}

static void init_mqtt(void)
{
	log_backend_deactivate(log_backend_mqtt_get());
}

const struct log_backend_api log_backend_mqtt_api = {
	.panic = panic,
	.init = init_mqtt,
	.put = IS_ENABLED(CONFIG_LOG_IMMEDIATE) ? NULL : send_output,
	.put_sync_string = IS_ENABLED(CONFIG_LOG_IMMEDIATE) ? sync_string : NULL,
	.put_sync_hexdump = NULL
};

/* Note that the backend can be activated only after we have the MQTT client
 * running so we must not start it immediately.
 */
LOG_BACKEND_DEFINE(log_backend_mqtt, log_backend_mqtt_api, true);

const struct log_backend *log_backend_mqtt_get(void)
{
	return &log_backend_mqtt;
}
