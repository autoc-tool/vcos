############################################################################
#  Copyright (c) 2025 Li Auto Inc. and its affiliates
#  Licensed under the Apache License, Version 2.0(the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at

#      http://www.apache.org/licenses/LICENSE-2.0

#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
############################################################################

from components.rt_framework.config_plug_in.generator import utils
from func_module.generator.common.common import FunctionBase
from components.rt_framework.config_plug_in.generator.context.functions import Function as OsFunction

from jinja2 import pass_context
from decimal import Decimal
from collections import defaultdict


class Function(FunctionBase):
    """
    Class containing all functions. Add new static functions here
    to be included in the jinja2 context
    """
    _cache = []

    @staticmethod
    def clear_caches():
        for ch in Function._cache:
            ch.clear_cache()

    @staticmethod
    @pass_context
    @utils.cache_once(_cache)
    def get_app_cfg(context):
        return OsFunction.get_app_cfg(context)

