#include "TzCfg.h"

/*  Module: TzCfg201.ino

    If you are developing an app that displays dates or logs data, you may wish to test to see
    how DST transitions impact your code and its output. This example shows how that can be done.

    This example displays to a serial console. The firmware has a 10 second countdown timer to 
    give users time to connect their console. 
	
	A sample of the example's output is included in the comments, at the bottom of this file. 
	
    --------------------------------------------------------------------------------------------
    --------------------- T R A N S I T I O N    T E S T    M E T H O D S ----------------------
    --------------------------------------------------------------------------------------------
    
    For those who wish to perform transition testing, TzCfg provides two methods that will alter
    the current time zone's next transition time:
      
      1. tzCfg.setNextTransitionTime(time_t) sets the transition time to time_t.
	  2. tzCfg.transitionNow() is short hand for tzCfg.setNextTransitonTime(Time.now())
      
    As it does in the production environment, tzCfg.maintainLocalTime() will perform the 
    transition when Time.now() is not less than the transition time.

    It should be noted that these test methods do nothing unless the current time zone observes
    DST and there is a pending DST transition. When a transition is triggered, the pending
    transition is cleared until the next time tzCfg.setLocalTime() is run. This normally occurs
    on reboots and scheduled refreshes, but tzCfg.setLocalTime() can be called (at will) in a
    test program if a reset is desired. 
    
    To demonstrate the commands mentioned above, this example includes four Particle functions:
 
        1. "TransistNow" performs a DST transition immediately ... no argument is required.

        2. "TransistIn10" performs a DST transition in 10 seconds ... no argument is required.

        3. "TransitionAt" performs a DST transition at the epoch-time specified in the argument.
            Sites like: https://www.epochconverter.com generate an epoch-time based on standard
            date/time parameters. When this Particle function is used, this code will display the
			transition date/time in human readable format on the serial console.

        4. "Reset" resets local time by calling tzCfg.setLocalTime() ... no argument is required.

	________________________________________________________________________________________________
	
			PLEASE REMEMBER TO INSERT YOUR TIMEZONEDB API KEY IN THE CODE BELOW ...
	________________________________________________________________________________________________
*/

TzCfg tzCfg;
static time_t tTime = 0;
static time_t rTime = 0;
char timezoneID[] = "America/Chicago";
const int TBD = 0, STATIC = 1, USES_DST = 2;
int zoneType = TBD;

// -----------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------- setup()
void setup() {
    
	// Particle functions that allow transitions to be triggered or reset via the Particle Console/Web
	Particle.function("TransistNow", TransistNow);
	Particle.function("TransistIn10", TransistIn10);
	Particle.function("TransistAt", TransistAt);
	Particle.function("RefreshAt", RefreshAt);
	Particle.function("Reset", Reset);


    // Enable communication with the serial console, and give the user 10 seconds to connect.
    Serial.begin(9600);
    for (int i = 10; i > 0; i--) {  //  <-- countdown timer
        Serial.print("\rStarting in ");
        Serial.print(String(i));
        Serial.print(" Seconds ... ");
        delay(1000);
     }
    Serial.print("\r                                   \r"); // <-- erase countdown timer

    //  Initialize TzCfg 
    tzCfg.begin();      // <-- Required, Must be run before any other TzCfg method is run ...
    
    //  Set the timezonedb API key
    tzCfg.setApiKey_timezonedb("YOUR_TIMEZONEDB_API_KEY");
	
	// Configure the device's local time settings by ID
    tzCfg.setTimezoneByID(timezoneID);
    if (tzCfg.getNextTransitionTime() == 0) {
        zoneType = STATIC;
    } else { 
        zoneType = USES_DST;
    }

    Serial.println("\n\n\tTzCfg201 -------------------------------------------------------------------------");
    displayInfo();
}

