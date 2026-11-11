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

from dataclasses import dataclass, fields
from .canlin_manager import CanLinManager
from . import DoX


@dataclass
class DoCanCfgParams:
    chn_idx: int = 0
    phys_id: int = 0x7E1
    resp_id: int = 0x7E9
    func_id: int = 0x7DF
    can_fd: bool = True
    tx_data_length: int = 64
    device_serial: str = None

    def to_dict(self):
        return {field.name: getattr(self, field.name) for field in fields(self)}


@dataclass
class DoIpCfgParams:
    local_ip_address: str = ""
    local_port: int = 0
    remote_ip_address: str = ""
    remote_port: int = 13400
    source_address: str = "0E02"
    target_address: str = "0409"

    def to_dict(self):
        return {field.name: getattr(self, field.name) for field in fields(self)}


class DiagManager(DoX):
    def __init__(self, canlin_manager: CanLinManager = None):
        self._canlin_manager = canlin_manager
        super().__init__(canlin_driver=canlin_manager)

    def init(self):
        self.deinit()
        self.__init__(canlin_manager=self._canlin_manager)

    def deinit(self):
        super().deinit()

    def diag_config(self, do_can_cfg=DoCanCfgParams(), do_ip_cfg=DoIpCfgParams(), **kwargs):
        params = do_can_cfg.to_dict()
        params.update(do_ip_cfg.to_dict())
        error_code, diag_idx = super().diag_config(**params)
        return error_code, diag_idx

    def diag_request(self, req, diag_index=0, **kwargs):
        return super().diag_request(req=req, diag_index=diag_index, **kwargs)

    def diag_request_and_get_response(self, req, diag_index=0):
        error_code, resp_bytearray = super().diag_request_and_get_response(req=req, diag_index=diag_index)
        return error_code, resp_bytearray

    def diag_service_10_session_control(self, session_type, diag_index=0, check_positive_resp=False, is_phys=True):
        service_id = 0x10
        reqdata = bytearray([service_id, session_type])
        expect_data = [service_id+0x40, session_type]
        error_code, rx_payload = self.diag_request_and_get_response(req=reqdata, diag_index=diag_index)
        error_code = self.__error_code_update(check_positive_resp=check_positive_resp, error_code=error_code, rx_payload=rx_payload, expect_data=expect_data)
        return error_code, rx_payload

    def diag_service_11_reset_ecu(self, reset_type, diag_index=0, check_positive_resp=False, is_phys=True):
        service_id = 0x11
        reqdata = bytearray([service_id, reset_type])
        expect_data = [service_id+0x40, reset_type]
        error_code, rx_payload = self.diag_request_and_get_response(req=reqdata, diag_index=diag_index)
        error_code = self.__error_code_update(check_positive_resp=check_positive_resp, error_code=error_code, rx_payload=rx_payload, expect_data=expect_data)
        return error_code, rx_payload

    def diag_service_14_clear_dtc(self, dtc_group=[0xFF, 0xFF, 0xFF], diag_index=0, check_positive_resp=False, is_phys=True):
        service_id = 0x14
        reqdata = bytearray([service_id] + dtc_group)
        expect_data = [service_id+0x40]
        error_code, rx_payload = self.diag_request_and_get_response(diag_index=diag_index, req=reqdata)
        error_code = self.__error_code_update(check_positive_resp=check_positive_resp, error_code=error_code, rx_payload=rx_payload, expect_data=expect_data)
        return error_code, rx_payload

    def diag_service_22_read_data_by_did(self, did: list = [0xF1, 0x90], diag_index=0, check_positive_resp=False, is_phys=True):
        service_id = 0x22
        reqdata = bytearray([service_id] + did)
        expect_data = [service_id+0x40] + did
        error_code, rx_payload = self.diag_request_and_get_response(diag_index=diag_index, req=reqdata)
        error_code = self.__error_code_update(check_positive_resp=check_positive_resp, error_code=error_code, rx_payload=rx_payload, expect_data=expect_data)
        return error_code, rx_payload

    def diag_service_2e_write_data_by_did(self, did, data: list, diag_index=0, check_positive_resp=False, is_phys=True):
        service_id = 0x2E
        reqdata = bytearray([service_id] + did + data)
        expect_data = [service_id+0x40] + did
        error_code, rx_payload = self.diag_request_and_get_response(diag_index=diag_index, req=reqdata)
        error_code = self.__error_code_update(check_positive_resp=check_positive_resp, error_code=error_code, rx_payload=rx_payload, expect_data=expect_data)
        return error_code, rx_payload

    def diag_service_27_security_access_request_seed(self, sub_function, diag_index=0, check_positive_resp=False, is_phys=True):
        service_id = 0x27
        reqdata = bytearray([service_id, sub_function])
        expect_data = [service_id+0x40, sub_function]
        error_code, rx_payload = self.diag_request_and_get_response(diag_index=diag_index, req=reqdata)
        error_code = self.__error_code_update(check_positive_resp=check_positive_resp, error_code=error_code, rx_payload=rx_payload, expect_data=expect_data)
        return error_code, rx_payload

    def diag_service_27_security_access_send_key(self, sub_function, key_array, diag_index=0, check_positive_resp=False, is_phys=True):
        service_id = 0x27
        reqdata = bytearray([service_id, sub_function] + key_array)
        expect_data = [service_id+0x40, sub_function]
        error_code, rx_payload = self.diag_request_and_get_response(diag_index=diag_index, req=reqdata)
        error_code = self.__error_code_update(check_positive_resp=check_positive_resp, error_code=error_code, rx_payload=rx_payload, expect_data=expect_data)
        return error_code, rx_payload

    def diag_service_2f_io_control(self, data_id: list, control_opt_record: list,  mask_record :list = [], diag_index=0, check_positive_resp=False, is_phys=True):
        service_id = 0x2F
        reqdata = bytearray([service_id] + data_id + control_opt_record + mask_record)
        expect_data = [service_id+0x40] + data_id
        error_code, rx_payload = self.diag_request_and_get_response(diag_index=diag_index, req=reqdata)
        error_code = self.__error_code_update(check_positive_resp=check_positive_resp, error_code=error_code, rx_payload=rx_payload, expect_data=expect_data)
        return error_code, rx_payload

    def diag_service_3d_write_memory(self, mem_addr: list, mem_size: list, data: list, diag_index=0, check_positive_resp=False, is_phys=True):
        service_id = 0x3D
        addr_len_format_id = ((len(mem_size) & 0x0F) << 4) | (len(mem_addr) & 0x0F)
        reqdata = bytearray([service_id, addr_len_format_id] + mem_addr + mem_size + data)
        expect_data = [service_id+0x40]
        error_code, rx_payload = self.diag_request_and_get_response(diag_index=diag_index, req=reqdata)
        error_code = self.__error_code_update(check_positive_resp=check_positive_resp, error_code=error_code, rx_payload=rx_payload, expect_data=expect_data)
        return error_code, rx_payload

    def diag_service_23_read_memory(self, mem_addr: list, mem_size: list, diag_index=0, check_positive_resp=False, is_phys=True):
        service_id = 0x23
        addr_len_format_id = ((len(mem_size) & 0x0F) << 4) | (len(mem_addr) & 0x0F)
        reqdata = bytearray([service_id, addr_len_format_id] + mem_addr + mem_size)
        expect_data = [service_id+0x40]
        error_code, rx_payload = self.diag_request_and_get_response(diag_index=diag_index, req=reqdata)
        error_code = self.__error_code_update(check_positive_resp=check_positive_resp, error_code=error_code, rx_payload=rx_payload, expect_data=expect_data)
        return error_code, rx_payload

    def diag_service_38_request_file_transfer(self, mode_operation, file_path_and_name_length: list, filename: list,
                                              dataformatid=[], filesizelen=[], sizeuncompressed=[], sizecompressed=[], diag_index=0, check_positive_resp=False, is_phys=True):
        service_id = 0x38
        reqdata = bytearray([service_id, mode_operation] + file_path_and_name_length + filename + dataformatid + filesizelen + sizeuncompressed + sizecompressed)
        expect_data = [service_id+0x40]
        error_code, rx_payload = self.diag_request_and_get_response(diag_index=diag_index, req=reqdata)
        error_code = self.__error_code_update(check_positive_resp=check_positive_resp, error_code=error_code, rx_payload=rx_payload, expect_data=expect_data)
        return error_code, rx_payload

    def __error_code_update(self, check_positive_resp, error_code, rx_payload, expect_data):
        if check_positive_resp:
            expect_len = len(expect_data)
            error_code = error_code if len(rx_payload) >= expect_len and list(rx_payload[:expect_len]) == expect_data else 1
        return error_code

    def get_error_description(self, ret_value):
        return super().get_error_description(ret_value)
