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

import sys
import re
from pathlib import Path

def process_file(output_path, input_path, include_paths, processed=None):
    if processed is None:
        processed = set()

    input_path = Path(input_path).resolve()
    if input_path in processed:
        return
    processed.add(input_path)

    with input_path.open('r', encoding='utf-8') as f:
        lines = f.readlines()

    with output_path.open('a', encoding='utf-8') as out:
        for line in lines:
            line_strip = line.strip()
            match = re.match(r'#include\s*[<"]([^">]+)[">]', line_strip)
            if match:
                include_file = match.group(1)
                found = False

                direct_path = input_path.parent / include_file
                if direct_path.exists():
                    process_file(output_path, direct_path, include_paths, processed)
                    found = True
                else:
                    for path in include_paths:
                        candidate = Path(path) / include_file
                        if candidate.exists():
                            process_file(output_path, candidate, include_paths, processed)
                            found = True
                            break

                if not found:
                    print(f'ERROR: Cannot find "{include_file}" from {input_path}', file=sys.stderr)
                    if output_path.exists():
                        output_path.unlink()
                    sys.exit(1)
                continue
            out.write(line)
        out.write("\n")

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print("Usage: process_includes.py <output_file> <input_file> [include_paths...]", file=sys.stderr)
        sys.exit(1)

    output_file = Path(sys.argv[1])
    input_file = sys.argv[2]
    include_dirs = sys.argv[3:]

    if output_file.exists():
        output_file.unlink()

    process_file(output_file, input_file, include_dirs)
