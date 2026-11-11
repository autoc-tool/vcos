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

import argparse
import sys
import os
import logging
import json5

logging.basicConfig(
    level=logging.DEBUG,
    format="[%(levelname)s - %(asctime)s] %(message)s",
    datefmt="%Y-%m-%d %H:%M:%S"
)

CURRENT_DIR = os.path.dirname(os.path.abspath(__file__))
def get_input_args():
    parser = argparse.ArgumentParser(description="Use this script to run pytest")
    parser.add_help
    parser.add_argument("-a", "--arch", required=True, default="",help="the arch name, see the fold under runner")
    parser.add_argument("-j", "--json5", required=True, default="", help="the json5 file in arch fold, using ',' to seperate different json5 name")
    parser.add_argument("-r", "--radius", required=True, default="0", help="the radius of test cases")
    parser.add_argument("-n", "--name", required=False, help="the name of test cases")
    parser.add_argument("-e", "--extention_pytest_args", type=str, required=False, default="", help="extention pytest args like -vs")
    if len(sys.argv) == 1:
        parser.logging.info_help(sys.stderr)
        sys.exit(1)
    args = parser.parse_args()
    return args


def run_pytest_case(args):
    cmds = []
    runners_path = CURRENT_DIR
    logging.info(runners_path)
    arch_path = os.path.join(runners_path, args.arch)
    for json5_file in args.json5.split(","):
        json5_path = os.path.join(arch_path, json5_file)
        json5_name = json5_file.split(".")[0]
        with open(json5_path, "r", encoding="utf-8") as f:
            dic = json5.load(f)
            radius = args.radius
            if radius not in ["0", "1", "2", "3"]:
                raise ValueError("radius should be in 0-3")
            if f"radius{radius}" in dic.keys():
                for group in dic[f"radius{radius}"]:
                    if (not args.name) or (args.name and args.name == group.get("name")):
                        logging.info(group)
                        cmd = f'pytest {args.extention_pytest_args} -m "{group.get("markers")}" {group.get("tests")} --parent_name={json5_name} --alluredir=report/{json5_name}_{group.get("name")}-ci-results --clean-alluredir'
                        cmds.append(cmd)
            else:
                raise ValueError(f"radius{radius} not defined in {json5_path}")

    logging.info(f"the whole cmds are: {cmds}")
    return cmds

if __name__ == "__main__":
    args = get_input_args()
    cmds = run_pytest_case(args)
    os.chdir(os.path.join(CURRENT_DIR, "..", "pytests"))
    ret = 0
    for cmd in cmds:
        ret = ret or os.system(cmd)
    sys.exit(ret)