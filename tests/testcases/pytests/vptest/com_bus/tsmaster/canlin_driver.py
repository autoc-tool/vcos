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
import threading
import time
import logging
import libTSCANAPI as tsapi
from datetime import datetime
from ctypes import c_size_t, c_int32
from typing import Optional, List
from libTSCANAPI import *
from ..base_class.base_types import CanMsg, LinMsg
from ..base_class.canlin_base import CanLinDriverBase
from ..base_class.value_converter import ValueConverter



class CanLinDriver(CanLinDriverBase):
    def __init__(self, log_enabled=False, log_file=None, queue_max=10000, **kwargs):
        tsapi.initialize_lib_tscan(True, True, False)
        self._dev_serial_list = []
        self._dev_handle_list = []
        self._dev_channel_list = []
        self._dev_event_list = []
        self._lin_dev_channel_list = []
        self._lin_dev_event_list = []

    def deinit(self):
        for dev_idx in range(len(self._dev_handle_list)):
            handle = self._dev_handle_list[dev_idx]
            self.can_cancel_user_callback(dev_idx=dev_idx)
            self.lin_cancel_user_callback(dev_idx=dev_idx)
            for chn_idx in self._lin_dev_channel_list[dev_idx]:
                self.lin_clear_schedule_tables(dev_idx=dev_idx, chn_idx=chn_idx)
            handle.shut_down()
        tsapi.finalize_lib_tscan()
        self._dev_serial_list = []
        self._dev_handle_list = []
        self._dev_channel_list = []
        self._dev_event_list = []
        self._lin_dev_channel_list = []
        self._lin_dev_event_list = []

    def device_connect(self, device_serial=None, use_default_cfg=False, **kwargs):
        if device_serial in self._dev_serial_list:
            dev_idx = self._dev_serial_list.index(device_serial)
        else:
            self.ts_event = TsDevEvent()
            dev_obj = tsapi.TSMasterDevice(configs=[], hwserial=device_serial)
            self._dev_handle_list.append(dev_obj)
            self._dev_channel_list.append([])
            self._dev_serial_list.append(device_serial)
            self._dev_event_list.append(self.ts_event)
            self._lin_dev_channel_list.append([])
            self._lin_dev_event_list.append(self.ts_event)
            dev_idx = len(self._dev_serial_list) - 1
            if use_default_cfg:
                for chn_idx in range(8):
                    self.can_connect_channel(dev_idx=dev_idx, chn_idx=chn_idx)
        error_code = 0
        return error_code, dev_idx

    def can_connect_channel(self, dev_idx, chn_idx, fd=True, arb_rate=500, data_rate=2000, **kwargs):
        handle = self._dev_handle_list[dev_idx]
        if fd:
            ret = tsapi.tsapp_configure_baudrate_canfd(handle.HwHandle, chn_idx, arb_rate, data_rate, 1, 0, 1)
        else:
            ret = tsapi.tsapp_configure_baudrate_can(handle.HwHandle, chn_idx, arb_rate, 1)
        if ret == 0:
            self._dev_channel_list[dev_idx].append(chn_idx)
        return ret

    def load_dbc(self, dbc_file: str, dev_idx=0, **kwargs) -> None:
        return self._dev_handle_list[dev_idx].load_dbc(dbc_file)

    def can_msg(self, chn_idx, msg_id, data, dlc=None, is_fd=False, is_brs=False, is_rx=None, is_extended_id=False, **kwargs) -> Optional[CanMsg]:
        return CanMsg(channel=chn_idx, arbitration_id=msg_id, data=data, dlc=dlc, is_fd=is_fd, bitrate_switch=is_brs, is_rx=is_rx, is_extended_id=is_extended_id)

    def can_msg_by_dbc(self, dev_idx, chn_idx, msgid_or_msgname, signal_values={}, is_phy_val=True, **kwargs)->Optional[CanMsg]:
        db_msg = self.__find_msg_from_dbc(dev_idx, msgid_or_msgname)
        if db_msg is None:
            raise ValueError(f"msgid_or_msgname:{msgid_or_msgname} not found")
        all_signal_values = dict.fromkeys(db_msg.signal_tree, 0)
        if is_phy_val:
            all_signal_values.update(signal_values)
        else:
            for name, val in signal_values.items():
                val_new = ValueConverter.raw_to_phy(db_msg._signal_dict.get(name, None), val)
                all_signal_values.update({name: val_new})
        data = db_msg.encode(all_signal_values)
        msg = self.can_msg(chn_idx=chn_idx, msg_id=db_msg.frame_id, data=data, dlc=db_msg.length, is_fd=db_msg.is_fd, is_brs=db_msg.is_fd, is_extended_id=db_msg.is_extended_frame)
        return msg

    def can_send(self, msg: CanMsg, dev_idx, timeout=0.1, **kwargs):
        self._dev_handle_list[dev_idx].send_msg(msg=msg, timeout=timeout, sync=False, is_cyclic=False)

    def can_set_user_callback(self, dev_idx, callback, **kwargs):
        self._dev_event_list[dev_idx].CAN_USER_CALLBACK = callback
        return tsapi.tsapp_register_event_canfd_whandle(self._dev_handle_list[dev_idx].HwHandle, self._dev_event_list[dev_idx].ONCANFDEvent)

    def can_cancel_user_callback(self, dev_idx=0, **kwargs):
        if self._dev_event_list[dev_idx].CAN_USER_CALLBACK is None:
            return 0
        self._dev_event_list[dev_idx].CAN_USER_CALLBACK = None
        return tsapi.tsapp_unregister_event_canfd_whandle(self._dev_handle_list[dev_idx].HwHandle, self._dev_event_list[dev_idx].ONCANFDEvent)

    def can_recv(self, dev_idx, chn_idx, msgid_or_msgname=None, timeout: float = 0.01, **kwargs) -> Optional[CanMsg]:
        db_msg = None if msgid_or_msgname is None else self.__find_msg_from_dbc(dev_idx, msgid_or_msgname)
        expect_id = msgid_or_msgname if msgid_or_msgname is None or isinstance(msgid_or_msgname, int) else db_msg.frame_id
        start_time = time.perf_counter()
        while time.perf_counter() - start_time <= timeout:
            msg = self._dev_handle_list[dev_idx].recv(chn_idx, timeout)
            if msg is None:
                return None
            if expect_id is None:
                return msg
            else:
                if msg.arbitration_id == expect_id:
                    return msg
        return msg

    def can_recv_spec_num_from_fifo(self, chn_idx, dev_idx, is_fd, timeout, num):
        size = c_int32(num)
        if is_fd is False:
            canbuffer = (tsapi.TLIBCAN * 100)()
            r = tsapi.tsfifo_receive_can_msgs(self._dev_handle_list[dev_idx].HwHandle, canbuffer, size, chn_idx, False)
        else:
            canbuffer = (tsapi.TLIBCANFD * 100)()
            r = tsapi.tsfifo_receive_canfd_msgs(self._dev_handle_list[dev_idx].HwHandle, canbuffer, size, chn_idx, False)
        return [tsapi.tosun_convert_msg(msg) for msg in canbuffer]


    def can_recv_signals(self, dev_idx, chn_idx, msgid_or_msgname, signalnames, timeout:float=0.1, is_phy_val=True, **kwargs) -> Optional[dict]:
        db_msg = self.__find_msg_from_dbc(dev_idx, msgid_or_msgname)
        msg = self.can_recv(dev_idx=dev_idx, chn_idx=chn_idx, msgid_or_msgname=msgid_or_msgname, timeout=timeout)
        signal_dcit = dict.fromkeys(signalnames)
        if msg is not None:
            for signal_name in signal_dcit.keys():
                signal_dcit[signal_name] = self._dev_handle_list[dev_idx].get_signal_value(msg, signal_name)
                if not is_phy_val:
                    signal_dcit[signal_name] = ValueConverter.phy_to_raw(db_msg._signal_dict.get(signal_name, None), signal_dcit[signal_name])
        return signal_dcit

    def can_send_periodic(self, msg: CanMsg, period: float = 0.01, dev_idx=0, **kwargs):
        self._dev_handle_list[dev_idx].send_msg(msg, period, False, is_cyclic=True)

    def can_cancel_send_periodic(self, msg: CanMsg, dev_idx=0, **kwargs):
        newmsg = tsapi.msg_convert_tosun(msg)
        if isinstance(newmsg, tsapi.TLIBCANFD):
            tsapi.tsapp_delete_cyclic_msg_canfd(self._dev_handle_list[dev_idx].HwHandle, newmsg)
        else:
            tsapi.tsapp_delete_cyclic_msg_can(self._dev_handle_list[dev_idx].HwHandle, newmsg)

    def can_clear_rx_buf(self, chn_idx=None, dev_idx=0):
        chn_list = self._dev_channel_list[dev_idx] if chn_idx is None else [chn_idx]
        for chn in chn_list:
            tsapi.tsfifo_clear_canfd_receive_buffers(self._dev_handle_list[dev_idx].HwHandle, chn)
            tsapi.tsfifo_clear_can_receive_buffers(self._dev_handle_list[dev_idx].HwHandle, chn)

    def __find_msg_from_dbc(self, dev_idx, msgid_or_msgname):
        if msgid_or_msgname is None:
            return None
        find_name = isinstance(msgid_or_msgname, str)
        db = self._dev_handle_list[dev_idx].db
        msg = db.dbc_list_by_name.get(msgid_or_msgname) if find_name else db.dbc_list_by_id.get(msgid_or_msgname)
        return msg

    def lin_connect_channel(self, dev_idx, chn_idx, baudrate_kbps, lin_node_type, **kwargs):
        handle = self._dev_handle_list[dev_idx]
        ret1 = tsapi.tsapp_configure_baudrate_lin(handle.HwHandle, chn_idx, c_double(baudrate_kbps), LIN_PROTOCOL.LIN_PROTOCOL_21)
        ret2 = tsapi.tslin_set_node_funtiontype(handle.HwHandle, chn_idx, lin_node_type)
        if ret1 == 0 and ret2 == 0:
            self._lin_dev_channel_list[dev_idx].append(chn_idx)
        return ret1 | ret2

    def lin_set_user_callback(self, dev_idx, callback, **kwargs):
        self._lin_dev_event_list[dev_idx].LIN_USER_CALLBACK = callback
        return tsapi.tsapp_register_event_lin_whandle(self._dev_handle_list[dev_idx].HwHandle, self._lin_dev_event_list[dev_idx].ONLINEvent)

    def lin_cancel_user_callback(self, dev_idx=0, **kwargs):
        if self._dev_event_list[dev_idx].LIN_USER_CALLBACK is None:
            return 0
        self._dev_event_list[dev_idx].LIN_USER_CALLBACK = None
        return tsapi.tsapp_unregister_event_lin_whandle(self._dev_handle_list[dev_idx].HwHandle, self._lin_dev_event_list[dev_idx].ONLINEvent)

    def lin_reset_schedule_tables(self, dev_idx, chn_idx, **kwargs):
        handle = self._dev_handle_list[dev_idx].HwHandle
        tsapi.tslin_stop_lin_channel(handle,chn_idx)
        tsapi.tslin_batch_set_schedule_start(handle, chn_idx)
        tsapi.tslin_clear_schedule_tables(handle,chn_idx)

    def lin_add_schedule_frames(self, frames:List[LinMsg], slottime_ms, dev_idx, start_schedule, **kwargs):
        handle = self._dev_handle_list[dev_idx].HwHandle
        for frame in frames:
            tlibmsg = self._lin_msg_type_cast(frame)
            tlibmsg.FProperties = 0x00 if frame.is_rx else 0x01
            tsapi.tslin_batch_add_schedule_frame(handle, frame.channel, tlibmsg, slottime_ms)
        if start_schedule:
            self.lin_start_schedule_tables(chn_idx=frame.channel, dev_idx=dev_idx)

    def lin_start_schedule_tables(self, chn_idx, dev_idx, **kwargs):
        handle = self._dev_handle_list[dev_idx].HwHandle
        tsapi.tslin_batch_set_schedule_end(handle, chn_idx)
        tsapi.tslin_start_lin_channel(handle, chn_idx)

    def lin_clear_schedule_tables(self, dev_idx, chn_idx, **kwargs):
        handle = self._dev_handle_list[dev_idx].HwHandle
        tsapi.tslin_clear_schedule_tables(handle,chn_idx)

    def lin_msg(self, chn_idx, msg_id, data, dlc, is_rx, **kwargs):
        return LinMsg(channel=chn_idx, arbitration_id=msg_id, data=data, dlc=dlc, is_fd=False, bitrate_switch=False, is_rx=is_rx, is_extended_id=False)

    def lin_send(self, msg: LinMsg, dev_idx, timeout=0.01, is_sync:bool=False, **kwargs):
        handle = self._dev_handle_list[dev_idx].HwHandle
        tlibmsg = self._lin_msg_type_cast(msg)
        if is_sync:
            ret = tsapi.tsapp_transmit_lin_sync(handle, tlibmsg, int(timeout*1000))
        else:
            ret = tsapi.tsapp_transmit_lin_async(handle, tlibmsg)
        return ret

    def _lin_msg_type_cast(self, msg):
        if isinstance(msg, LinMsg):
            data_c_char_p = c_char_p(bytes(msg.data))
            arr_c_uint8 = cast(data_c_char_p, POINTER(c_uint8 * msg.dlc)).contents
            tlibmsg = TLIBLIN(FIdxChn=msg.channel,FDLC=msg.dlc, FIdentifier=msg.arbitration_id, FData=arr_c_uint8)
            return tlibmsg
        elif isinstance(msg, TLIBLIN):
            return None


