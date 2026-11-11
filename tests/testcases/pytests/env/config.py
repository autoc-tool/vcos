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

import json5
import os
from pathlib import Path

class ConfigLoader:
    class _Config:
        def __repr__(self):
            return f"<Config {self.__dict__}>"  # Config object representation

    def __init__(self):
        self._config = self._Config()
        self._loaded = False

    def _dict2obj(self, d, parent_obj):
        """Recursively convert a dictionary to object attributes"""
        for key, value in d.items():
            if isinstance(value, dict):
                new_obj = self._Config()
                self._dict2obj(value, new_obj)
                setattr(parent_obj, key, new_obj)
            else:
                setattr(parent_obj, key, value)

    def _convert_type(self, env_value, original_type):
        """Handle type conversion for environment variables"""
        if original_type is bool:
            return env_value.lower() in ('true', '1', 'yes')
        try:
            return original_type(env_value)
        except (ValueError, TypeError):
            return env_value  # Fallback to string type

    def _apply_env_overrides(self, node, parent_prefix=""):
        """Recursively apply environment variable overrides"""
        for attr_name in dir(node):
            if attr_name.startswith("__"):
                continue
            attr_value = getattr(node, attr_name)
            
            # Process nested configuration objects
            if isinstance(attr_value, self._Config):
                self._apply_env_overrides(attr_value, f"{parent_prefix}{attr_name}__")
            else:
                # Build environment variable name (e.g., RELAY__PORT)
                env_var = f"{parent_prefix}{attr_name}".upper()
                if (env_val := os.getenv(env_var)) is not None:
                    converted = self._convert_type(env_val, type(attr_value))
                    setattr(node, attr_name, converted)

    def load(self, env=None):
        """Load configuration and apply environment variable overrides"""
        if self._loaded and env is None:
            return

        config_path = Path(__file__).parent / "environment.json5"
        
        try:
            with open(config_path, "r", encoding="utf-8") as f:
                config_data = json5.load(f)
        except FileNotFoundError:
            raise RuntimeError(f"Missing config file: {config_path}")
        except ValueError:
            raise RuntimeError("Invalid config format")

        # Clear existing configuration
        for attr in dir(self._config):
            if not attr.startswith("__"):
                delattr(self._config, attr)

        self._dict2obj(config_data, self._config)
        self._apply_env_overrides(self._config)
        self._loaded = True

    def __getattr__(self, name):
        """Lazy-load configuration when accessing attributes"""
        if not self._loaded:
            self.load()
        return getattr(self._config, name)

# Singleton configuration object
config = ConfigLoader()

if __name__ == "__main__":
    print("RUNNING_ENV:", config.RUNNING_ENV)
    print("RELAY.PORT:", config.RELAY.PORT)