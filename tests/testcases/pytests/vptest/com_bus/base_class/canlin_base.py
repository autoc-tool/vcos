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

from abc import ABC, abstractmethod


class CanLinDriverBase(ABC):

    @abstractmethod
    def deinit(self):
        pass

    @abstractmethod
    def device_connect(self):
        pass

    @abstractmethod
    def can_connect_channel(self):
        pass

    @abstractmethod
    def load_dbc(self):
        pass

    @abstractmethod
    def can_msg(self):
        pass

    @abstractmethod
    def can_msg_by_dbc(self):
        pass

    @abstractmethod
    def can_send(self):
        pass

    @abstractmethod
    def can_recv(self):
        pass

    @abstractmethod
    def can_recv_signals(self):
        pass

    @abstractmethod
    def can_send_periodic(self):
        pass

    @abstractmethod
    def can_cancel_send_periodic(self):
        pass

    @abstractmethod
    def can_set_user_callback(self):
        pass

    @abstractmethod
    def can_cancel_user_callback(self):
        pass

    @abstractmethod
    def can_clear_rx_buf(self):
        pass
