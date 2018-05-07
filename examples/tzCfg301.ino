#include "TzCfg.h"

/*  Module TzCfg301

    This firmware demonstrates how TzCfg users can change their device's time zone via the web. 
	
        - Output is displayed on a serial console
        
        - Input commands and arguments are collected via Particle Functions.

            - The setZoneByID function requires a time zone ID like: America/Chicago
              A list of time zine IDs is available on wikipedia.
              Link: https://en.wikipedia.org/wiki/List_of_tz_database_time_zones

	        - The setZoneByGPS function requires GPS coordinates like: 46.9729,-91.7726
	          Use more digits for more accuracy.

	        - The setZoneByIP function requires NO argument.

	        - The erasetzBlock function erases time zone information from your device's EEPROM
	          to enable test scenarios that require a previously unused device. 

	In the comments at the bottom of this file, you will find sample output generated from 
	this firmware.
*/

TzCfg tzCfg;
// ----------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------- setup()
void setup() {
	
	// Register the Particle Function that will allow time zone changes via the web and/or 
	// the Particle Console
    Particle.function("SetZonebyID", webChangeID);
    Particle.function("SetZOneByGPS", webChangeGPS);
    Particle.function("SetZoneByIP", webChangeIP);
    Particle.function("eraseTzBlock", eraseTzBlock);
    
    // Enable communication with the serial console, and give the user 10 seconds to connect.
    Serial.begin(9600);
    for (int i = 10; i > 0; i--) {  //  <-- Countdown
        Serial.print("\rStarting in ");
        Serial.print(String(i));
        Serial.print(" Seconds ... ");
        delay(1000);
     }
    Serial.print("\r                                   \r"); // clear display

    // Use TzCfg to set the devices local time
    tzCfg.begin();
    tzCfg.setApiKey_timezonedb((char*)"YOUR_TIMEZONEDB_API_KEY"); 
    tzCfg.setTimezoneByIP();
    
    // Display local time settings
    Serial.println("\n\n\ttzLib301 -------------------------------------------------------------------------");
    displayTzInfo();
    Serial.println("\n\n\t\t<<< WAITING FOR A TIME ZONE CHANGE ... FROM THE WEB >>>");

}


// ----------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------- loop()
void loop() {
    
    // Keep local time zone information stored in EEPROM current, and perform DST transitions
    tzCfg.maintainLocalTime();
}


//--------------------------------------------------------------------------------------------
//---------------  PARTICLE FUNCTIONS TO ENABLE WEB BASED TIME ZONE CHANGES  ------------------
//--------------------------------------------------------------------------------------------

int webChangeID(String id) {
    char zoneId[65];
    id.toCharArray(zoneId, sizeof(zoneId));
    tzCfg.setTimezoneByID(zoneId);
	Serial.println("\n\n\tTime Zone Changed by ID  -------------------------------------------------------");
    displayTzInfo();
    return 0;
}

int webChangeGPS(String position) {
    float lat = atof(position);
    char* comma = strchr(position,',');
    float lng = atof(comma + 1);
    if ((lat >= -90) && (lat <= 90) && (lng >= -180) && (lng <= 180)) {
        tzCfg.setTimezoneByGPS(lat,lng);
	    Serial.println("\n\n\tTime Zone Changed by GPS  ------------------------------------------------------");
        displayTzInfo();
        return 0;
    } else {
        return -1;
    }
}

int webChangeIP(String ip) {
    tzCfg.setTimezoneByIP(); 
	Serial.println("\n\n\tTime Zone Changed by IP  -------------------------------------------------------");
	Serial.printf("\t\t\t\t\t\tIP Address = %s\r\n", tzCfg.getLocalIP());
    displayTzInfo();
    return 0;
}

int eraseTzBlock(String ip) {
    tzCfg.eraseTzEeprom();
	Serial.println("\n\n\tErased Time Zone data in EEPROM  -----------------------------------------------");
    return 0;
}


