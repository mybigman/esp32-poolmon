/*
 * MIT License
 *
 * Copyright (c) 2018 David Antliff
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

#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

#include "constants.h"
#include "resources.h"
#include "utils.h"
#include "system_monitor.h"
#include "publish.h"

#define TAG "system_monitor"

#define CHECK_PERIOD (60 * 1000) // milliseconds

typedef struct
{
    const datastore_t * datastore;
    const publish_context_t * publish_context;
} task_inputs_t;

static TaskHandle_t _task_handle = NULL;

static void system_task(void * pvParameter)
{
    ESP_LOGD(TAG, "%s", __FUNCTION__);
    assert(pvParameter);
    ESP_LOGI(TAG, "Core ID %d", xPortGetCoreID());

    task_inputs_t * task_inputs = (task_inputs_t *)pvParameter;
    const datastore_t * datastore = task_inputs->datastore;

    TickType_t last_wake_time = xTaskGetTickCount();

    while (1)
    {
        last_wake_time = xTaskGetTickCount();

        uint32_t ram_free = esp_get_free_heap_size();  // byte-addressable heap memory
        uint32_t iram_free = heap_caps_get_free_size(MALLOC_CAP_32BIT);  // IRAM 32-bit aligned heap
        uint32_t uptime = microseconds_since_boot() / 1000000;

        ESP_LOGI(TAG, "RAM free: %u bytes", ram_free);
        ESP_LOGI(TAG, "RAM 32bit aligned free: %u bytes", iram_free);
        ESP_LOGI(TAG, "Uptime: %u seconds", uptime);

        bool mqtt_connected = false;
        datastore_get_bool(datastore, RESOURCE_ID_SYSTEM_TIME_SET, 0, &mqtt_connected);
        if (mqtt_connected)
        {
            datastore_set_uint32(datastore, RESOURCE_ID_SYSTEM_RAM_FREE, 0, ram_free);
            datastore_set_uint32(datastore, RESOURCE_ID_SYSTEM_IRAM_FREE, 0, iram_free);
            datastore_set_uint32(datastore, RESOURCE_ID_SYSTEM_UPTIME, 0, uptime);
        }

        vTaskDelayUntil(&last_wake_time, CHECK_PERIOD / portTICK_PERIOD_MS);
    }

    free(task_inputs);
    _task_handle = NULL;
    vTaskDelete(NULL);
}

void system_monitor_init(UBaseType_t priority, const datastore_t * datastore, publish_context_t * publish_context)
{
    task_inputs_t * task_inputs = malloc(sizeof(*task_inputs));
    if (task_inputs)
    {
        memset(task_inputs, 0, sizeof(*task_inputs));
        task_inputs->datastore = datastore;
        task_inputs->publish_context = publish_context;
        xTaskCreate(&system_task, "system_task", 4096, task_inputs, priority, &_task_handle);
    }
}

void system_monitor_delete(void)
{
    if (_task_handle)
        vTaskDelete(_task_handle);
}
