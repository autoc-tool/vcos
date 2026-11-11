# This Python file uses the following encoding: utf-8
# ##############################################################################
# Copyright (c) 2025 Li Auto Inc. and its affiliates
# Licensed under the Apache License, Version 2.0(the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ##############################################################################

import can
import os
import threading
import time
import logging
from datetime import datetime
from typing import Optional, List
from .env_cfg import CHANNEL_INDEX, CAN_CHANNEL_MAPPING, LIN_CHANNEL_MAPPING
from .socketcanlin_base import SocketCanLinBase, BusType
from .canlin_listener import CanLinListener
from ..base_class.base_types import CanMsg, LinMsg
from ..base_class.canlin_base import CanLinDriverBase


CUR_DIR = os.path.dirname(__file__)

class CanLinDriver(CanLinDriverBase):
    def __init__(self, log_enabled=False, log_file=None, queue_max=10000):
        self.listener = None
        self.log_enabled = log_enabled
        if self.log_enabled:
            current_time = datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
            logging_file = os.path.join(CUR_DIR, 'log', f"canlin_log_{current_time}.asc") if log_file is None else log_file
            if not os.path.exists(os.path.dirname(logging_file)):
                os.makedirs(os.path.dirname(logging_file))
            self.listener = CanLinListener(can.ASCWriter(logging_file))
        self.canbus = SocketCanLinBase(CAN_CHANNEL_MAPPING, bustype=BusType.CanBus, log_listener=self.listener)
        self.linbus = SocketCanLinBase(LIN_CHANNEL_MAPPING, bustype=BusType.LinBus, log_listener=self.listener)
        self.queue_max = queue_max

    def deinit(self):
        if self.listener:
            self.listener.close()
        self.canbus.close()
        self.linbus.close()

    def device_connect(self, use_default_cfg=False, **kwargs):
        if use_default_cfg:
            for chn_idx in range(8):
                self.can_connect_channel(chn_idx=chn_idx)
        error_code = 0
        dev_idx = 0
        return error_code, dev_idx

    def can_connect_channel(self, chn_idx, fd=True, dbc:str=None, queue_size:int=None, log_enabled=True, **kwargs):
        if queue_size is None:
            queue_size = self.queue_max
        self.canbus.connect_channel(channel_idx=chn_idx, interface='socketcan', fd=fd, queue_size=queue_size, log_enabled=log_enabled)
        if dbc:
            self.load_dbc(dbc, chn_idx=chn_idx)

    def can_msg(self, chn_idx, msg_id, data, dlc=None, is_fd=False, is_brs=False, is_rx=None, is_extended_id=False, **kwargs)->Optional[CanMsg]:
        dlc = can_dlc_to_length(dlc)
        return self.canbus.msg(channel_idx=chn_idx, msg_id=msg_id, data=data, dlc=dlc, is_fd=is_fd, is_brs=is_brs, is_rx=is_rx, is_extended_id=is_extended_id)

    def can_msg_by_dbc(self, chn_idx, msgid_or_msgname, signal_values={}, is_phy_val=True, **kwargs)->Optional[CanMsg]:
        return self.canbus.msg_by_signals(channel_idx=chn_idx, msgid_or_msgname=msgid_or_msgname, signal_values=signal_values, is_phy_val=is_phy_val)

    def can_send(self, msg:can.Message, chn_idx:int=None, **kwargs):
        self.canbus.send(msg, chn_idx)

    def can_set_user_callback(self, callback, **kwargs):
        self.canbus.set_user_callback(callback)

    def can_cancel_user_callback(self, **kwargs):
        self.canbus.cancel_user_callback()

    def can_recv(self, chn_idx, msgid_or_msgname=None, timeout:float=0.1, **kwargs) -> Optional[can.Message]:
        return self.canbus.recv(channel_idx=chn_idx, msgid_or_msgname=msgid_or_msgname, timeout=timeout)

    def can_recv_spec_num_from_fifo(self, chn_idx, timeout, num, **kwargs):
        msg_list = []
        start_time = time.perf_counter()
        while time.perf_counter() - start_time <= timeout and len(msg_list) < num:
            msg = self.can_recv(chn_idx=chn_idx, timeout=0.01)
            if msg is None:
                return msg_list
            msg_list.append(msg)
        return msg_list

    def can_recv_signals(self, chn_idx, msgid_or_msgname, signalnames, timeout:float=0.1, **kwargs) -> Optional[dict]:
        return self.canbus.recv_signals(channel_idx=chn_idx, msgid_or_msgname=msgid_or_msgname, signals=signalnames, timeout=timeout, is_phy_val=True)

    def can_sendmessage_by_data(self, chn_idx, arbitration_id, data, is_fd, **kwargs):
        msg = can.Message(arbitration_id=arbitration_id, data=data, is_fd=is_fd, is_extended_id=False,channel=self.canbus.get_channel(chn_idx))
        self.can_send(msg)

    def can_send_periodic(self, msg:can.Message, period:float=0.1, **kwargs):
        self.canbus.send_periodic(msg, period)

    def can_cancel_send_periodic(self, msg:can.Message, **kwargs):
        self.canbus.cancel_send_periodic(msg)

    def load_dbc(self, dbc_file: str, chn_idx=None, **kwargs) -> bool:
        return self.canbus.load_dbc_ldf(dbc_file, channel_idx=chn_idx)

    def can_clear_rx_buf(self, channel_idx=None, **kwargs):
        self.canbus.clear_msg(channel_idx)

    def lin_connect_channel(self, chn_idx, lin_node_type, queue_size=10000, log_enabled=False, **kwargs):
        self.linbus.connect_channel(channel_idx=chn_idx, interface='socketcan', fd=False, queue_size=queue_size, log_enabled=log_enabled)
        return 0

    def lin_set_user_callback(self, callback, **kwargs):
        self.linbus.set_user_callback(callback)
        return 0

    def lin_cancel_user_callback(self, **kwargs):
        self.linbus.cancel_user_callback()

    def lin_reset_schedule_tables(self, chn_idx, **kwargs):
        return self.linbus.lin_clear_schedule_tables(chn_idx)

    def lin_add_schedule_frames(self, frames:List[LinMsg], slottime_ms, start_schedule, **kwargs):
        return self.linbus.lin_add_schedule_frames(frames=frames, slottime_ms=slottime_ms, start_schedule=start_schedule)

    def lin_start_schedule_tables(self, chn_idx, **kwargs):
        return self.linbus.lin_start_schedule_tables(chn_idx)

    def lin_clear_schedule_tables(self, chn_idx, **kwargs):
        return self.linbus.lin_clear_schedule_tables(chn_idx)

    def lin_msg(self, chn_idx, msg_id, data, dlc, is_rx, **kwargs):
        return self.linbus.msg(channel_idx=chn_idx, msg_id=msg_id, data=data, dlc=dlc, is_rx=is_rx)

    def lin_send(self, msg: LinMsg, timeout=0.01, is_sync:bool=False, **kwargs):
        if is_sync:
            self.linbus.lin_send_sync(msg, timeout)
        else:
            self.linbus.lin_send_async(msg)


def can_dlc_to_length(dlc):
    dlc_to_length_map = {
        0: 0,
        1: 1,
        2: 2,
        3: 3,
        4: 4,
        5: 5,
        6: 6,
        7: 7,
        8: 8,
        9: 12,
        10: 16,
        11: 20,
        12: 24,
        13: 32,
        14: 48,
        15: 64
    }
    return dlc_to_length_map.get(dlc, dlc)