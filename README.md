# TzCfg

## A library to automatically maintain local time settings on Particle IOT devices

### So, what does "Automatically" mean?
The TzCfg library configures the device's local time settings based on one of the following three items
* The device's *IP address*, which it gets from the Internet ... that's automatic.
* The device's *GPS coordinates* supplied by the GPS hardware or software of your choice ... that's automatic.
* A user-specified "time zone ID" (aka "Olson Name")... okay, not so automatic ... but two out of three's not bad.  

Time-zone changes can be triggered at any time, via software, or via the web. 

Time zone information is saved in EEPROM, so the device can set proper time after a reboot, even if it has no network connectivity at that time.

Oh, and yes, tzCfg automatically performs DST transitions at their scheduled time.


## How the Library Works ...

* IANA maintains the time zone database that communication companies and OS vendors use to manage local time world wide. 

* TzCfg obtains IANA time zone information via [timezonedb.com](https://timezonedb.com). TzCfg users must register for a timezonedb.com API-key to access the data. The key is free ( for up to 1 lookup per second ). They charge a fee for access above this level. [Registration link](http://timezonedb.com/register)

* To enable time zone configuration by IP, TzCfg needs to know the Particle device's IP address and the time zone ID associated with that address. This information is obtained from [ip-api.com](http://ip-api.com) which does not require an API key for non-commercial access up to 150 lookups per minute. Commercial use requires preapproval ... see the site for more details.  

*It should be noted that TzCfg does not use Particle's WiFi.localIP() function to obtain the device's IP address because that would not work.  Most IOT devices are configured with non-routable addresses (like 192.168.xxx.xxx, or 10.xxx.xxx.xxx). Non-routable addresses like these CAN NOT be used for time zone lookups. ipapi.com returns the IP address designated as the "return address" on packets sent from your device. This is most likely the IP address assigned to the Internet side of the IP gateway that your device uses to communicate with the web. 

The following example configures the device's local time settings based on the device's IP address. It will also perform DST transitions at their scheduled time. 
```cpp
#include <TzCfg.h>

TzCfg tzCfg;

void Setup() {
    tzCfg.begin();
    tzCfg.setApiKey_timezonedb((char*)"YOUR_TIMEZONEDB_API_KEY");
    tzCfg.setTimezoneByIP();
}

void Loop() {
    tzCfg.maintainLocalTime();
}
```

One additional command may be required if the device's EEPROM currently stores other data. See the QuickStart Guide for details. 

## TzCfg Documentation:

TzCfg Documentation:  
* [TzCfg QuickStartGuide](https://docs.google.com/document/d/e/2PACX-1vQL7RNLxRsPbv9EC9xgdZFwIcbJHS-eEm-ocAxOIScUA4sIZUem_HSJZbh2hFMzVZ7V2BmjUy5fAJlr/pub)
* [TzCfg Command Reference](https://docs.google.com/document/d/e/2PACX-1vSAg2uOsghutDC64-izE44u9ZXh8S8i--FXHapayXSZ-EPrm93bIpW2d_fWUszqxKYYxMqM0jmWe8mE/pub)

##	Firmware Examples
TzCfg is packaged with three firmware examples. All three are designed to be run with a serial console.
*	*TzCfg101.ino* demonstrates setting local time and displaying local time variables exposed by tzLib and the Particle Time class.
*	*TzCfg201.ino* demonstrates how TzCfg can perform DST transitions on command ... to enable testing of software that logs or displays data with time related information. 
*	*TzCfg301.ino* demonstrates how a device's time zone can be changed via the web. Particle functions are used to allow users to change timezones by entering a timezone ID, by entering GPS coordinates, and/or by simply clicking "setZonebyIP".  

Sample "serial console output" is included in the comments at the the bottom of each example's .ino file.

## Current State of the Project ...

* TzCfg (0.0.1) Beta 1
* Known defects: 0
* Second draft of documentation is complete ... see links above.
* Code has been posted to GitHub 
				
Please report any issues, suggestions, or other comments to the author and maintainer: rwpalmeribm@gmail.com.

Many thanks to Wagner Sartori (https://github.com/trunet) for his suggestion to use timezonedb.com as a source for timezone data. Without that suggestion, this library would not exist. 





