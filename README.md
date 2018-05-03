# TzCfg

## Automatically maintains the device's local time settings.

### So, what does "Automatically" mean?
The TzCfg library can configure the local device's time zone settings based on"
* The device's IP address
* The device's GPS coordinates
* A user-specified "time zone ID" (aka "Olson Name") 

Time-zone changes can be made at any time, via the web, or GPS input.

Time zone information is saved in EEPROM, so the device can set proper time after a reboot, even if it has no network connectivity.

Oh, and yes, tzCfg automatically performs DST transitions.  


## How the Library Works ...

* IANA maintains the time zone database that communication companies and OS vendors use to manage local time world wide. 

* TzCfg obtains IANA time zone information via "timezonedb.com". TzCfg users must obtain a timezonedb.com API-key to access the data. Access is free ( for up to 1 lookup per second ). They charge a fee for access above this level. 

* To enable time zone configuration by IP, TzCfg needs to know the Particle device's IP address and the time zone ID associated with that address. This information is obtained from ip-api.com". It should be noted that TzCfg does not use Particle's WiFi.localIP() function to obtain the device's IP address because many/most devices are configured with non-routable addresses (like 182.168.xxx.xxx, or 10.xxx.xxx.xxx). These can not be used for time zone lookups. 

* Time zone data is stored in EEPROM to assue that the data is available whenever the system reboots, even if no network connection is available at that time. 

## Sample Usage
#### Most implementations require the addition of four lines of code. This sample includes a fifth to make it clear when the fifth line must be included.

```cpp		
	setup() {
	    tzCfg.begin();                          		// <- 1. Prepare tzLib to run

	    tzCfg.setEepromStartByte(0);            		// <- 2. Tell tzLib where to store data in EEPROM		  
		  
	    tzCfg.setApiKey_timezonedb((char*)"<apikey>");	// <- 3. Set the default time zone

	    tzCfg.setTimezoneByIP();                        // <- 4. One of three configuration options  
	}
		   
	loop() {
	    tzLib.maintainLocaltime();       // <- 5. Performs DST transitions & keeps time zone data current.
	}
```

*__I M P O R T A N T__*  
* 	The command "TzCfg.setEepromStartByte(0);" reflects the TzCfg default setting, and can be omitted in most implementations.
* 	If the default location is not available for TzCfg to use, this command must be modified and left in place. 
*	For example, "TzCfg.setEepromStartByte(512);" would tell TzCfg to use EEPROM bytes 512-639. 
*	Please see the tzLib-QuickStart guide (link below) for more details. 


// TODO: complete the following information |||||||||||||||||||||||||||||||||||||||||||||||||||||||
// TODO: upload documentation and fix the following links.
####[TzCfg QuickStartGuide](https://docs.google.com/document/d/e/2PACX-1vQL7RNLxRsPbv9EC9xgdZFwIcbJHS-eEm-ocAxOIScUA4sIZUem_HSJZbh2hFMzVZ7V2BmjUy5fAJlr/pub)


##	Library Documentation
TzCfg documentation includes an Overview, a QuickStart Guide, and a Reference Guide. 
https://github.com/rwpalmer/tzCfg/tree/master/doc


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
	   