class TsDevEvent():
    def __init__(self):
        self.CAN_USER_CALLBACK = None
        self.LIN_USER_CALLBACK = None
        self.ONCANFDEvent = tsapi.OnTx_RxFUNC_CANFD_WHandle(self.On_CANFD_Event)
        self.ONLINEvent = tsapi.OnTx_RxFUNC_LIN_WHandle(self.On_LIN_Event)

    def On_CANFD_Event(self, obj, lp_msg):
        if self.CAN_USER_CALLBACK:
            contents = lp_msg.contents
            msg = CanMsg(
                arbitration_id=contents.FIdentifier,
                timestamp=contents.FTimeUs,
                is_extended_id=((contents.FProperties & 0x4) != 0),
                is_rx=((contents.FProperties & 0x1) == 0),
                channel=contents.FIdxChn,
                is_fd=contents.FFDProperties & 0x1,
                dlc=contents.FDLC,
                data=contents.FData[:contents.FDLC],
            )
            self.CAN_USER_CALLBACK(msg)

    def On_LIN_Event(self, obj, lp_msg):
        if self.LIN_USER_CALLBACK:
            contents = lp_msg.contents
            msg = LinMsg(
                arbitration_id=contents.FIdentifier,
                timestamp=contents.FTimeUs,
                is_extended_id=0,
                is_rx=((contents.FProperties & 0x1) == 0),
                channel=contents.FIdxChn,
                is_fd=0,
                dlc=contents.FDLC,
                data=contents.FData[:contents.FDLC],
            )
            self.LIN_USER_CALLBACK(msg)
