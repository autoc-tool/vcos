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

from env.config import config
from .base_class.base_types import CanLinDevEnum
from .base_class.base_types import DiagTPEnum

if config.RUNNING_ENV == 'SIM':
    _CAN_LIN_DEV = CanLinDevEnum.SocketCan
    _DIAG_DRV = DiagTPEnum.SocketCan
else:
    _CAN_LIN_DEV = CanLinDevEnum.TSMaster
    _DIAG_DRV = DiagTPEnum.TSMasterCan

if _CAN_LIN_DEV == CanLinDevEnum.SocketCan:
    from .socketcan.canlin_driver import CanLinDriver
elif _CAN_LIN_DEV == CanLinDevEnum.TSMaster:
    from .tsmaster.canlin_driver import CanLinDriver
else:
    raise ValueError(f'Unknown CAN LIN Devices: {_CAN_LIN_DEV}')

if _DIAG_DRV == DiagTPEnum.SocketCan:
    from .socketcan.docan import DoCan as DoX
elif _DIAG_DRV == DiagTPEnum.TSMasterCan:
    from .tsmaster.docan import DoCan as DoX
else:
    raise ValueError(f'Unknown DIAG DRV: {_DIAG_DRV}')


from .canlin_manager import CanLinManager
from .diag_manager import DiagManager, DoCanCfgParams

__all__ = ["CanLinManager", "DiagManager", "DoCanCfgParams"]