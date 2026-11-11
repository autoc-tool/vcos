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
"""
COM模板内函数定义模块.
定义jinja2模板中所使用的函数.
"""

from jinja2 import pass_context

from func_module.generator.common.common import FunctionBase

signal_type_map = {
    "COM_BOOLEAN": "bool",
    "COM_FLOAT32": "float",
    "COM_FLOAT64": "double",
    "COM_SINT64": "sint64",
    "COM_SINT32": "sint32",
    "COM_SINT16": "sint16",
    "COM_SINT8": "sint8",
    "COM_UINT64": "uint64",
    "COM_UINT32": "uint32",
    "COM_UINT16": "uint16",
    "COM_UINT8": "uint8",
    "COM_UINT8_N": "uint8"
}


class Function(FunctionBase):
    """
    Class containing all functions. Add new static functions here
    to be included in the jinja2 context
    """

    @staticmethod
    @pass_context
    def get_com_json_config(context):
        com_json_config = context["com_json"]["config"]
        return com_json_config

    @staticmethod
    @pass_context
    def get_com_controller(context):
        can_controller_list = context["com_json"]["ipdu_group_refs"]
        return can_controller_list

    @staticmethod
    @pass_context
    def get_com_controller_num(context):
        can_controller_list = context["com_json"]["ipdu_group_refs"]
        return len(can_controller_list)

    @staticmethod
    @pass_context
    def GetSignalId(context, signal_name, dictionary):
        """
        get signal signal_id
        """

        _ = context
        # print("signal_name", signal_name)
        # print("g_config", g_config)
        return dictionary["signals"][signal_name]["signal_id"]

    @staticmethod
    @pass_context
    def GetSignalObject(context, signal_name, dictionary):
        """
        get signal object
        """
        _ = context
        return dictionary[signal_name]

    @staticmethod
    @pass_context
    def GetSignalType(context, signalObj):
        """
        get signal type
        """
        _ = context
        if signalObj["is_float"]:
            if signalObj["length"] <= 32:
                return "COM_FLOAT32"
            else:
                return "COM_FLOAT64"
        elif signalObj["is_signed"]:
            if signalObj["length"] > 32:
                return "COM_SINT64"
            elif signalObj["length"] > 16:
                return "COM_SINT32"
            elif signalObj["length"] > 8:
                return "COM_SINT16"
            elif signalObj["length"] > 1:
                return "COM_SINT8"
            else:
                return "COM_BOOLEAN"
        else:
            if signalObj["length"] > 64:
                return "COM_UINT8_N"
            elif signalObj["length"] > 32:
                return "COM_UINT64"
            elif signalObj["length"] > 16:
                return "COM_UINT32"
            elif signalObj["length"] > 8:
                return "COM_UINT16"
            elif signalObj["length"] > 1:
                return "COM_UINT8"
            else:
                return "COM_BOOLEAN"

    @staticmethod
    @pass_context
    def GetSignalVariableType(context, signalObj):
        """
        map signal_type to platform data type
        """
        _ = context
        return signal_type_map[signalObj["ComSignalType"]["Value"]]

    @staticmethod
    @pass_context
    def GetIPduObject(context, message_name, dictionary):
        """
        get ipdu object
        """
        _ = context
        return dictionary[message_name]
    
    @staticmethod
    @pass_context
    def GetGeneralObject(context, name, dictionary):
        """
        get ipdu object
        """
        _ = context
        return dictionary[name]


    @staticmethod
    @pass_context
    def GetMainFunctionObject(context, mainfunction_name, dictionary):
        """
        get mainfunction object
        """
        _ = context
        return dictionary[mainfunction_name]

    @staticmethod
    @pass_context
    def GetIPduGroupObject(context, ipdu_group_name, dictionary):
        """
        get ipdu group object
        """
        _ = context
        return dictionary[ipdu_group_name]

    @staticmethod
    @pass_context
    def GetRoutePathObject(context, route_path, dictionary):
        """
        get route_path object
        """
        _ = context
        return dictionary[route_path]

    @staticmethod
    @pass_context
    def GetDestSignalObject(context, dest_signal_name, dictionary):
        """
        get dest_signal object
        """
        _ = context
        return dictionary[dest_signal_name]

    @staticmethod
    @pass_context
    def GetListLen(context, obj):
        """
        get list length
        """
        _ = context
        return len(obj)

    @staticmethod
    @pass_context
    def CalSignalEndposition(length, start):
        """
        calculate the end position of signal
        """
        # print("length:",length, "start:", start, "res:", (start ^ 7 + length - 1) ^ 7, "\n")
        if start != -1:
            return (start ^ 7 + length - 1) ^ 7
        else:
            return -1
