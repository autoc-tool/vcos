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

import can
import threading
import os
import time
import atexit

def _flush_and_close_writer(writer:can.ASCWriter):
    if writer and not writer.file.closed:
        writer.file.flush()
        os.fsync(writer.file.fileno())
        writer.file.close()
        writer.stop()

class CanLinListener(can.Listener):
    def __init__(self, writer:can.ASCWriter):
        self.writer = writer
        self.lock = threading.Lock()
        atexit.register(_flush_and_close_writer, self.writer)

    def on_message_received(self, msg):
        with self.lock:
            self.writer.on_message_received(msg)

    def close(self):
        _flush_and_close_writer(self.writer)
