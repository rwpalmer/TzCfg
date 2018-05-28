# TzCfg

## A library to maintain local time settings on Particle IOT devices

### Library Functionality ...

TzCfg methods automatically configure a device's local time settings based upon one of the following data elements:
* *IP address* (automatically detected)
* *GPS coordinates* (which can be obtained from a GPS device, a cellular API, or other source)
* *time zone ID* (aka, Olson Name) 

In normal usage, TzCfg methods configure local time settings in firmware setup(), but these methods can also be called from firmware loop() to update local time settings for mobile devices, and from a Particle function to change local time settings via network commands.

Time zone information is saved in EEPROM, and the library will configure local time to the last known time zone settings if the device reboots when no network connectivity is available.

Since IANA updates its time zone database on a regular basis, TzCfg performs a periodic "refresh" operation that will update EEPROM if relevant data (like the time of the next DST transition) has changed. 

TzCfg automatically performs DST transitions when they are scheduled. This is a local operation. 

TzCfg methods expose a number of data elements for public use. These include: 
* Time zone ID
* Time zone abbreviation
* Next DST transition time
* HTTP Status message (useful for debugging when an update fails)

Methods are also available to support various firmware test scenarios ... like validating that logging and display logic perform properly when a DST transition takes place. 

TzCfg strives to keep resource usage to a minimum.  


## How the Library Works ...

* IANA maintains the time zone database that communication companies and OS vendors use to manage local time world wide. 

* TzCfg obtains IANA time zone information via [timezonedb.com](https://timezonedb.com). TzCfg users must register for a timezonedb.com API-key to access the data. The key is free ( for up to 1 lookup per second ). They charge a fee for access above this level. [Registration link](http://timezonedb.com/register)

* To enable time zone configuration by IP, TzCfg needs to know the Particle device's IP address and the time zone ID associated with that address. This information is obtained from [ip-api.com](http://ip-api.com) which does not require an API key for non-commercial access up to 150 lookups per minute. Commercial use requires preapproval ... see the site for more details.  

*It should be noted that TzCfg does not use Particle's WiFi.localIP() function to obtain the device's IP address because that would not work.  Most IOT devices are configured with non-routable addresses (like 192.168.xxx.xxx, or 10.xxx.xxx.xxx). Non-routable addresses like these CAN NOT be used for time zone lookups. ipapi.com returns data based upon the IP address designated as the "return address" on packets sent from your device. In most cases, this would be the IP address assigned to the Internet side of the IP gateway that your device uses to communicate with the web. 

The following code sample configures the device's local time settings based on the device's IP address. It will also perform DST transitions at their scheduled time. This is all that is needed for most implementations. 
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





