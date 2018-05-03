# TzCfg

## Automatically maintains the device's local time settings.

### So, what does "Automatically" mean?
The TzCfg library configures the device's local time settings based on one of the following three items
* The device's IP address, which it gets from the internet
* The device's GPS coordinates supplied by GPS hardware or software
* A user-specified "time zone ID" (aka "Olson Name") 

Time-zone changes can be triggered at any time, via local code, or via the web. 

Time zone information is saved in EEPROM, so the device can set proper time after a reboot, even if it has no network connectivity at that time.

Oh, and yes, tzCfg automatically performs DST transitions at their scheduled time. 


## How the Library Works ...

* IANA maintains the time zone database that communication companies and OS vendors use to manage local time world wide. 

* TzCfg obtains IANA time zone information via [timezonedb.com](http//timezonedb.com). TzCfg users must register for a timezonedb.com API-key to access the data. The key is free ( for up to 1 lookup per second ). They charge a fee for access above this level. [Registration link](http://timezonedb.com/register)

* To enable time zone configuration by IP, TzCfg needs to know the Particle device's IP address and the time zone ID associated with that address. This information is obtained from [ip-api.com](http://ip-api.com) which does not require an API key for non-commercial access up to 150 lookups per minute. Commercial use required preapproval ... see the site for more details.  

*It should be noted that TzCfg does not use Particle's WiFi.localIP() function to obtain the device's IP address because many/most devices are configured with non-routable addresses (like 192.168.xxx.xxx, or 10.xxx.xxx.xxx). Non-routable addresses can not be used for time zone lookups. IP-API returns the IP address designated as the "return address" on packets sent from your device. This is most likely the IP address assigned to the internet side of the IP gateway that your device uses to reach the internet. 

* Time zone data is stored in EEPROM to assure that the data is available whenever the system reboots, even if no network connection is available at that time. 

## TzCfg Documentation

TzCfg Documentation: 
[TzCfg QuickStartGuide](https://docs.google.com/document/d/e/2PACX-1vQL7RNLxRsPbv9EC9xgdZFwIcbJHS-eEm-ocAxOIScUA4sIZUem_HSJZbh2hFMzVZ7V2BmjUy5fAJlr/pub)


##	Firmware Examples
TzCfg is packaged with three firmware examples. All three are designed to be run with a serial console.
*	*TzCfg101.ino* demonstrates setting local time and displaying local time variables exposed by tzLib and the Particle Time class.
*	*TzCfg201.ino* extends TzCfg101 by performing a test DST transition and reprinting the local time variables.
*	*TzCfg301.ino* demonstrates how a device's time zone can be changed via the web.

Sample "serial console output" is included in the comments at the the bottom of each example's .ino file. tzLib301.ino comments includes HTML that can be used to trigger a time zone change on the device. https://github.com/rwpalmer/TzCfg/tree/master/examples


## Current State of the Project ...

TzCfg (0.0.1) will exit alpha testing soon.
*	Known defects:
	
Code is almost there ... documentation is being developed
			
Please report any issues, suggestions, and comments to the author and maintainer: rwpalmeribm@gmail.com
	   





