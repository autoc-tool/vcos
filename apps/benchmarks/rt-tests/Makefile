############################################################################
# apps/benchmarks/rt-tests/Makefile
#
# SPDX-License-Identifier: Apache-2.0
#
# Licensed to the Apache Software Foundation (ASF) under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.  The
# ASF licenses this file to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance with the
# License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
# License for the specific language governing permissions and limitations
# under the License.
#
############################################################################

############################################################################
# Targets
############################################################################


include $(APPDIR)/Make.defs

PROGNAME  += cyclictest pmqtest pi_stress signaltest ptsematest hackbench
PROGNAME  += sigwaittest
MAINSRC   += rt-tests/src/cyclictest/cyclictest.c
MAINSRC   += rt-tests/src/pmqtest/pmqtest.c
MAINSRC   += rt-tests/src/pi_tests/pi_stress.c
MAINSRC   += rt-tests/src/signaltest/signaltest.c
MAINSRC   += rt-tests/src/ptsematest/ptsematest.c
MAINSRC   += rt-tests/src/hackbench/hackbench.c
MAINSRC   += rt-tests/src/sigwaittest/sigwaittest.c

PRIORITY  = $(CONFIG_BENCHMARK_RTTESTS_PRIORITY)
STACKSIZE = $(CONFIG_BENCHMARK_RTTESTS_STACKSIZE)
MODULE    = $(CONFIG_BENCHMARK_RTTESTS)

RTTESTS_PATH    = $(APPDIR)/benchmarks/rt-tests/rt-tests
RTTESTS_URL     = git://git.kernel.org/pub/scm/utils/rt-tests/rt-tests.git
RTTESTS_VERSION = cadd661f984c0e6717e681fdaca1ce589b0ed964
RTTESTS_GIT     = rt-tests

ifeq ($(wildcard rt-tests/.git),)
rt-tests:
	@echo "Git Cloning: $(RTTESTS_URL)"
	$(Q) git clone $(RTTESTS_URL)
	@echo "Checkout commit: $(RTTESTS_VERSION)"
	$(Q) cd $(RTTESTS_PATH) && \
	git checkout -q $(RTTESTS_VERSION)
	@echo "Patching: Applying patch"
	$(Q) cd $(RTTESTS_PATH) && \
	patch -p1 < ../0001-rt-tests-Port-for-NuttX.patch && \
	patch -p1 < ../0002-rt-sched-Port-for-NuttX.patch && \
	patch -p1 < ../0003-pi_stress-Port-for-NuttX.patch && \
	patch -p1 < ../0004-signaltest-Port-for-NuttX.patch && \
	patch -p1 < ../0005-hackbench-Port-for-NuttX.patch && \
	patch -p1 < ../0006-pmqtest-Port-for-NuttX.patch && \
	patch -p1 < ../0007-ptsematest-Port-for-NuttX.patch && \
	patch -p1 < ../0008-sigwaittest-Port-for-NuttX.patch

context:: rt-tests
endif

distclean::
	$(call DELDIR, $(RTTESTS_GIT))

CSRCS     = rt-tests/src/lib/rt-error.c rt-tests/src/lib/histogram.c
CSRCS    += rt-tests/src/lib/rt-utils.c rt-tests/src/lib/rt-numa.c
CSRCS    += rt-tests/src/lib/rt-get_cpu.c
CSRCS    += rt-tests/src/lib/rt-sched.c

CFLAGS   += -Irt-tests/src/include -I. -DVERSION=0.1
CFLAGS   += -D_POSIX_THREAD_PRIO_INHERIT=1 -DSYS_gettid=0 -Dgettid=rttest_gettid -D'syscall(x)=((pid_t)(x))'
CFLAGS   += -w -DRTTESTS_PRIORITY=$(CONFIG_BENCHMARK_RTTESTS_PRIORITY)+1

include $(APPDIR)/Application.mk
