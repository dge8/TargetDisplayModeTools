# TargetDisplayModeTools
These are some tools and notes on Apple's Target Display Mode for 2009-2011-era iMacs.

A few years ago during COVID lockdown I purchased a broken 27" 2011 iMac to use as a cheap external monitor and made a few notes along the way. Key discoveries:
- Newer Macs can use these iMacs as external displays when connected via appropriate Thunderbolt cables/adapters
- Target Display Mode works on macOS 10.13 High Sierra (the latest supported by these iMacs) by copying the binary `/usr/libexec/dpd` from a 10.9 Mavericks installer.
- Target Display Mode is handled by the motherboard and doesn't need a working GPU (handy since failure of the Radeon 6xx0M series GPUs in these iMacs is common)
- Target Display Mode can be activated over SSH using the Karabiner VirtualHIDDevice kext (`cmdf2` code in this repo) so no keyboard or mouse is needed

Hopefully sharing these notes will keep a few more 27" iMacs from being thrown out.

## Repo contents
- `cmdf2` simulates connecting a physical keyboard and pressing Cmd-F2 to activate Target Display Mode. It works over SSH and even without console login (IIRC. Otherwise create a regular user and enable auto-login). It uses Karabiner's VirtualHIDDevice kext and is based on [this example code](https://github.com/pqrs-org/Karabiner-VirtualHIDDevice-archived/tree/master/example/virtual_keyboard_example). You'll need Xcode Command Line Tools to compile.
- `tdm.fish` contains a fish shell function for controlling TDM over SSH and e.g. demonstrates use of `cmdf2` via SSH.
- `com.local.LimitHDDFan.plist` is a `launchd` job file to limit fan speed on startup using smcFanControl.
 
## Procedure
- Install fresh macOS 10.13 High Sierra from USB
- If GPU is faulty, move graphics kexts out of `/System/Library/Extensions` and rebuild kext cache (more details below). Probably need to disable SIP. Graphics and VNC screen sharing will be slow.
- Connect to network and install updates, enable SSH and VNC access for admin user
- Replace `/usr/libexec/dpd` with version from latest Mavericks installer (more details below)
- If hard drive is missing, install smcFanControl and add `launchd` job to limit fan speed (more details below)
- Success!

## Problems and solutions

### Booting macOS when the GPU is faulty
*Symptom*: 2011 27” iMac boots to grey screen after progress bar completes

*Cause*: faulty AMD Radeon card, common issue with machines of this era

*Workaround*: Move `ATIRadeonX3000.kext` and `ATI6000Controller.kext` (AMD… in later MacOS) from `/System/Library/Extensions` to e.g. `/System/Library/DisabledExtensions`. In later versions of macOS with a kext cache run `touch /System/Library/Extensions` to force a cache rebuild.
- Some sites suggest moving all ATI* or AMD* kexts but this doesn’t appear to be necessary
- Some sites suggest changing nvram `boot-args` to `agc=0` or `agc=-1` but this also doesn’t appear to do anything on an iMac (might be relevant to MBP graphics switching)

### Disabling SIP when GPU is faulty
*Problem*: Can’t move kexts while SIP is enabled, can’t disable SIP since Recovery Partition is inaccessible due to attempting to load above kexts

*Solution A*: Boot into single-user recovery mode by holding Option, then pressing Cmd-S immediately after selecting the recovery partition. Couldn’t get this working with the 10.13.6 recovery partition, so may need to use an older recovery partition (e.g. 10.11.6). Run `csrutil disable` from single-user-recovery mode. See also <https://apple.stackexchange.com/questions/332587/single-user-recovery-mode-on-high-sierra-10-13-6>

*Solution B*: Plug boot media into another Mac and move kexts from there (or maybe Target Disk Mode)

*Possible solution*: (not tried) disable SIP directly using the nvram command from single-user mode using an older pre-SIP macOS (e.g. 10.6.8): `nvram csr-active-config="w%00%00%00"`

### Getting Target Display Mode to work on macOS 10.13 High Sierra
*Problem*: Target Display Mode will not start. Console contains repeated messages every 10 seconds:
	`com.apple.xpc.launchd[1] (com.apple.dpd[182]): Service exited with abnormal code: 75`
 
*Cause*: Target Display Mode is toggled by `com.apple.dpd` (a single binary located at `/usr/libexec/dpd`) which listens for the Cmd-F2 sequence. dpd is repeatedly crashing which means TDM cannot be triggered.

*Solution*: The issue seems to just affect `dpd` binaries from 10.10 Yosemite onwards (tested on 10.10.5, 10.11.6, 10.13.6 `dpd`). Replacing the later `dpd` binary with the 10.9.5 Mavericks `dpd` (MD5: `6355f7378348de906ad1ace46c936c8c`) resolves the issue.

This can be obtained from the Mavericks installer PKG by directly mounting it with `hdiutil attach` and opening `/Packages/BSD.pkg` in e.g. Suspicious Package.