// -----------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------- loop()
void loop() {
    
    // Keep local time zone information stored in EEPROM current, and perform DST transitions
    tzCfg.maintainLocalTime();
    
    if ((tTime > 0) && (Time.now() >= (tTime))) { 
        tTime = 0;
        Serial.println("\n\n\t\t\t\t<<< DST Transition Complete >>>\n");
        displayInfo();
    }
    if ((rTime > 0) && (Time.now() >= (rTime))) { 
        rTime = 0;
        Serial.println("\n\n\t\t\t\t<<< EEPROM Refresh Complete >>>\n");
        displayInfo();
    }

}

void displayInfo() {
    Serial.printf("\n\ttzCfg.getHTTPStatus() returns:\t\t%s\r\n", tzCfg.getHttpStatus());
    Serial.printf("\ttzCfg.getTimezone() returns:\t\t%s\r\n", tzCfg.getTimezone());
    Serial.printf("\ttzCfg.getTimezoneAbbr() returns:\t%s\r\n", tzCfg.getTimezoneAbbr());
    Serial.printf("\tParticle Time.zone() returns:\t\t%.2f\r\n", Time.zone());
    Serial.println(Time.isDST()? "\tParticle isDst() returns:\t\ttrue":"\tParticle isDst() returns:\t\tfalse");
    Serial.printf("\tParticle Time.getDSTOffset() returns:\t%.2f\r\n", Time.getDSTOffset());
	Serial.printf("\n\t\t\t     Local Time is:\t%s\r\n", (const char*)Time.format(Time.now(), TIME_FORMAT_DEFAULT));
	if ((tzCfg.getNextTransitionTime() > 0) && (zoneType == USES_DST)){
	    Serial.printf("\t\t    Next DST Transition is:\t%s\r\n", (const char*)Time.format(tzCfg.getNextTransitionTime(), TIME_FORMAT_DEFAULT));
	} else {
	    Serial.printf("\t\t    Next DST Transition Time will be updated when EEPROM is refreshed\r\n");
	}
	Serial.printf("\t\t    Next EEPROM Refresh is:\t%s\r\n", (const char*)Time.format(tzCfg.getNextRefreshTime(), TIME_FORMAT_DEFAULT));
}


// -----------------------------------------------------------------------------------------------
// PARTICLE FUNCTIONS ----------------------------------------------------------------------------


int TransistNow(String s) { 
	Serial.println("\n\n\t\t\t\t<<< Transitioning NOW >>>");
	tzCfg.transitionNow();
	tTime=Time.now();
	return 0;
}

int TransistIn10(String s) { 
    tTime = Time.now() + 10;
	tzCfg.setNextTransitionTime(tTime);
	Serial.println("\n\n\t\t\t<<< DST Transition in 10 seconds >>>");
	Serial.printf("\t\t    Next DST Transition is:\t%s\r\n", (const char*)Time.format(tzCfg.getNextTransitionTime(), TIME_FORMAT_DEFAULT));
    rTime = tTime + 10;
    tzCfg.setNextRefreshTime(rTime);
    Serial.println("\n\n\t\t\t<<< EEPROM Refresh Rescheduled >>>");
    Serial.printf("\t\t    Next EEPROM Refresh is:\t%s\r\n", (const char*)Time.format(tzCfg.getNextRefreshTime(), TIME_FORMAT_DEFAULT));
	return 0;
}

int TransistAt(String s) { // transition at a specified EPOCH time
    time_t time = atol(s);
	if ((time < Time.now()) || (time > 2524607999)) return -1;
	tTime = time;
	tzCfg.setNextTransitionTime(tTime);
	Serial.println("\n\n\t\t\t<<< Transitioning Rescheduled >>>");
	Serial.printf("\t\t    Next DST Transition is:\t%s\r\n", (const char*)Time.format(tzCfg.getNextTransitionTime(), TIME_FORMAT_DEFAULT));
	return 0;
}

int RefreshAt(String s) { // refresh at a specified EPOCH time
	time_t time = atol(s);
	if ((time < Time.now()) || (time > 2524607999)) return -1;
	rTime = time;
	tzCfg.setNextRefreshTime(rTime);
	Serial.println("\n\n\t\t\t<<< Next Refresh Rescheduled >>>");
	Serial.printf("\t\t    Next EEPROM Refresh is:\t%s\r\n", (const char*)Time.format(tzCfg.getNextRefreshTime(), TIME_FORMAT_DEFAULT));
	return 0;
}

