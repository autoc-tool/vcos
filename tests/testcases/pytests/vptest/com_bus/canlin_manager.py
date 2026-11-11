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

import time
from typing import Optional, List
from .base_class.base_types import CanMsg, LinMsg, LinNodeModeEnum
from . import CanLinDriver


class CanLinManager(CanLinDriver):

    def __init__(self, log_enabled=False, log_file=None, queue_max=10000):
        self._init = True
        self._log_enabled = log_enabled
        self._log_file = log_file
        self._queue_max = queue_max
        super().__init__(log_enabled=log_enabled, log_file=log_file, queue_max=queue_max)

    def init(self):
        self.deinit()
        self.__init__(log_enabled=self._log_enabled, log_file=self._log_file, queue_max=self._queue_max)

    def deinit(self):
        super().deinit()
        self._init = False

    def device_connect(self, device_serial=None, use_default_cfg=False):
        error_code, dev_idx = super().device_connect(device_serial=device_serial, use_default_cfg=use_default_cfg)
        return error_code, dev_idx

    def can_connect_channel(self, chn_idx, dev_idx=0, fd=True, arb_rate=500, data_rate=2000, queue_size=None, log_enabled=True):
        kwargs = locals().copy()
        kwargs.pop('self')
        return super().can_connect_channel(**kwargs)

    def load_dbc(self, dbc_file: str, dev_idx=0, chn_idx=None):
        return super().load_dbc(dbc_file=dbc_file, dev_idx=dev_idx, chn_idx=chn_idx)

    def can_msg(self, chn_idx, msg_id, data, dlc=None, is_fd=False, is_brs=False, is_rx=False, is_extended_id=False) -> CanMsg:
        kwargs = locals().copy()
        kwargs.pop('self')
        return super().can_msg(**kwargs)

    def can_msg_by_dbc(self, chn_idx, msgid_or_msgname, signal_values={}, is_phy_val=True, dev_idx=0) -> CanMsg:
        kwargs = locals().copy()
        kwargs.pop('self')
        return super().can_msg_by_dbc(**kwargs)

    def can_send(self, msg: CanMsg, dev_idx=0, timeout=0.2):
        return super().can_send(msg=msg, dev_idx=dev_idx, timeout=timeout)

    def can_recv(self, chn_idx, dev_idx=0, msgid_or_msgname=None, timeout=0.5, wait_timeout=False) -> Optional[CanMsg]:
        start_time = time.perf_counter()
        msg_last = None
        while (time.perf_counter() - start_time) <= timeout:
            msg = super().can_recv(dev_idx=dev_idx, chn_idx=chn_idx, msgid_or_msgname=msgid_or_msgname, timeout=timeout)
            if wait_timeout:
                msg_last = msg if msg is not None else msg_last
            else:
                return msg
        return msg_last

    def can_recv_spec_num_from_fifo(self, chn_idx, dev_idx=0, is_fd=True, timeout=0.5, num=1) -> Optional[list]:
        msg_list = super().can_recv_spec_num_from_fifo(chn_idx=chn_idx, dev_idx=dev_idx, is_fd=is_fd, timeout=timeout, num=num)
        return msg_list

    def can_recv_signals(self, chn_idx, msgid_or_msgname, signalnames, is_phy_val=True, dev_idx=0, timeout=0.5, wait_timeout=False) -> Optional[dict]:
        start_time = time.perf_counter()
        signal_val_last = None
        while (time.perf_counter() - start_time) <= timeout:
            signal_val = super().can_recv_signals(
                chn_idx=chn_idx, msgid_or_msgname=msgid_or_msgname,
                signalnames=signalnames, is_phy_val=is_phy_val,
                dev_idx=dev_idx, timeout=timeout
            )
            signal_val.update({key: val.value for key, val in signal_val.items() if hasattr(val, "value")})
            if wait_timeout:
                signal_val_last = signal_val if set(signal_val.values()) != {None} else signal_val_last
            else:
                return signal_val
        return signal_val_last

    def can_send_periodic(self, msg: CanMsg, period: float = 0.01, dev_idx=0):
        return super().can_send_periodic(msg=msg, period=period, dev_idx=dev_idx)

    def can_cancel_send_periodic(self, msg: CanMsg, dev_idx=0):
        return super().can_cancel_send_periodic(msg=msg, dev_idx=dev_idx)

    def can_set_user_callback(self, callback, dev_idx=0):
        return super().can_set_user_callback(dev_idx=dev_idx, callback=callback)

    def can_cancel_user_callback(self, dev_idx=0):
        return super().can_cancel_user_callback(dev_idx=dev_idx)

    def can_clear_rx_buf(self, chn_idx, dev_idx=0):
        return super().can_clear_rx_buf(chn_idx=chn_idx, dev_idx=dev_idx)

    def lin_connect_channel(self, chn_idx, dev_idx=0, baudrate_kbps=19.2, lin_node_type=LinNodeModeEnum.Slave, log_enabled=False):
        kwargs = locals().copy()
        kwargs.pop('self')
        return super().lin_connect_channel(**kwargs)

    def lin_set_user_callback(self, callback, dev_idx=0):
        return super().lin_set_user_callback(dev_idx=dev_idx, callback=callback)

    def lin_cancel_user_callback(self, dev_idx=0):
        return super().lin_cancel_user_callback(dev_idx=dev_idx)

    def lin_reset_schedule_tables(self, chn_idx, dev_idx=0):
        return super().lin_reset_schedule_tables(chn_idx=chn_idx, dev_idx=dev_idx)

    def lin_add_schedule_frames(self, frames:List[LinMsg], slottime_ms=10, start_schedule=True, dev_idx=0):
        return super().lin_add_schedule_frames(frames=frames, slottime_ms=slottime_ms, start_schedule=start_schedule, dev_idx=dev_idx)

    def lin_start_schedule_tables(self, chn_idx, dev_idx=0):
        return super().lin_start_schedule_tables(chn_idx=chn_idx, dev_idx=dev_idx)

    def lin_clear_schedule_tables(self, chn_idx, dev_idx=0):
        return super().lin_clear_schedule_tables(chn_idx=chn_idx, dev_idx=dev_idx)

    def lin_msg(self, chn_idx, msg_id, data, dlc=8, is_rx=False) -> LinMsg:
        return super().lin_msg(chn_idx=chn_idx, msg_id=msg_id, data=data, dlc=dlc, is_rx=is_rx)

    def lin_send(self, msg: LinMsg, dev_idx=0, is_sync:bool=False, timeout=0.01):
        return super().lin_send(msg=msg, dev_idx=dev_idx, is_sync=is_sync, timeout=timeout)
