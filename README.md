# Stenomod IBUS

I wanted to use my original Stenomod 15 as a game controller. So I wrote firmware to make it happen. The Adafruit Metro Mini on this can't present as an HID, and there's no u2 chip to flash with HoodLoader2 in order to gain that capability (the Metro Mini uses a CP2014 USB-to-serial converter instead), so I had it use the IBUS protocol (a digital serial protocol that is primarily used by FlySky RC for control and telemetry) to send keypress data to the computer. From there, it is possible to use [vJoySerialFeeder](https://github.com/Cleric-K/vJoySerialFeeder) to read that data (as bitmapped button channels) and feed it to [vJoy](https://github.com/shauleiz/vJoy) or any other virtual joystick vJoySerialFeeder supports, allowing one to use the Stenomod as a game controller. It's probably possible to use even more software to map those controller inputs to keyboard and mouse or something else, but I haven't tried it.

The [stenomod.json](stenomod.json) file is a vJoySerialFeeder configuration that will set up bitmapped button channels for all 24 separate keys (the number row keys and the asterisk keys are each wired together as one big button, so they won't show up separately). Some applications may have trouble dealing with controllers using too high of a button (Skullgirls, for instance, won't bind any buttons over #20). If this happens, just remap the appropriate bits to a lower button. One button can only be mapped to one key. Assigning multiple keys to one button will cause only one to function.
  
The code is largely based on the Arduino Keypad's MultiKey example. All the TX Bolt parts are taken from
[StenoMod-C](https://github.com/SmackleFunky/StenoMod-C) by SmackleFunky. I fixed a bug that causes S2 to send 0xE0, which isn't a valid TX Bolt byte and locks up Plover. I'd send a pull request, but I'm still trying to figure out how that one works.

And, of course, thank you to [Charley Shattuck](https://stenomod.blogspot.com/) [(Github)](https://github.com/CharleyShattuck) for the Stenomod.  None of this would be possible without you.

Everything in this repository is LGPL v2.1 licensed.
