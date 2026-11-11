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

import os
import can
import cantools
import isotp
import queue
import time
import logging
from enum import IntEnum
from ctypes import *
from typing import Optional, List
from .canlin_listener import CanLinListener
from ..base_class.base_types import CanMsg, LinMsg, LinNodeModeEnum
from ..base_class.value_converter import ValueConverter

class BusType(IntEnum):
    CanBus = 0
    LinBus = 1


class SocketCanLinBase:
    def __init__(self, channel_mapping:dict, log_listener:CanLinListener=None, bustype=BusType.CanBus):
        self.handle = {}
        self.tx_msg = {}
        self.notifier = {}
        self.user_cbk = None
        self.periodic_task = {}
        self.database = {}
        self.rx_queue = {}
        self.chn_mapping = channel_mapping
        self.log_listener = log_listener
        self.log_enabled = {}
        self.lin_cfg = {}
        self.bus_type = bustype

    def get_channel(self, channel_idx):
        if channel_idx < len(self.chn_mapping):
            return self.chn_mapping[channel_idx]
        return None

    def _msg_preprocess(self, channel, msg:CanMsg):
        need_user_process = True
        id = msg.arbitration_id
        if (id in self.tx_msg[channel].keys()):
            need_user_process = False
            msg.is_rx = False
        else:
            msg.is_rx = True
        return need_user_process

    def _lin_msg_preprocess(self, channel, msg:LinMsg):
        lin_chn_cfg = self.lin_cfg[channel]
        send_async = lin_chn_cfg['send_async']
        id = msg.arbitration_id
        dlc = msg.dlc
        if lin_chn_cfg['mode'] == LinNodeModeEnum.Slave:
            if dlc == 0:
                if isinstance(send_async, LinMsg) and id == send_async.arbitration_id:
                    self.send(send_async)
                elif lin_chn_cfg['sch_start'] and id in lin_chn_cfg['sch'].keys() and lin_chn_cfg['sch'][id]['frame'].is_rx is False:
                    self.send(lin_chn_cfg['sch'][id]['frame'])
                return True
        if id in self.tx_msg[channel].keys() and msg.equals(self.tx_msg[channel][id], timestamp_delta=None, check_direction=False):
            msg.is_rx = False
            del self.tx_msg[channel][id]
            return False
        msg.is_rx = True
        return True

    def _rx_callback(self, channel, msg:CanMsg):
        if self.bus_type == BusType.CanBus:
            need_user_process = self._msg_preprocess(channel, msg)
        else:
            need_user_process = self._lin_msg_preprocess(channel, msg)
        if self.log_enabled[channel]:
            self.log_listener.on_message_received(msg)
        if not need_user_process:
            return
        msg.channel = self.chn_mapping.index(msg.channel)
        if not self.rx_queue[channel].full():
            self.rx_queue[channel].put(msg)
        if self.user_cbk:
            self.user_cbk(msg)
        # print(f"{channel}: {msg}")

    def connect_channel(self, channel_idx, interface='socketcan', fd=True, filter=None, queue_size=10000, log_enabled=True):
        channel = self.get_channel(channel_idx)
        self.log_enabled[channel] = bool(log_enabled and self.log_listener)
        self.tx_msg[channel] = {}
        if self.handle.get(channel):
            self.handle[channel].shutdown()
        self.handle[channel] = can.interface.Bus(channel=channel, interface=interface, receive_own_messages=True, fd=fd)
        self.rx_queue[channel] = queue.Queue(queue_size)
        self.notifier[channel] = can.Notifier(self.handle[channel], [lambda msg, ch=channel: self._rx_callback(ch, msg)])
        if self.bus_type == BusType.LinBus:
            self.lin_cfg[channel] = {'sch':{}, 'mode': LinNodeModeEnum.Slave, 'send_async':None, 'sch_start':False}
        logging.info(f"Creat {channel}")

    def set_user_callback(self, user_callback):
        if user_callback:
            self.user_cbk = user_callback

    def cancel_user_callback(self):
        self.user_cbk = None

    def msg(self, channel_idx, msg_id, data, dlc=None, is_fd=False, is_brs=False, is_rx=False, is_extended_id=False):
        channel = self.get_channel(channel_idx)
        return CanMsg(channel=channel, arbitration_id=msg_id, data=data, dlc=dlc, is_fd=is_fd, bitrate_switch=is_brs, is_rx=is_rx, is_extended_id=is_extended_id)

    def msg_by_signals(self, channel_idx, msgid_or_msgname, signal_values={}, is_phy_val=True):
        msg = self._find_msg_from_dbc(channel_idx=channel_idx, msgid_or_msgname=msgid_or_msgname)
        if msg is None:
            return None
        all_signal_values = dict.fromkeys(msg.signal_tree, 0)
        for key, val in signal_values.items():
            all_signal_values.update({key: val})
            if not is_phy_val:
                sig_info = msg._signal_dict.get(key, None)
                all_signal_values.update({key: ValueConverter.raw_to_phy(sig_info, val)})
        data = msg.encode(all_signal_values)
        return self.msg(channel_idx=channel_idx, msg_id=msg.frame_id, data=data,dlc=msg.length, is_fd=msg.is_fd, is_brs=msg.is_fd, is_extended_id=msg.is_extended_frame)

    def send(self, msg:CanMsg, channel_idx:int=None):
        channel = msg.channel if channel_idx is None else self.get_channel(channel_idx)
        self.tx_msg[channel].update({msg.arbitration_id: msg})
        self.handle[channel].send(msg)

    def send_by_hex_data(self, channel_idx, arbitration_id, data, is_fd):
        channel = self.get_channel(channel_idx)
        msg = CanMsg(arbitration_id=arbitration_id, data=data,  is_fd=is_fd, is_extended_id=False, channel=channel)
        self.send(msg)

    def send_periodic(self, msg:CanMsg, period:float=0.1):
        channel = msg.channel
        task = self.handle[channel].send_periodic(msg, period)
        if channel not in self.periodic_task.keys():
            self.periodic_task[channel] = {}
        if msg.arbitration_id in self.periodic_task[channel].keys():
            self.periodic_task[channel][msg.arbitration_id].stop()
        self.periodic_task[channel][msg.arbitration_id] = task
        print(f"Periodic TX: {msg.arbitration_id} on {channel} with period {period}")

    def cancel_send_periodic(self, msg:CanMsg):
        channel = msg.channel
        channel_tasks = self.periodic_task.get(channel, {})
        task = channel_tasks.pop(msg.arbitration_id, None)
        if task:
            task.stop()
            print(f"Stopped periodic TX: {msg.arbitration_id} on {channel}")

    def recv(self, channel_idx, msgid_or_msgname=None, timeout:float=0.1) -> Optional[CanMsg]:
        channel = self.get_channel(channel_idx)
        msg_info = self._find_msg_from_dbc(channel_idx=channel_idx, msgid_or_msgname=msgid_or_msgname) if isinstance(msgid_or_msgname, str) else True
        if msg_info is None:
            print(f'SocketCanLinBase Error: message name error {msgid_or_msgname}')
            return None
        start_time = time.perf_counter()
        while time.perf_counter() - start_time <= timeout:
            try:
                msg = self.rx_queue[channel].get(timeout=timeout)
                if msgid_or_msgname is None:
                    return msg
                elif isinstance(msgid_or_msgname, int):
                    if msg.arbitration_id == msgid_or_msgname:
                        return msg
                else:
                    if msg.arbitration_id == msg_info.frame_id:
                        return msg
            except queue.Empty:
                pass
        return None

    def recv_signals(self, channel_idx, msgid_or_msgname, signals:list, timeout:float=0.1, is_phy_val=True)->Optional[dict]:
        dbc_msg = self._find_msg_from_dbc(channel_idx=channel_idx, msgid_or_msgname=msgid_or_msgname, signals=signals)
        if dbc_msg is None:
            return None
        received_msg = self.recv(channel_idx=channel_idx, msgid_or_msgname=msgid_or_msgname, timeout=timeout)
        all_signals = dbc_msg.decode(received_msg.data)
        if is_phy_val:
            signals_vals = {name: all_signals[name] for name in signals}
        else:
            signals_vals = {name: ValueConverter.raw_to_phy(dbc_msg._signal_dict.get(name, None), all_signals[name]) for name in signals}
        return signals_vals

    def clear_msg(self, channel_idx=None):
        if channel_idx is None:
            for channel in self.rx_queue.keys():
                self.rx_queue[channel] = queue.Queue()
        else:
            channel = self.get_channel(channel_idx)
            self.rx_queue[channel] = queue.Queue()

    def load_dbc_ldf(self, file:str, channel_idx=None)->bool:
        try:
            channel = None if channel_idx is None else self.get_channel(channel_idx)
            if file.endswith('.dbc'):
                db = cantools.database.load_file(file)
                self.database.setdefault(channel, []).append(db)
        except:
            return False
        return True

    def _find_msg_from_dbc(self, channel_idx, msgid_or_msgname, signals=[]):
        channel = self.get_channel(channel_idx)
        find_name = isinstance(msgid_or_msgname, str)
        dbc_list = self.database.get(channel, []) + self.database.get(None, [])
        messages = []
        for dbc_db in dbc_list:
            msg = dbc_db._name_to_message.get(msgid_or_msgname) if find_name else dbc_db._frame_id_to_message.get(msgid_or_msgname)
            if msg is not None:
                messages.append(msg)
        messages = [msg for msg in messages if len(set(msg.signal_tree)) == len(set(msg.signal_tree+signals))]
        if len(messages) == 0:
            print(f'SocketCanLinBase ERROR: Not found message:({msgid_or_msgname}), channle: ({channel_idx}:{channel})')
            return None
        if len(messages) > 1:
            print(f'SocketCanLinBase ERROR: Found multiple messages:({msgid_or_msgname}))')
        return messages[0]

    def lin_send_sync(self, msg:LinMsg, timeout:float=0.01):
        rxmsg = self.recv(msg.channel, msgid_or_msgname=msg.arbitration_id, timeout=timeout)
        if rxmsg is None:
            self.send(msg)

    def lin_clear_schedule_tables(self, chn_idx):
        channel = self.get_channel(chn_idx)
        self.lin_cfg[channel]['sch_start'] = False
        self.lin_cfg[channel]['sch'] = {}

    def lin_send_async(self, msg:LinMsg):
        self.lin_cfg[msg.channel]['send_async'] = msg

    def lin_add_schedule_frames(self, frames:List[LinMsg], slottime_ms, start_schedule):
        channel = frames[0].channel
        for frame in frames:
            if channel != frame.channel:
                raise Exception('All channels must be the same')
            self.lin_cfg[channel]['sch'].update({frame.arbitration_id: {'frame': frame, 'slottime_ms': slottime_ms}})
        if start_schedule:
            self.lin_cfg[channel]['sch_start'] = True

    def lin_start_schedule_tables(self, chn_idx):
        channel = self.get_channel(chn_idx)
        self.lin_cfg[channel]['sch_start'] = True

    def close(self):
        for _, notifier in self.notifier.items():
            notifier.stop()
        for _, tasks in self.periodic_task.items():
            for task in tasks.values():
                task.stop()
        for bus_obj in self.handle.values():
            bus_obj.shutdown()
        self.notifier = {}
        self.periodic_task = {}
        self.handle = {}