The Mavericks installer PKG is available from the Mac App Store using the link [here](https://support.apple.com/en-au/102662), or by using `mas-cli` with `mas install 675248567` as long as it has already been purchased, as per <https://forums.macrumors.com/threads/can-someone-give-me-a-mavericks-download-link.2279301/>.

### Triggering Target Display Mode without a physical keyboard or mouse
*Problem*: Want to remotely toggle Target Display Mode over SSH. 

*Solution*: Install [Karabiner Elements](https://karabiner-elements.pqrs.org), which includes a compiled and signed kext for [Karabiner-VirtualHIDDevice](https://github.com/pqrs-org/Karabiner-VirtualHIDDevice-archived). Compile `cmdf2` code in this repo (based on `Karabiner-VirtualHIDDevice/example/virtual_keyboard_example`) and run as sudo over SSH to trigger TDM.
Use `modifier::left_command` followed by `kHIDUsage_Csmr_DisplayBrightnessIncrement`.
`AppleVendorKeyboard_Brightness_Up` doesn’t work for some reason.

*Other useful references*:
- <https://github.com/pqrs-org/Karabiner-VirtualHIDDevice-archived/blob/master/src/include/karabiner_virtual_hid_device.hpp.tmpl>
<https://opensource.apple.com/source/IOHIDFamily/IOHIDFamily-700/IOHIDFamily/AppleHIDUsageTables.h>

*Possible solution*: Remap some keys using Karabiner as per
<https://apple.stackexchange.com/questions/170175/switch-to-target-display-mode-without-keyboard>

*Possible solution*: Remap keys with `hidutil` and `launchd` as per <https://rakhesh.com/mac/using-hidutil-to-map-macos-keyboard-keys/> and <https://hidutil-generator.netlify.app>

*Almost solution*: `osascript` over SSH works on macOS 10.6.8 (apparently up to 10.9.5 according to above link): `osascript -e 'tell application "System Events" to key code 144 using command down'`. Confirmed not working on 10.13.6 despite enabling Accessibility access for Terminal and Script Editor. <https://apple.stackexchange.com/questions/362787/automator-and-or-applescript-to-automatically-command-f2-to-enter-target-display>

*Almost solution*: [VirtualKVM](https://github.com/duanefields/VirtualKVM) seems like it might work for those that want TDM triggered upon attach/detach of Thunderbolt between their MacBook and iMac (along with simultaneous enable/disable of Bluetooth to facilitate transfer of mouse/keyboard control) and don’t mind a helper app running on both devices. However, couldn’t get it to work, possibly due to network issues blocking client-host connection or possibly due to daisy-chaining my iMac off an Apple Thunderbolt Display (MBP connects to TBD).

*Almost solution*: <https://github.com/BlueM/cliclick> - didn’t work over SSH, tried Cmd with F2, brightness, etc.

### Reducing fan speed when iMac hard drive is removed
*Problem*: iMac HDD fan runs loudly all the time with no hard drive

*Cause*: Apple hard drives contain a built-in temperature sensor which is now absent, so the SMC runs the fan at maximum to be safe.

*Solution*: Install [smcFanControl](https://www.eidac.de). Set the maximum HDD fan speed using the `smc` tool within using a `launchd` script (here as `com.local.LimitHDDFan.plist`).

*Command*: `/Applications/smcFanControl.app/Contents/Resources/smc -k F1Mx -w 1130`
`smc` tool info: <https://discussions.apple.com/thread/2554924>
launchd template: <https://superuser.com/questions/229773/run-command-on-startup-login-mac-os-x>
Note: use RunAtLoad instead of LaunchOnlyOnce

### Monitoring iMac temperature from command line
*Problem*: Want to track iMac temperatures over SSH to avoid overheating with the fan turned down

*Solution*: Install [HardwareMonitor](https://www.bresink.com/osx/HardwareMonitor.html) and use command-line tool: `/Applications/HardwareMonitor.app/Contents/MacOS/hwmonitor -c`

### Miscellaneous notes
- `dpd` has an associated binary `dpaudiothru` responsible for passing through audio which seems to work fine on newer macOS versions.
- `dmtest` reinstalls missing recovery partitions: `./dmtest ensureRecoveryPartition` <https://davidjb.com/blog/2016/12/creating-a-macos-recovery-partition-without-reinstalling-osx-or-re-running-your-installer/>
- To stop Screen Sharing locking the screen when disconnecting: `sudo defaults write /Library/Preferences/com.apple.RemoteManagement RestoreMachineState -bool NO`
- SilentKnight checks EFI is up-to-date and can install updates for e.g. Gatekeeper and XProtect <https://eclecticlight.co/lockrattler-systhist/>
    - Command-line version `silnite` requires Swift Runtime for Command Line Tools: <https://support.apple.com/kb/dl1998?locale=en_US>
- 2011 iMac graphics card upgrade megathread: <https://forums.macrumors.com/threads/2011-imac-graphics-card-upgrade.1596614/>
- Some macOS install packages combine the installer PKG file and the InstallESD DMG and can be mounted directly using `hdiutil attach`
- Older macOS (e.g. 10.6.8) don’t support ed25519 SSH keys

