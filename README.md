# TargetDisplayModeTools
These are some tools and notes on Apple's Target Display Mode for 2009-2011-era iMacs.

A few years ago during COVID lockdown I purchased a 27" 2011 iMac with a dead graphics card to use as a cheap external monitor and made a few notes along the way.
Key discoveries:
- New Macs can still use these iMacs as external displays when connected via appropriate Thunderbolt cables/adapters
- Target Display Mode works on macOS 10.13 High Sierra (the latest supported by these iMacs) by copying the binary `/usr/libexec/dpd` from a 10.9 Mavericks installer.
- Target Display Mode is handled by the motherboard and doesn't need a working graphics card (handy since failure of the Radeon 6xx0M chips in these iMacs is common). Note though that brightness control *does* depend on the graphics card so won't work.
- Target Display Mode can be activated without a physical keyboard or mouse using the Karabiner VirtualHIDDevice kext (`cmdf2` code in this repo). I alternated between toggling TDM over SSH and using an Apple Remote and [BetterTouchTool](https://folivora.ai).

Hopefully sharing these notes will keep a few more 27" iMacs from being thrown out.

## Contents
- `cmdf2` simulates connecting a physical keyboard and pressing Cmd-F2 to activate Target Display Mode. It uses Karabiner's VirtualHIDDevice kext and is based on [this example code](https://github.com/pqrs-org/Karabiner-VirtualHIDDevice-archived/tree/master/example/virtual_keyboard_example). Compile with the usual `make` and `make install` after installing Xcode Command Line Tools.
- `tdm.fish` contains fish shell functions for controlling TDM over SSH and e.g. demonstrates use of `cmdf2` via SSH.
- `com.local.LimitHDDFan.plist` is a `launchd` job file to limit fan speed on startup using smcFanControl.
 
## Overview
- Install fresh macOS 10.13 High Sierra from USB
- If GPU is faulty, disable the graphics kexts to force macOS to use software graphics. The macOS interface and VNC screen sharing will be very slow, but macOS should boot.
- Connect to network and install updates, enable SSH and VNC access for admin user. Keyboard and mouse no longer required.
- Replace `/usr/libexec/dpd` with version from latest Mavericks installer
- Compile or download `cmdf2` and move to e.g. `/usr/local/bin/cmdf2`
- If hard drive is missing, install smcFanControl and add `launchd` job to limit fan speed (more details below)
- The iMac can now be connected via a Thunderbolt cable and run `sudo cmdf2` via SSH to enter Target Display Mode.

## Booting macOS when the GPU is faulty
A common issue with the 2011 27” iMac is that the graphics card will fail, which looks like the iMac hanging on a grey screen after the boot progress bar completes.
One solution is to replace/upgrade the graphics card, for which there's a ridiculously long discussion about how/which cards to upgrade to [here](https://forums.macrumors.com/threads/2011-imac-graphics-card-upgrade.1596614/).

For Target Display Mode purposes, however, the iMac can be made to boot without using the graphics card (in software graphics mode) by moving the kernel extensions (kexts) `AMDRadeonX3000.kext` and `AMD6000Controller.kext` out of `/System/Library/Extensions` (and into e.g. `/System/Library/DisabledExtensions`) and `touch`ing the `/System/Library/Extensions` folder to force a rebuild of the kext cache.

The easiest way to achieve this is via Target Disk Mode: hold `T` as the iMac is booting, connect it to another Mac via Thunderbolt, and use Finder on that Mac to move the files. 

Otherwise it can be done by disabling SIP and booting into Single User Mode.

Disabling SIP is non-trivial since the dead graphics card will block booting into Recovery Mode.
[Some online sources](https://apple.stackexchange.com/questions/332587/single-user-recovery-mode-on-high-sierra-10-13-6) suggest there exists a Single User Recovery mode which can be accessed by holding Option to reach the boot options screen, then immediately holding Cmd-S after selecting the recovery partition. Should this elusive mode exist, SIP could then be disabled with `csrutil disable`, however, I could not manage to make this work.

Instead I found it easiest to disable SIP by booting to a macOS install USB for a version that pre-dates SIP (e.g. 10.10 Yosemite) and "manually" disabling SIP by running `nvram csr-active-config="w%00%00%00"` from the Terminal.

After disabling SIP, boot into Single User mode by holding Cmd-S as the iMac is booting. At the command line, run:
```
cd /System/Library
mkdir DisabledExtensions
mv Extensions/AMDRadeonX3000.kext DisabledExtensions/
mv Extensions/AMD6000Controller.kext DisabledExtensions/
touch Extensions
reboot
```

Some further notes:
- Brightness control of the display is handled by the graphics card and will no longer work
- `dmtest` can be used to restore a missing recovery partition as per [this blog post](https://davidjb.com/blog/2016/12/creating-a-macos-recovery-partition-without-reinstalling-osx-or-re-running-your-installer/)
- Some sources suggest moving all ATI* or AMD* kexts but this doesn’t appear to be necessary
- Some sources suggest changing nvram `boot-args` to `agc=0` or `agc=-1` but this also doesn’t appear to do anything on an iMac (might be relevant to MBP graphics switching)

## Getting Target Display Mode to work on macOS 10.13 High Sierra
Having disabled the graphics kexts as above, Target Display Mode will not start, and the following message may appear every 10 seconds in the Console: `com.apple.xpc.launchd[1] (com.apple.dpd[182]): Service exited with abnormal code: 75`.
`com.apple.dpd` is the service responsible for listening for Cmd-F2 and enabling Target Display Mode. 
The service runs the binary at `/usr/libexec/dpd`.
This issue seems to just affect `dpd` binaries from 10.10 Yosemite onwards (tested with 10.10.5, 10.11.6, and 10.13.6), and can be resolved by copying over an older `dpd` from a 10.9.5 Mavericks installer.

First, download the latest Mavericks installer from the Mac App Store using the link [here](https://support.apple.com/en-au/102662). It can also be downloaded using `mas-cli` with `mas install 675248567` if it has been purchased before, as per [this link](https://forums.macrumors.com/threads/can-someone-give-me-a-mavericks-download-link.2279301/).

The installer package can be directly mounted by running e.g. `hdiutil attach Downloads/Mavericks.pkg`. Then open `/Packages/BSD.pkg` in e.g. [Suspicious Package](https://mothersruin.com/software/SuspiciousPackage/) and copy `/usr/libexec/dpd` (MD5: `6355f7378348de906ad1ace46c936c8c`) to the same location on the iMac (the Finder shortcut Cmd-Shift-G to navigate to `/usr/libexec`, which is a hidden folder, may be useful here). 

A further note:
- `dpd` has an associated binary `dpaudiothru` responsible for passing through audio which seems to work fine on High Sierra, no replacement needed.

## Entering Target Display Mode without a physical keyboard or mouse
To enter TDM without any physical devices connected at all (e.g. over the network via SSH), install [Karabiner Elements](https://karabiner-elements.pqrs.org) v12.10.0, the latest version available for High Sierra, which includes a compiled and signed kext for [Karabiner-VirtualHIDDevice](https://github.com/pqrs-org/Karabiner-VirtualHIDDevice-archived).
Then, download the binary for `cmdf2` from the releases page (or download the source code and compile), move it to e.g. `/usr/local/bin`, and run with `sudo cmdf2`.
To not require a password every time with `sudo`, try [this](https://askubuntu.com/questions/159007/how-do-i-run-specific-sudo-commands-without-a-password) and specify e.g. `/usr/local/bin/cmdf2`.

Target Display Mode can now be toggled by running `sudo cmdf2` over the network via SSH, or automatically on startup using a `launchd` job, or by pressing a button on an Apple Remote (I used [BetterTouchTool](https://folivora.ai) to run commands from an Apple remote I had spare). 

What didn't work for me but might for others:
- `osascript` over SSH works on macOS 10.6.8 (apparently up to 10.9.5 according to some sources): `osascript -e 'tell application "System Events" to key code 144 using command down'`. Confirmed not working on 10.13.6 despite enabling Accessibility access for Terminal and Script Editor.
- [VirtualKVM](https://github.com/duanefields/VirtualKVM) automatically toggles TDM upon attach/detach of the Thunderbolt cable (and enable/disable of Bluetooth to transfer mouse/keyboard control), but requires an app running in the background on both devices. I couldn’t get it to work, possibly due to network issues, or possibly because I was daisy-chaining my iMac off an Apple Thunderbolt Display (Main MacBook Pro <-> Thunderbolt Display <-> iMac in Target Display Mode).
- <https://github.com/BlueM/cliclick> didn’t work over SSH, tried Cmd with F2, brightness, etc.
- Remapping using Karabiner as per [this SE post](https://apple.stackexchange.com/questions/170175/switch-to-target-display-mode-without-keyboard).
- Remapping keys with `hidutil` and `launchd` as per [here](https://rakhesh.com/mac/using-hidutil-to-map-macos-keyboard-keys/) and [here](https://hidutil-generator.netlify.app)

References:
- `cmdf2` is based on [this example code](https://github.com/pqrs-org/Karabiner-VirtualHIDDevice-archived/tree/master/example/virtual_keyboard_example)
- <https://github.com/pqrs-org/Karabiner-VirtualHIDDevice-archived/blob/master/src/include/karabiner_virtual_hid_device.hpp.tmpl>
- <https://opensource.apple.com/source/IOHIDFamily/IOHIDFamily-700/IOHIDFamily/AppleHIDUsageTables.h>

## Reducing fan speed when iMac hard drive is removed
The iMac fan will run at maximum speed if the hard drive is removed, because Apple hard drives contain a built-in temperature sensor, and removal of the sensor causes SMC to run the fan at maximum to be safe.

To solve this, install [smcFanControl](https://www.eidac.de). This includes the command-line tool `smc` for controlling fans, which can be automated using a `launchd` script (in this repo as `com.local.LimitHDDFan.plist`). E.g. `/Applications/smcFanControl.app/Contents/Resources/smc -k F1Mx -w 1130`.

To avoid overheating, iMac temperatures can be monitored over SSH by installing [HardwareMonitor](https://www.bresink.com/osx/HardwareMonitor.html). Run `/Applications/HardwareMonitor.app/Contents/MacOS/hwmonitor -c`.

References:
- `smc` tool info <https://discussions.apple.com/thread/2554924>
- launchd template: <https://superuser.com/questions/229773/run-command-on-startup-login-mac-os-x> (Note: use RunAtLoad instead of LaunchOnlyOnce)

## Roadmap and Contributions
The iMac I was using for this project has since had its power supply fail and I don't see any further work being done here.
However, if someone wants to build on the `cmdf2` code to write something that e.g. automatically enters Target Display Mode when the Thunderbolt cable is connected, let me know and I would be happy to link to it from here.

## License
These notes and code are released under the MIT license, see LICENSE.