void displayTzInfo() { 
    // Display data for new time zone to Serial Console
	Serial.printf("\n\ttzCfg.getHTTPStatus() returns:\t\t%s\r\n", tzCfg.getHttpStatus());
	Serial.printf("\ttzCfg.getTimezone() returns:\t\t%s\r\n", tzCfg.getTimezone());
	Serial.printf("\ttzCfg.getTimezoneAbbr() returns:\t%s\r\n", tzCfg.getTimezoneAbbr());
	Serial.printf("\tParticle Time.zone() returns:\t\t%.2f\r\n", Time.zone());
	Serial.println(Time.isDST()? "\tParticle isDst() returns:\t\ttrue":"\tParticle isDst() returns:\t\tfalse");
	Serial.printf("\tParticle Time.getDSTOffset() returns:\t%.2f\r\n", Time.getDSTOffset());
	Serial.printf("\n\t\t\t     Local Time is:\t%s\r\n", (const char*)Time.format(Time.now(), TIME_FORMAT_DEFAULT));
	Serial.printf("\t\t    Next DST Transition is:\t%s\r\n", (const char*)Time.format(tzCfg.getNextTransitionTime(), TIME_FORMAT_DEFAULT));
	Serial.printf("\t\t    Next EEPROM Refresh is:\t%s\r\n", (const char*)Time.format(tzCfg.getNextRefreshTime(), TIME_FORMAT_DEFAULT));
    
}


/*____________________________________________________________________________________________________________

                         SAMPLE Serial Console Output from this Firmware
  _____________________________________________________________________________________________________________
  
  Produced by booting a Photon where TzCfg301 called tzCfg.setTimezoneByIP() in setup()

        tzLib301 -------------------------------------------------------------------------

        tzCfg.getHTTPStatus() returns:          HTTP Processing Completed Normally
        tzCfg.getTimezone() returns:            America/Chicago
        tzCfg.getTimezoneAbbr() returns:        CDT
        Particle Time.zone() returns:           -6.00
        Particle isDst() returns:               true
        Particle Time.getDSTOffset() returns:   1.00

                             Local Time is:     Sat May  5 08:47:55 2018
                    Next DST Transition is:     Sun Nov  4 02:00:00 2018
                    Next EEPROM Refresh is:     Fri May 25 07:35:55 2018


                <<< WAITING FOR A TIME ZONE CHANGE ... FROM THE WEB >>>

  Produced by Particle function setZoneByGPS with the argument: 40.720200, -81.874237
 
        Time Zone Changed by GPS  ------------------------------------------------------

        tzCfg.getHTTPStatus() returns:          HTTP Processing Completed Normally
        tzCfg.getTimezone() returns:            America/New_York
        tzCfg.getTimezoneAbbr() returns:        EDT
        Particle Time.zone() returns:           -5.00
        Particle isDst() returns:               true
        Particle Time.getDSTOffset() returns:   1.00

                             Local Time is:     Sat May  5 09:52:17 2018
                    Next DST Transition is:     Sun Nov  4 02:00:01 2018
                    Next EEPROM Refresh is:     Fri May 25 08:40:17 2018

  Produced by Particle function setZoneByID with the argument: Europe/Amsterdam

        Time Zone Changed by ID  -------------------------------------------------------

        tzCfg.getHTTPStatus() returns:          HTTP Processing Completed Normally
        tzCfg.getTimezone() returns:            Europe/Amsterdam
        tzCfg.getTimezoneAbbr() returns:        CEST
        Particle Time.zone() returns:           1.00
        Particle isDst() returns:               true
        Particle Time.getDSTOffset() returns:   1.00

                             Local Time is:     Sat May  5 15:53:09 2018
                    Next DST Transition is:     Sun Oct 28 03:00:00 2018
                    Next EEPROM Refresh is:     Fri May 25 14:41:09 2018
  
  Produced by Particle function eraseTzBlock with no argument

        Erased Time Zone data in EEPROM  -----------------------------------------------

  Produced by Particle function setZoneByIP with no argument

        Time Zone Changed by IP  -------------------------------------------------------
                                                IP Address = 280.58.93.57

        tzCfg.getHTTPStatus() returns:          HTTP Processing Completed Normally
        tzCfg.getTimezone() returns:            America/Chicago
        tzCfg.getTimezoneAbbr() returns:        CDT
        Particle Time.zone() returns:           -6.00
        Particle isDst() returns:               true
        Particle Time.getDSTOffset() returns:   1.00

                             Local Time is:     Sat May  5 08:53:56 2018
                    Next DST Transition is:     Sun Nov  4 02:00:00 2018
                    Next EEPROM Refresh is:     Fri May 25 07:41:56 2018


  _____________________________________________________________________________________________________________

*/


  