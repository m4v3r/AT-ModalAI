/****************************************************************************
 *
 *   Copyright (c) 2019-2021 PX4 Development Team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/**
 * @author CUAVcaijie <caijie@cuav.net>
 */

#include "safetybutton.hpp"
#include <cstdint>
#include <drivers/drv_hrt.h>
#include <systemlib/err.h>
#include <mathlib/mathlib.h>

using namespace time_literals;
const char *const UavcanSafetyBridge::NAME = "safety";

UavcanSafetyBridge::UavcanSafetyBridge(uavcan::INode &node) :
    UavcanSensorBridgeBase("uavcan_safety", ORB_ID(safety)),
    _sub_safety(node)
{
}

int UavcanSafetyBridge::init()
{
    int res = _sub_safety.start(SafetyCbBinder(this, &UavcanSafetyBridge::safety_sub_cb));

    if (res < 0) {
        DEVICE_LOG("failed to start uavcan sub: %d", res);
        return res;
    }

    return 0;
}

void UavcanSafetyBridge::safety_sub_cb(const uavcan::ReceivedDataStructure<ardupilot::indication::Button> &msg)
{
    // TODO: Right now this only handle the safety button but theoretically could handle any button type
    bool is_safety_button = msg.button == 1;
    bool safety_off = false;

    // If it's the safety button and it's pressed for more than a second
    if (is_safety_button && msg.press_time > 10) {
        safety_off = true;
    } else {
        return;
    }

    safety_s report{};
    report.timestamp                = hrt_absolute_time();
    report.safety_switch_available  = true;
    report.safety_off               = safety_off;

    publish(msg.getSrcNodeID().get(), &report);
}
