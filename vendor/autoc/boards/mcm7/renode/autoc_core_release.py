from Antmicro.Renode.Peripherals.Bus import Access, SysbusAccessWidth

def mc_autoc_core_release():
    bus = monitor.Machine.SystemBus

    def release_core(cpu, address, width, value):
        core = int(value) & 0xFF
        if core >= 1 and core <= 3:
            monitor.Parse("sysbus.cpu%d IsHalted false" % core)

    bus.AddWatchpointHook(0x40001000, SysbusAccessWidth.DoubleWord,
                          Access.Write, release_core)