int Reset(String s){ // reset local time
	Serial.println("\n\n\t\t\t\t<<< Resetting Local Time >>>\n");
    tzCfg.setTimezoneByID(timezoneID);
    displayInfo();
    return 0;
}

/*
----------------------------------------  S A M P L E  -  O U T P U T  -----------------------------------------

The following output was generated in two steps:
    1.  Booting the photon to TzCfg201 firmware invoked setTimezoneByID() method. This program then displayed
        the first block of data ... up to the horizontal line of asterisks **********.
    
    2.  The Particle Function "TransistIn10" was then called with no argument. That scheduled a transition to
        occur in 10 seconds, and an EEPROM refresh to occur in 20 seconds. The firmware's loop section then
        displayed the same block of data after each event.
        
        *   You will note that the transition changed the local clock ... as it is designed to do.

        *   The Mext EEPROM Refresh Time is stored in UTC, and the transition changed the UTC offset, so 
            it displays differently after the offset.
            
        *   The Next DST Transition Time is set to zero during the transition process, and it is reset when
            the EEPROM refresh takes place. This is by design.
            *   If every computer in the timezone attempted to contact timezonedb when a DST transition  took
                place, it would be akin to a "denial of service" attack ... so TzCfg performs the transition
                without any network interaction. EEPROM refreshes are a random event designed to validate 
                or to update the device's local time-zone settings.
        
        *   You will also note that the EEPROM refresh rest the time as it was before the transition. That
            would not happen in production ... because the server and the local time would have agreed when
            the refresh was executed.


----------------------------------------------------------------------------------------------------------------

        TzCfg201 -------------------------------------------------------------------------

        tzCfg.getHTTPStatus() returns:          HTTP Processing Completed Normally
        tzCfg.getTimezone() returns:            America/Chicago
        tzCfg.getTimezoneAbbr() returns:        CDT
        Particle Time.zone() returns:           -6.00
        Particle isDst() returns:               true
        Particle Time.getDSTOffset() returns:   1.00

                             Local Time is:     Sun May  6 14:36:06 2018
                    Next DST Transition is:     Sun Nov  4 02:00:00 2018
                    Next EEPROM Refresh is:     Sat May 26 13:24:06 2018

****************************************************************************************************************

                        <<< DST Transition in 10 seconds >>>
                    Next DST Transition is:     Sun May  6 14:36:26 2018


                        <<< EEPROM Refresh Rescheduled >>>
                    Next EEPROM Refresh is:     Sun May  6 14:36:36 2018


                                <<< DST Transition Complete >>>


        tzCfg.getHTTPStatus() returns:          HTTP Processing Completed Normally
        tzCfg.getTimezone() returns:            America/Chicago
        tzCfg.getTimezoneAbbr() returns:        CST
        Particle Time.zone() returns:           -6.00
        Particle isDst() returns:               false
        Particle Time.getDSTOffset() returns:   0.00

                             Local Time is:     Sun May  6 13:36:26 2018
                    Next DST Transition Time will be updated when EEPROM is refreshed
                    Next EEPROM Refresh is:     Sun May  6 13:36:36 2018


                                <<< EEPROM Refresh Complete >>>


        tzCfg.getHTTPStatus() returns:          HTTP Processing Completed Normally
        tzCfg.getTimezone() returns:            America/Chicago
        tzCfg.getTimezoneAbbr() returns:        CDT
        Particle Time.zone() returns:           -6.00
        Particle isDst() returns:               true
        Particle Time.getDSTOffset() returns:   1.00

                             Local Time is:     Sun May  6 14:36:37 2018
                    Next DST Transition is:     Sun Nov  4 02:00:00 2018
                    Next EEPROM Refresh is:     Sat May 26 13:24:37 2018


*/