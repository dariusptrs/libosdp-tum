#
#  Copyright (c) 2020 Siddharth Chandrasekaran <siddharth@embedjournal.com>
#
#  SPDX-License-Identifier: Apache-2.0
#

import time
import random
import osdp

output_cmd = {
    "command": osdp.CMD_OUTPUT,
    "output_no": 0,
    "control_code": 1,
    "timer_count": 10
}

buzzer_cmd = {
    "command": osdp.CMD_BUZZER,
    "reader": 0,
    "control_code": 1,
    "on_count": 10,
    "off_count": 10,
    "rep_count": 10
}

text_cmd = {
    "command": osdp.CMD_TEXT,
    "reader": 0,
    "control_code": 1,
    "temp_time": 20,
    "offset_row": 1,
    "offset_col": 1,
    "data": "PYOSDP"
}

led_cmd = {
    "command": osdp.CMD_LED,
    "reader": 1,
    "led_number": 1,
    "control_code": 1,
    "on_count": 10,
    "off_count": 10,
    "on_color": osdp.LED_COLOR_RED,
    "off_color": osdp.LED_COLOR_NONE,
    "timer_count": 10,
    "temporary": True
}

comset_cmd = {
    "command": osdp.CMD_COMSET,
    "address": 101,
    "baud_rate": 9600
}

keyset_cmd = {
    "command": osdp.CMD_KEYSET,
    "type": 1,
    "data": "01020304050607080910111213141517"
}

mfg_cmd = {
    "command": osdp.CMD_MFG,
    "vendor_code": 0x00030201,
    "mfg_command": 13,
    "data": bytes([9,1,9,2,6,3,1,7,7,0])
}

pd_info = [
    {
        "address": 101,
        "channel_type": "message_queue",
        "channel_speed": 115200,
        "channel_device": '/tmp/osdp_mq',
    }
]

key = '01020304050607080910111213141516'

commands = [ output_cmd, buzzer_cmd, text_cmd, led_cmd, comset_cmd, mfg_cmd ]

def event_handler(address, event):
    print("Address: ", address, " Event: ", event)

cp = osdp.ControlPanel(pd_info, master_key=key)
cp.set_event_callback(event_handler)
cp.set_loglevel(6)

count = 0
while True:
    cp.refresh()

    if (count % 100 == 99):
        # send a random command to the PD_0
        r = random.randint(0, len(commands)-1)
        cp.send_command(0, commands[r])

    time.sleep(0.05)
    count += 1
