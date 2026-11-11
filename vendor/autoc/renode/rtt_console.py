#
# Host side of SEGGER RTT. The guest runs the real library and owns
# _SEGGER_RTT. This timer only copies the up/down rings to the console.
#

import System
from Antmicro.Renode.Peripherals.Timers import LimitTimer

_rtt_timer = None

def _u32(bus, address):
    return (bus.ReadByte(address)
            | (bus.ReadByte(address + 1) << 8)
            | (bus.ReadByte(address + 2) << 16)
            | (bus.ReadByte(address + 3) << 24))

def _put_u32(bus, address, value):
    bus.WriteByte(address, value & 0xff)
    bus.WriteByte(address + 1, (value >> 8) & 0xff)
    bus.WriteByte(address + 2, (value >> 16) & 0xff)
    bus.WriteByte(address + 3, (value >> 24) & 0xff)

def _read_up(bus, console, channel):
    buffer = _u32(bus, channel + 4)
    size = _u32(bus, channel + 8)
    write_offset = _u32(bus, channel + 12)
    read_offset = _u32(bus, channel + 16)

    if buffer == 0 or size < 2 or write_offset >= size or read_offset >= size:
        return
    if write_offset == read_offset:
        return

    address = read_offset
    while address != write_offset:
        console.DisplayChar(bus.ReadByte(buffer + address))
        address += 1
        if address == size:
            address = 0

    _put_u32(bus, channel + 16, write_offset)

def _write_down(bus, console, channel):
    buffer = _u32(bus, channel + 4)
    size = _u32(bus, channel + 8)
    write_offset = _u32(bus, channel + 12)
    read_offset = _u32(bus, channel + 16)

    if buffer == 0 or size < 2 or write_offset >= size or read_offset >= size:
        return

    if write_offset >= read_offset:
        used = write_offset - read_offset
    else:
        used = (size - read_offset) + write_offset

    free = (size - 1) - used
    if free <= 0 or console.IsEmpty():
        return

    data = console.ReadBuffer(free)
    count = len(data)
    if count == 0:
        return
    if count > free:
        count = free

    address = write_offset
    for index in range(count):
        bus.WriteByte(buffer + address, data[index])
        address += 1
        if address == size:
            address = 0

    _put_u32(bus, channel + 12, address)

def _poll(bus, console, control_block):
    try:
        marker = ""
        for offset in range(10):
            marker += chr(bus.ReadByte(control_block + offset))
        if marker != "SEGGER RTT":
            return

        up_count = _u32(bus, control_block + 16)
        if up_count == 0 or up_count > 16:
            return

        up = control_block + 24
        down = up + (24 * up_count)
        _read_up(bus, console, up)
        _write_down(bus, console, down)
    except Exception:
        return

def mc_setup_rtt_console(console):
    global _rtt_timer

    bus = monitor.Machine.SystemBus
    cpu = None
    for candidate in bus.GetCPUs():
        cpu = candidate
        break

    if cpu is None:
        return

    found, addresses = bus.TryGetAllSymbolAddresses("_SEGGER_RTT", context=cpu)
    if not found:
        cpu.WarningLog("Symbol '_SEGGER_RTT' not found. Load the ELF before setup_rtt_console.")
        return

    control_block = list(addresses)[0]
    console.Echo = False

    if _rtt_timer is not None:
        _rtt_timer.Enabled = False

    def tick():
        _poll(bus, console, control_block)

    timer = LimitTimer(monitor.Machine.ClockSource, 1000, None, "rtt-console", 1)
    timer.AutoUpdate = True
    timer.EventEnabled = True
    timer.LimitReached += System.Action(tick)
    timer.Enabled = True
    _rtt_timer = timer
