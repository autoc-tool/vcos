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

from enum import IntEnum

class CHANNEL_INDEX(IntEnum):
    (CHN1, CHN2, CHN3, CHN4, CHN5, CHN6, CHN7, CHN8, CHN9, CHN10, CHN11, CHN12, CHN13, CHN14, CHN15, CHN16, CHN17,
        CHN18, CHN19, CHN20, CHN21, CHN22, CHN23, CHN24, CHN25, CHN26, CHN27, CHN28, CHN29, CHN30, CHN31, CHN32) = (
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
        11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
        21, 22, 23, 24, 25, 26, 27, 28, 29,
        30, 31
    )

CAN_CHANNEL_MAPPING = [
    'vcan0', 'vcan1', 'vcan2', 'vcan3', 'vcan4', 'vcan5', 'vcan6', 'vcan7', 'vcan8', 'vcan9',
    'vcan10', 'vcan11', 'vcan12', 'vcan13', 'vcan14', 'vcan15', 'vcan16', 'vcan17', 'vcan18', 'vcan19',
    'vcan20', 'vcan21', 'vcan22', 'vcan23', 'vcan24', 'vcan25', 'vcan26', 'vcan27', 'vcan28', 'vcan29',
    'vcan30', 'vcan31'
]

LIN_CHANNEL_MAPPING = [
    'vlin0', 'vlin1', 'vlin2', 'vlin3', 'vlin4', 'vlin5', 'vlin6', 'vlin7', 'vlin8', 'vlin9',
    'vlin10', 'vlin11', 'vlin12', 'vlin13', 'vlin14', 'vlin15', 'vlin16', 'vlin17', 'vlin18', 'vlin19',
    'vlin20', 'vlin21', 'vlin22', 'vlin23', 'vlin24', 'vlin25', 'vlin26', 'vlin27', 'vlin28', 'vlin29',
    'vlin30', 'vlin31'
]
