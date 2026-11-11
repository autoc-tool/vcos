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


class DiagBase(ABC):

    @abstractmethod
    def __init__(self, **kwargs):
        pass

    @abstractmethod
    def deinit(self):
        pass

    @abstractmethod
    def diag_config(self, **kwargs):
        pass

    @abstractmethod
    def diag_request(self, req, diag_index=0, **kwargs):
        pass

    def diag_get_response(self, diag_index=0, **kwargs):
        pass

    @abstractmethod
    def diag_request_and_get_response(self, req, diag_index=0, **kwargs):
        pass

    def get_error_description(self, ret_value):
        pass
