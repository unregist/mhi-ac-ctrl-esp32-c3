# mhi-ac-ctrl-esp32-c3
Control MHI Airconditioning locally using your own ESP32-C3-DevkitM-1



This code is based on [@absalom-muc's MHI-AC-Ctrl](https://github.com/absalom-muc/MHI-AC-Ctrl), [@mriksman's esp32 homekit implementation](https://github.com/mriksman/esp32_homekit_mhi/blob/e4a8a4382b990c8e64463411c47e911d1741d9d1/main/main.c) and [@ginkage's MHI-AC-Ctrl-ESPHome](https://github.com/ginkage/MHI-AC-Ctrl-ESPHome).

Everything is still very much Work in Progress, the features that are there do work reliably at two AC units at @hberntsen's home :).

Note that you'll need to enable active mode in Home Assistant before sending commands to the AC will work. Without that enabled, the ESP will passively listen to data and you'll be able to use timers again via the IR remote.

