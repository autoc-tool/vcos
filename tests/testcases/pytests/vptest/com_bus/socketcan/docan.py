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

import logging
import can
import isotp
from typing import Optional
from ..base_class.diag_base import DiagBase
from .env_cfg import CAN_CHANNEL_MAPPING

class DoCan(DiagBase):
    def __init__(self, **kwargs):
        self._diag_handle_phys = []
        self._diag_handle_func = []

    def deinit(self):
        for handle in self._diag_handle_phys:
            handle.stop()
            handle.bus.shutdown()
        self._diag_handle_phys.clear()

        for handle in self._diag_handle_func:
            handle.stop()
            handle.bus.shutdown()
        self._diag_handle_func.clear()

    def diag_config(self, chn_idx, phys_id, resp_id, func_id, can_fd=True,
                    tx_data_length=64, tx_padding=0xAA, blocking_send=True, **kwargs):
        channel = self.__get_channel(chn_idx)
        bus = can.interface.Bus(channel=channel, interface='socketcan', fd=can_fd)
        params = {'tx_data_length': tx_data_length, 'tx_data_min_length': 8, 'tx_padding': tx_padding, 'can_fd': can_fd, 'blocking_send': True}
        isotp_address = isotp.Address(txid=phys_id, rxid=resp_id)
        self._diag_handle_phys.append(isotp.CanStack(bus=bus, address=isotp_address, params=params))
        self._diag_handle_phys[-1].start()
        bus_func = can.interface.Bus(channel=channel, interface='socketcan', fd=can_fd)
        isotp_address_func = isotp.Address(txid=func_id, rxid=resp_id)
        self._diag_handle_func.append(isotp.CanStack(bus=bus_func, address=isotp_address_func, params=params))
        self._diag_handle_func[-1].start()
        errorno = 0
        return errorno, len(self._diag_handle_phys)-1

    def diag_request(self, req: bytearray, diag_index=0, is_phys=True, timeout=1, **kwargs):
        try:
            handle = self.__get_diag_handle(diag_index, is_phys)
            handle.send(req, send_timeout=timeout)
        except Exception as e:
            logging.error(f'diag_send_request: {e}')
            return -1
        return 0

    def diag_get_response(self, diag_index=0, is_phys=True, timeout=1, **kwargs) -> Optional[bytearray]:
        rx_payload = None
        try:
            handle = self.__get_diag_handle(diag_index, is_phys)
            rx_payload = handle.recv(block=True, timeout=timeout)
        except Exception as e:
            logging.error(f'diag_get_response: {e}')
        rx_payload = bytearray(0) if rx_payload is None else rx_payload
        return rx_payload

    def diag_request_and_get_response(self, req:bytearray, diag_index=0, is_phys=True, tx_timeout=0.5, rx_timeout=0.5, **kwargs) -> Optional[bytearray]:
        rx_payload = None
        error_code = 0
        try:
            self.diag_request(diag_index=diag_index, req=req, is_phys=is_phys, timeout=tx_timeout)
            rx_payload = self.diag_get_response(diag_index=diag_index, is_phys=is_phys, timeout=rx_timeout)
        except Exception as e:
            error_code = 1
            logging.error(f'diag_request_and_get_response: {e}')
        rx_payload = bytearray(0) if rx_payload is None else rx_payload
        return error_code, rx_payload

    def __get_channel(self, chn_idx):
        return CAN_CHANNEL_MAPPING[chn_idx]

    def __get_diag_handle(self, diag_idx, is_phys) -> Optional[isotp.CanStack]:
        return self._diag_handle_phys[diag_idx] if is_phys else self._diag_handle_func[diag_idx]
