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

from ctypes import c_uint32, c_uint8, c_int32, c_char_p
import libTSCANAPI as tsapi
from .canlin_driver import CanLinDriver
from ..base_class.diag_base import DiagBase


class DoCan(DiagBase):
    def __init__(self, canlin_driver: CanLinDriver = None, **kwargs):
        tsapi.initialize_lib_tscan(True, True, False)
        self._canlin_driver = canlin_driver
        self._diag_handle = []
        self._resp_buf = (c_uint8*4096)()

    def deinit(self):
        tsapi.finalize_lib_tscan()

    def diag_config(self, chn_idx, phys_id, resp_id, func_id, can_fd=True,
                    tx_data_length=64, device_serial=None, **kwargs):
        pDiagModuleIndex = c_uint8(0)
        if self._canlin_driver is None:
            self._canlin_driver = CanLinDriver()
            error_code, dev_idx = self._canlin_driver.device_connect(device_serial=device_serial)
            self._canlin_driver.can_connect_channel(dev_idx=dev_idx, chn_idx=chn_idx)
        else:
            dev_idx = self._canlin_driver._dev_serial_list.index(device_serial) if device_serial is not None else 0
        r = tsapi.tsdiag_can_create(self._canlin_driver._dev_handle_list[dev_idx].HwHandle, pDiagModuleIndex=pDiagModuleIndex, AChnIndex=chn_idx, ASupportFDCAN=can_fd,
                                    AMaxDLC=tx_data_length, ARequestID=phys_id, ARequestIDIsStd=True, AResponseID=resp_id, AResponseIDIsStd=True,
                                    AFunctionID=func_id, AFunctionIDIsStd=True)
        if (r != 0):
            print("create uds handle failed!")
            return -1, 0
        else:
            print("uds handle = ", pDiagModuleIndex)
            self._diag_handle.append(pDiagModuleIndex)
            return r, len(self._diag_handle) - 1

    def diag_request(self, req, diag_index=0, **kwargs):
        senddata = (c_uint8 * len(req))(*req)
        error_code = tsapi.tstp_can_send_request(self._diag_handle[diag_index], senddata, len(senddata))
        return error_code

    def diag_request_and_get_response(self, req, diag_index=0, **kwargs):
        senddata = (c_uint8 * len(req))(*req)
        resp_len = c_int32(len(self._resp_buf))
        error_code = tsapi.tstp_can_request_and_get_response(self._diag_handle[diag_index], senddata, len(senddata), self._resp_buf, resp_len, ATimeOutMs=5000)
        resp_bytearray = bytearray(self._resp_buf[:resp_len.value])
        return error_code, resp_bytearray

    def get_error_description(self, ret_value):
        if 0 != ret_value:
            description_str = c_char_p()
            if 'Error code not vaild' == description_str:
                return tsapi.tscan_get_error_description(ret_value)
            return str(description_str.value)
        else:
            return "success"
