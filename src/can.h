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

#ifndef EMSESP_CAN_H
#define EMSESP_CAN_H

#include "helpers.h"
#include "mqtt.h"
#include "console.h"

#include <uuid/log.h>

namespace emsesp {

class CanBus {
  public:

    CanBus();
    ~CanBus() = default;

    void start();
    void loop();
   
#ifdef EMSESP_DEBUG
    void test();
#endif

    static uuid::log::Logger logger_;

  private:
};

} // namespace emsesp

#endif