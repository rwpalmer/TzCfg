#include "TzCfg.h"

/* Module: TzCfg101.ino
This firmware example demonstrates the most commonly used TzCfg methods.

The following discussion references functions as tzCfg.<functionName>() ... assuming TzCfg was instantiated as:
	TzCfg tzCfg;
 

	WARNING: 	TzCfg requires 128 bytes of EEPROM storage. In TzCfg parlance, this 128 byte block is called the tzBlock. 
					- Default location: 	The first 128 bytes of EEPROM storage.
					- Alternate location:	Use tzCfg.setEepromStartByte() to select a different location
											Valid locations range from 0 to (EEPROM.length() - 128);
					- Do NOT use TzCfg unless the required EEPROM storage is available.
            
    This code displays output on a serial console. The firmware has a 10 second countdown timer to give users
    time to connect their console. 
	
    A sample of the example's output is included in the comments, at the bottom of this file. 


					REMEMBER TO ENTER YOUR TIMEZONEDB API KEY INTO THE CODE ...
	
	*/

TzCfg tzCfg;

// ----------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------- setup()
void setup() {
    
    // Enable communication with the serial console, and give the user 10 seconds to connect.
    Serial.begin(9600);
    for (int i = 10; i > 0; i--) {  //  <-- countdown timer
        Serial.print("\rStarting in ");
        Serial.print(String(i));
        Serial.print(" Seconds ... ");
        delay(1000);
     }
    Serial.print("\r                                   \r"); // erase countdown timer


    //  Initialize TzCfg 
    tzCfg.begin();      // <-- Required, MUST be run before any other TzCfg method is run ...
    
    //  Set the timezonedb API key
    tzCfg.setApiKey_timezonedb("YOUR_TIMEZONEDB_API_KEY");		// <-- Required
	
	//	Select the location where TzCfg will store the TzBlock mentioned in the WARNING above.
	//	This is not normally required, but may be required by users who already use EEPROM.
	tzCfg.setEepromStartByte(512); // <-- Tells tzCfg to use EEPROM bytes 512 to 639

	//	Configure the device's local time based on the IP address it uses.
	tzCfg.setTimezoneByIP(); // <-- One of the "setTimzoneBy" commands is required here

	//  Call a local function to display timezone information
	displayInfo();

}
// ---------------------------------------------------------------------------------- loop()
void loop() {
    
    // Keep local time zone information stored in EEPROM current, and perform DST transitions
    tzCfg.maintainLocalTime();	// <-- Required
}


void displayInfo() {

	/*  OPTIONAL CALLS TO TzCfg and Particle  METHODS ----------------------------------
	    
	    The TzCfg and Particle functions used below can be used to display useful information. 
	    tzCfg.getLocalIP and tzCfg.getHTTPStatus() are specifically designed to aid in troubleshooting.
	    
	    Note: The IP address returned by TzCfg.getLocalIP is the "return address" on messages that your Particle 
	    device transmits to the internet. In most cases, this is NOT the IP address assigned to your device. 
	    More likely, it is the IP address assigned to internet side of the internet gateway that your device uses. 
    */
            Serial.println("\n\n\tTzCfg101 -------------------------------------------------------------------------");
        	Serial.printf("\t\t\t\t\t\tIP Address = %s\r\n", tzCfg.getLocalIP());

		//  display the status of the last HTTP dialog
            Serial.print("\n\ttzCfg.getHTTPStatus() returns:\t\t");
            Serial.println(tzCfg.getHttpStatus());
            
        //  TzCfg exposes the local time zone ID:
            Serial.print("\ttzCfg.getZone() returns:\t\t");
            Serial.println(tzCfg.getTimezone());

        //  TzCfg exposes the local time zone's abbreviation. (This varies with DST)
            Serial.print("\ttzCfg.getZoneAbbr() returns:\t\t");
            Serial.println(tzCfg.getTimezoneAbbr());
            
        //  Particle functions display Particle settings after TzCfg runs
            Serial.print("\tParticle Time.zone() returns:\t\t");
            Serial.println(Time.zone());
            
            Serial.println(Time.isDST()? "\tParticle isDst() returns:\t\ttrue":"\tParticle isDst() returns:\t\tfalse");

            Serial.print("\tParticle Time.getDSTOffset() returns:\t");
            Serial.println(Time.getDSTOffset());
            
        //  print local time
            
            Serial.print("\n\t\t\tLocal Time is:\t");
            Serial.println(Time.format(Time.now(), TIME_FORMAT_DEFAULT));

}

/* 
----------------------------------------  S A M P L E  -  O U T P U T  -----------------------------------------

        TzCfg101 -------------------------------------------------------------------------
                                                IP Address = 218.185.93.57

        tzCfg.getHTTPStatus() returns:          HTTP Processing Completed Normally
        tzCfg.getZone() returns:                America/Chicago
        tzCfg.getZoneAbbr() returns:            CDT
        Particle Time.zone() returns:           -6.00
        Particle isDst() returns:               true
        Particle Time.getDSTOffset() returns:   1.00

                        Local Time is:  Tue May  1 16:19:04 2018

----------------------------------------------------------------------------------------------------------------
                 
*/



