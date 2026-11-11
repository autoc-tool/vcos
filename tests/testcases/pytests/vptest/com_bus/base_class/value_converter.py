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
import cantools


class ValueConverter:

    @classmethod
    def phy_to_raw(cls, sig_info, val):
        if sig_info is None:
            logging.error("No signal info available")
            return val
        if isinstance(sig_info, cantools.database.can.signal.Signal):
            if isinstance(val, cantools.database.namedsignalvalue.NamedSignalValue):
                return val.value
            return (val - sig_info.offset) / sig_info.scale

    @classmethod
    def raw_to_phy(cls, sig_info, val):
        if sig_info is None:
            logging.error("No signal info available")
            return val
        if isinstance(sig_info, cantools.database.can.signal.Signal):
            if isinstance(val, str):
                return val
            return val * sig_info.scale + sig_info.offset
