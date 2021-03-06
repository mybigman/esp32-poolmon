/*
 * MIT License
 *
 * Copyright (c) 2017 David Antliff
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef PUBLISH_H
#define PUBLISH_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "mqtt.h"

typedef struct
{
    QueueHandle_t queue;
} publish_context_t;

void publish_topics_init(const datastore_t * datastore, publish_context_t * publish_context);

publish_context_t * publish_init(mqtt_info_t * mqtt_info, unsigned int queue_depth, UBaseType_t priority, const char * root_topic);
void publish_delete(void);
void publish_free(publish_context_t ** publish_context);

void publish_resource(const publish_context_t * publish_context, const datastore_t * datastore, datastore_resource_id_t id, datastore_instance_id_t instance);
void publish_direct(const publish_context_t * publish_context, const char * topic, const uint8_t * data, size_t length);

// Called by datastore whenever a subscribed value is set
void publish_callback(const datastore_t * datastore, datastore_resource_id_t id, datastore_instance_id_t instance, void * context);

#endif // PUBLISH_H
