/*
 * EMS-ESP - https://github.com/emsesp/EMS-ESP
 * Copyright 2020  Paul Derbyshire
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
 *
 * CAN bus code
 * Copyright 2022  Pieter Palmers
 */

#include "can.h"
#include "emsesp.h"

#include "driver/gpio.h"
#include "driver/can.h"

#define CAN_TX_GPIO_NUM                     GPIO_NUM_5
#define CAN_RX_GPIO_NUM                     GPIO_NUM_4

#define NO_OF_ITERS                     3
#define RX_TASK_PRIO                    9

#define ID_MASTER_STOP_CMD              0x0A0
#define ID_MASTER_START_CMD             0x0A1
#define ID_MASTER_PING                  0x0A2
#define ID_SLAVE_STOP_RESP              0x0B0
#define ID_SLAVE_DATA                   0x0B1
#define ID_SLAVE_PING_RESP              0x0B2

static const can_filter_config_t f_config = CAN_FILTER_CONFIG_ACCEPT_ALL();
static const can_timing_config_t t_config = CAN_TIMING_CONFIG_125KBITS();

//Set TX queue length to 0 due to listen only mode
static const can_general_config_t g_config = {.mode = CAN_MODE_LISTEN_ONLY,
                                            .tx_io = CAN_TX_GPIO_NUM, .rx_io = CAN_RX_GPIO_NUM,
                                            .clkout_io = CAN_IO_UNUSED, .bus_off_io = CAN_IO_UNUSED,
                                            .tx_queue_len = 0, .rx_queue_len = 5,
                                            .alerts_enabled = CAN_ALERT_NONE,
                                            .clkout_divider = 0};


#ifdef ESP32
#define YIELD
#else
#define YIELD yield()
#endif

extern "C" {
    static SemaphoreHandle_t rx_sem;

    /* --------------------------- Tasks and Functions -------------------------- */

    static void can_receive_task(void *arg)
    {
        emsesp::CanBus *parent = static_cast<emsesp::CanBus *>(arg);
        uuid::log::Logger &logger_ = parent->logger_;

        xSemaphoreTake(rx_sem, portMAX_DELAY);
        LOG_INFO("Task started");
        char data_buf[64];

        while (true) {
            can_message_t rx_msg;
            can_receive(&rx_msg, portMAX_DELAY);
            memset(data_buf, 0, sizeof(data_buf));
            for(int i=0; i<rx_msg.data_length_code; i++) {
                snprintf(data_buf+(i*3), 64-(i*3), "%02X ", rx_msg.data[i]);
            }
            LOG_INFO("RCV %08X %08X [%d] %s", rx_msg.flags, rx_msg.identifier, rx_msg.data_length_code, data_buf);
        }

        xSemaphoreGive(rx_sem);
        vTaskDelete(NULL);
    }
};

namespace emsesp {

uuid::log::Logger CanBus::logger_{F_(canbus), uuid::log::Facility::DAEMON};

CanBus::CanBus() {

}


void CanBus::start() {
    rx_sem = xSemaphoreCreateBinary();
    xTaskCreatePinnedToCore(can_receive_task, "CAN_rx", 4096, this, RX_TASK_PRIO, NULL, tskNO_AFFINITY);

    //Install and start CAN driver
    ESP_ERROR_CHECK(can_driver_install(&g_config, &t_config, &f_config));
    LOG_INFO("CAN Driver installed");
    ESP_ERROR_CHECK(can_start());
    LOG_INFO("CAN Driver started");

    xSemaphoreGive(rx_sem);                     //Start RX task

}

void CanBus::loop() {

}

};
