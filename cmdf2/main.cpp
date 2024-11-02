#include "karabiner_virtual_hid_device_methods.hpp"
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/hid/IOHIDDevice.h>
#include <IOKit/hid/IOHIDElement.h>
#include <IOKit/hid/IOHIDManager.h>
#include <IOKit/hid/IOHIDQueue.h>
#include <IOKit/hid/IOHIDValue.h>
#include <IOKit/hidsystem/IOHIDShared.h>
#include <IOKit/hidsystem/ev_keymap.h>
#include <cmath>
#include <iostream>
#include <thread>

int main(int argc, const char* argv[]) {
  if (getuid() != 0) {
    std::cerr << "dispatch_keyboard_event_example requires root privilege." << std::endl;
  }

  //std::cout << pqrs::karabiner_virtual_hid_device::get_kernel_extension_name() << std::endl;

  kern_return_t kr;
  io_connect_t connect = IO_OBJECT_NULL;
  auto service = IOServiceGetMatchingService(kIOMasterPortDefault, IOServiceNameMatching(pqrs::karabiner_virtual_hid_device::get_virtual_hid_root_name()));
  if (!service) {
    std::cerr << "IOServiceGetMatchingService error" << std::endl;
    goto finish;
  }

  kr = IOServiceOpen(service, mach_task_self(), kIOHIDServerConnectType, &connect);
  if (kr != KERN_SUCCESS) {
    std::cerr << "IOServiceOpen error" << std::endl;
    goto finish;
  }

  {
    pqrs::karabiner_virtual_hid_device::properties::keyboard_initialization properties;
    kr = pqrs::karabiner_virtual_hid_device_methods::initialize_virtual_hid_keyboard(connect, properties);
    if (kr != KERN_SUCCESS) {
      std::cerr << "initialize_virtual_hid_keyboard error" << std::endl;
    }

    while (true) {
      //std::cout << "Checking virtual_hid_keyboard is ready..." << std::endl;

      bool ready;
      kr = pqrs::karabiner_virtual_hid_device_methods::is_virtual_hid_keyboard_ready(connect, ready);
      if (kr != KERN_SUCCESS) {
        std::cerr << "is_virtual_hid_keyboard_ready error: " << kr << std::endl;
      } else {
        if (ready) {
          //std::cout << "virtual_hid_keyboard is ready." << std::endl;
          break;
        }
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }

  {
    pqrs::karabiner_virtual_hid_device::properties::keyboard_initialization properties;
    //properties.country_code = 33;
    kr = pqrs::karabiner_virtual_hid_device_methods::initialize_virtual_hid_keyboard(connect, properties);
    if (kr != KERN_SUCCESS) {
      std::cerr << "initialize_virtual_hid_keyboard error" << std::endl;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }

  // left_command key down
  {
    pqrs::karabiner_virtual_hid_device::hid_report::keyboard_input report;
    report.modifiers.insert(pqrs::karabiner_virtual_hid_device::hid_report::modifier::left_command);

    kr = pqrs::karabiner_virtual_hid_device_methods::post_keyboard_input_report(connect, report);
    if (kr != KERN_SUCCESS) {
      std::cerr << "post_keyboard_input_report error" << std::endl;
    }
  }

  // brightness_up (f2) key down
  {
    // Apple brightness key doesn't work for enabling TDM
    //pqrs::karabiner_virtual_hid_device::hid_report::apple_vendor_keyboard_input report;
    //report.keys.insert(0x0020); // kHIDUsage_AppleVendorKeyboard_Brightness_Up

    // F2 key doesn't work for enabling TDM
    //pqrs::karabiner_virtual_hid_device::hid_report::keyboard_input report;
    //report.keys.insert(kHIDUsage_KeyboardF2);

    // Consumer display brightness increment does...?
    pqrs::karabiner_virtual_hid_device::hid_report::consumer_input report;
    report.keys.insert(kHIDUsage_Csmr_DisplayBrightnessIncrement);

    kr = pqrs::karabiner_virtual_hid_device_methods::post_keyboard_input_report(connect, report);
    if (kr != KERN_SUCCESS) {
      std::cerr << "post_keyboard_input_report error" << std::endl;
    }
  }

  // keyboard reset
  kr = pqrs::karabiner_virtual_hid_device_methods::reset_virtual_hid_keyboard(connect);
  if (kr != KERN_SUCCESS) {
    std::cerr << "reset_virtual_hid_keyboard error" << std::endl;
  }

finish:
  if (connect) {
    IOServiceClose(connect);
  }
  if (service) {
    IOObjectRelease(service);
  }

  return 0;
}
