#include "TzCfg.h"

/*      Library: tzCfg
        Module: TzCfg.cpp contains tzCfg's  core elements (data and methods). 
*/

/* ---------------------------------------------------------------------------- begin
    TzCfg's constructor ... MUST BE THE FIRST TzCfg command that is executed on the device
*/
void TzCfg::begin() {
    this->localIP[0]='\0';
    this->particleTimeSet = false;
    // locate the TzBlock in EEPROM ... if found, load it into memory
    this->eepromStartByte = searchForTzEeprom();
    if (this->eepromStartByte > -1) {
        EEPROM.get(this->eepromStartByte,this->tzEeprom);
        this->tzEepromExists = true;
    } else {
        this->tzEepromExists = false;
    }
    #ifdef LOGGING
        Serial.begin();
        Serial.println("-------------------------------------------------- TzCfg::begin()");
    	Serial.printf("tzCfg>\tParticle Settings: Time.zone() = %.2f, Time.isDST() = %s, Time.getDSTOffset() = %.2f\r\n", 
    	    Time.zone(), Time.isDST()? "true":"false", Time.getDSTOffset());
        Serial.printf("tzCfg>\tEepromStartByte = %d\r\n", this->eepromStartByte);
        tzEeprom.log((char*)"tzEeprom");
        Serial.printf("tzCfg>\tSizeof(tzEeprom) = %d\r\n", sizeof(this->tzEeprom));
    #endif

}
// -------------------------------------------------------------------- setApiKey_timezonedb()
// Loads the user's timezonedb API-key

void TzCfg::setApiKey_timezonedb(char* apikey) {
    strncpy(this->tzdbApiKey, apikey, sizeof(this->tzdbApiKey));
}

// ---------------------------------------------------------------------------- maintainLocalTime()
void TzCfg::maintainLocalTime() {
    
    // Perform a DST transition when the scheduled transition time arrives
    if ((this->tzEeprom.tranTime > 0) && !(this->tzEeprom.tranTime > Time.now())) {
        #ifdef LOGGING
            Serial.println("----- TzCfg::maintainLocalTime()");
            Serial.printf("Triggering the Transition @ %s\r\n", (const char *)Time.format(Time.now()));
            Serial.printf("tzEeprom.tranTime = %s\r\n", tzEeprom.tranTime);
        #endif
        transitionNow();
        this->tzEeprom.tranTime = 0; // precaution to prevent retriggering in an error condition
    }
    // Verify or Update the TzBlock in EEPROM when the scheduled refresh time arrives
    if (Time.now() >= this->eepromRefreshTime) {
        setLocalTime(BY_ZONEID);
    }
}
// ----------------------------------------------------------------------------- getLocalIP()
// Gets the IP address used to obtain time zone information
char* TzCfg::getLocalIP(void) {
    return (char*)this->localIP;
}

// ---------------------------------------------------------------------------- getTimezone()
// Gets the time zone ID
char* TzCfg::getTimezone(void) {
    return (char*)this->tzEeprom.id;
}

// ------------------------------------------------------------------------ getTimezoneAbbr()
// Gets the current time zone abbreviation which often changes with DST transitions
char* TzCfg::getTimezoneAbbr(void) {
    return (char*)this->tzEeprom.curAbbr;
}

// ------------------------------------------------------------------ getNextTransitionTime()
time_t TzCfg::getNextTransitionTime() {
    return this->tzEeprom.tranTime;
}

// --------------------------------------------------------------------- getNextRefreshTime()
time_t TzCfg::getNextRefreshTime() {
    return this->eepromRefreshTime;
}

// ------------------------------------------------------------------------ setTimezoneByID()
// Changes the time zone based on the time zone ID
int TzCfg::setTimezoneByID(char* id) {
    strncpy(this->newZoneID, id, sizeof(this->newZoneID));
	int ret = setLocalTime(BY_ZONEID);
	if (ret == EXIT_SUCCESS) return EXIT_SUCCESS;
	// assure Particle time is set to tzBlock ... even in an error condition
    if (( !this->particleTimeSet) && (this->tzEepromExists)) {
        updateDeviceSettings();
        setEepromRefreshTime();
    }
    return EXIT_FAILURE;
}

// ---------------------------------------------------------------------------- setTimezoneByGPS()
// Sets the time zone based on GPS coordinates
int TzCfg::setTimezoneByGPS(float lat,float lng) {
    this->latitude = lat;
    this->longitude = lng;
    this->newZoneID[0] = '\0';
    int ret = setLocalTime(BY_POSITION);
	if (ret == EXIT_SUCCESS) return EXIT_SUCCESS;
	// assure Particle time is set to tzBlock ... even in an error condition
    if (( !this->particleTimeSet) && (this->tzEepromExists)) {
        updateDeviceSettings();
        setEepromRefreshTime();
    }
    return EXIT_FAILURE;
}

// ---------------------------------------------------------------------------- setTimezoneByIP()
// Sets the time zone based on the device's IP address
int TzCfg::setTimezoneByIP(void) {
    Http http;
    Json json;
    bool error = false;
    char* jsonStr = NULL;
    uint jsonSize = 0;
    char hostName[] = "ip-api.com";
    char hostPath[] = "/json";
    char ipapiStatus[16] = "";
    char ipapiMsg[65] = "";
    // Perform the HTTP query and update the TzWeb
    this->statusMsg[0] = '\0';
    this->newZoneID[0] = '\0';
    int statusCode = http.getJson(hostName, 80, hostPath, jsonStr, jsonSize, statusMsg, sizeof(statusMsg));
    // note: jsonStr contains the JSON returned from the HTTP server.
    if ((statusCode == 200) && (jsonStr != NULL)) {
        json.fix(jsonStr, jsonSize);
        // handle ip-api JSON with {"status":"fail"} and where status is unknown
        if (json.get(ipapiStatus, sizeof(ipapiStatus), (char*) jsonStr, (char*) "status") == EXIT_SUCCESS) {
            if (strcmp(ipapiStatus, "fail") == 0) {
                error = true;
                if (json.get(ipapiMsg, sizeof(ipapiMsg), (char*) jsonStr, (char*) "message") == EXIT_SUCCESS) {
                    strncpy(this->statusMsg, "(ip-api) ", sizeof(this->statusMsg));
                    strncat(this->statusMsg, ipapiMsg, sizeof(this->statusMsg));
                } else { 
                    strncpy(this->statusMsg, "(E735) unable to parse ip-api <message>", sizeof(this->statusMsg));
                }
            }
        } else {
            strncpy(this->statusMsg, "(E733) unable to parse ip-api status", sizeof(this->statusMsg));
            error = true;
        }
        // parse ip-api JSON for IP address and time zone ID
        if ( !error) {
            if (json.get(this->newZoneID, sizeof(this->newZoneID), (char*) jsonStr, (char*) "timezone") == EXIT_SUCCESS) {
                if (json.get(this->localIP, sizeof(this->localIP),(char*) jsonStr, (char*) "query") == EXIT_SUCCESS) {
                    #ifdef LOGGING
                        Serial.printf("TzCfg>\tQuery to ip-api.com returns: IP Address = %s, Timezone ID = %s\r\n",this->localIP, this->newZoneID);
                    #endif
                    return setLocalTime(BY_ZONEID);
                } else {
                    strncpy(this->statusMsg, "(E742) unable to parse ip-api <ipaddress>", sizeof(this->statusMsg));
                    error = true;
                }
            } else {
            strncpy(this->statusMsg, "(E743) unable to parse ip-api <timezone>", sizeof(this->statusMsg));
            error = true;
            }
        }
    }

    // assure Particle time is set to tzBlock ... even in an error condition
    if (( !this->particleTimeSet) && (this->tzEepromExists)) {
        updateDeviceSettings();
        setEepromRefreshTime();
    }
    return EXIT_FAILURE;
}

// ---------------------------------------------------------------------------- setEepromStartByte()
// Defines where the TzBlock will be stored in EEPROM
void TzCfg::setEepromStartByte(int sb) {
    if (sb == this->eepromStartByte) {
        return;
    }
    if ((sb > 0) && !(sb > (int)(EEPROM.length() - sizeof(this->tzEeprom)))){
        #ifdef LOGGING
            Serial.println("\n\r----- setEepromStartByte()");
        #endif
        if (this->tzEepromExists) {
            #ifdef LOGGING
                Serial.printf("tzCfg>\tWriting TzBlock to EEPROM byte %d\r\n", sb);
            #endif
            eraseTzEeprom();
            EEPROM.put(sb,this->tzEeprom);
 			this->tzEepromExists = true;
       } else {
            #ifdef LOGGING
                Serial.print ("tzCfg>\teepromStartByte has been set to %d\r\n, sb");
            #endif
        }
        this->eepromStartByte = sb;
    } else {
        #ifdef LOGGING
            Serial.println("tzCfg>\tERROR: Attempt to set EepromStartByte rejected: byte# not in range");
        #endif
    }
    return;
}

// ---------------------------------------------------------------------------- transitionNow()
// Allows tzCfg users to instantly simulate a transition for testing purposes
void TzCfg::transitionNow(void) {
    if (this->tzEeprom.tranTime > 0) {
        this->tzEeprom.tranTime = 0;
        this->tzEeprom.curOffset = this->tzEeprom.tranOffset;
        this->tzEeprom.tranOffset = 0;
        strcpy(this->tzEeprom.curAbbr, this->tzEeprom.tranAbbr);
        this->tzEeprom.tranAbbr[0] = '\0';
        EEPROM.put(this->eepromStartByte, this->tzEeprom);
        updateDeviceSettings();
        #ifdef LOGGING
            Serial.println("----- TzCfg::transitionNow()");
            Serial.println(Time.format(Time.now()));
            Serial.printf("EepromStartByte = %d\r\n", this->eepromStartByte);
            this->tzEeprom.log((char*)"tzEeprom");
        #endif
    }
    return;
}

// ---------------------------------------------------------------------------- setNextTransitionTime()
// Allows tzCfg users to simulate a transition at a future time for testing purposes
// For example: "tzCfg.setNextTransitionTime(Time.now() + 30);", for 30 seconds from now
int TzCfg::setNextTransitionTime(time_t time) {
    this->tzEeprom.tranTime = time;
    #ifdef LOGGING
        Serial.println("----- TzCfg::setNextTransitionTime()");
        Serial.printf("NextTransitionTime = %s", (const char *)Time.format(this->tzEeprom.tranTime));
    #endif
    return 0;
}

// ---------------------------------------------------------------------------- setNextRefreshTime()
// Allows tzCfg users to reschedule the next EEPROM refresh for testing purposes
// For example: "tzCfg.setNextRefreshTime(Time.now() + 30);", for 30 seconds from now
int TzCfg::setNextRefreshTime(time_t time) {
    this->eepromRefreshTime = time;
    #ifdef LOGGING
        Serial.println("----- TzCfg::setNextRefreshTime()");
        Serial.printf("NextRefreshTime = %s", (const char *)Time.format(this->eepromRefreshTime)));
    #endif
    return 0;
}
 
// ---------------------------------------------------------------------------- eraseTzEeprom()
// Erases the TzBlock from EEPROM memory
// Allows tzCfg users to simulate how tzCfg will perform on a new device. 
void TzCfg::eraseTzEeprom(void) {
    TzBlock tzBlk;
    char signature[sizeof(TZ_SIGNATURE)];
    EEPROM.get(this->eepromStartByte, signature);
    if (strcmp(signature, TZ_SIGNATURE) == 0) {
        memset(&tzBlk, 0xFF, sizeof(tzBlk));
        EEPROM.put(this->eepromStartByte, tzBlk);
        #ifdef LOGGING
            Serial.printf("tzCfg>\tErased TzBlock @ EEPROM location %d\r\n ", eepromStartByte);
        #endif
        this->tzEepromExists = false;
    }
}

// ---------------------------------------------------------------------------- getHttpStatus()
char* TzCfg::getHttpStatus(void) {
    return (char*)this->statusMsg;
}



// ______________________________________________________________________________________________

//                    P R I V A T E    M E T H O D S    B E L O W


// ---------------------------------------------------------------------------- setLocalTime(char* id)
int TzCfg::setLocalTime(uint8_t lookupBy) {
   // Prepare to query server for timezone information ... 
    TzBlock tzWeb;
        #ifdef LOGGING
            Serial.println("\n\r-------------------------------------------------- TzCfg::setLocalTime()");
            Serial.printf("tzCfg>\teepromStartByte = %d\r\n",this->eepromStartByte);
            Serial.println(this->tzEepromExists? "tzCfg>\tzEepromExists = true":"tzCfg>\tzEepromExists = false");
            Serial.printf("tzCfg>\ttzEeprom.id = %s\r\n",this->tzEeprom.id);
            switch (lookupBy) {
                case BY_ZONEID:     
                    Serial.printf("tzCfg>\tLOOKUP BY ZONEID: %s\r\n", this->newZoneID);
                    break;
                case BY_POSITION:   
                    Serial.printf("tzCfg>\tLOOKUP BY POSITION --- Lat: %d, Lng: %d\r\n", this->latitude, this->longitude);
                    break;
                case BY_IP:
                    Serial.printf("tzCfg>\tLOOKUP BY IP ADDRESS: %s\r\n", this->localIP);
            }
        #endif
            
    // Query the HTTP Server
    bool queryComplete = false;
    bool queryError = false;
    int queryPass = 1;
    while (( !queryComplete) && ( !queryError) && (queryPass < 3)) {
        Http http;
        Json json;
        char hostName[] = "api.timezonedb.com";
        char hostPath[193] = "";
        char* jsonStr = NULL;
        strncpy(hostPath, "/v2/get-time-zone?key=", sizeof(hostPath));
        strncat(hostPath, this->tzdbApiKey, sizeof(hostPath));
        strncat(hostPath, "&format=json", sizeof(hostPath));
        switch (lookupBy) {
            case BY_ZONEID:
            case BY_IP:
                strncat(hostPath, "&by=zone&zone=", sizeof(hostPath));
                strncat(hostPath, this->newZoneID, sizeof(hostPath));
                break;
            case BY_POSITION:
                strncat(hostPath, "&by=position&lat=", sizeof(hostPath));
                strncat(hostPath, String(this->latitude), sizeof(hostPath));
                strncat(hostPath, "&lng=", sizeof(hostPath));
                strncat(hostPath, String(this->longitude), sizeof(hostPath));
                break;
        }
            
        if (queryPass == 2) {
            strncat(hostPath, "&time=", sizeof(hostPath));
            strncat(hostPath, String(tzWeb.tranTime), sizeof(hostPath));
        }
        // Perform the HTTP query and update the TzWeb
        this->statusMsg[0] = '\0';
        jsonStr = NULL;
        uint jsonSize = 0;
        int statusCode = http.getJson(hostName, 80, hostPath, jsonStr, jsonSize, this->statusMsg, sizeof(this->statusMsg));
        if ((statusCode == 200) && (jsonStr != NULL)) { 
            json.fix(jsonStr, jsonSize);
            char jsonStatus[7] = "";
            if (json.get(jsonStatus, sizeof(jsonStatus), (char*)jsonStr, (char*)"status") == EXIT_SUCCESS) {
                if (strncmp(jsonStatus, "OK", 2) == 0) {
                    float gmtOffset = 0;
                    if (json.get(gmtOffset, (char*)jsonStr, (char*)"gmtOffset") == EXIT_SUCCESS) {
                        gmtOffset = gmtOffset/3600;
                        char dst[2];
                        json.get(dst, sizeof(dst), (char*) jsonStr, (char*) "dst");
                        switch (queryPass) {
                            case 1:
                                tzWeb.curOffset = gmtOffset;
                                json.get(tzWeb.id, sizeof(tzWeb.id), (char*) jsonStr, (char*) "zoneName");
                                json.get(tzWeb.curAbbr, sizeof(tzWeb.curAbbr), (char*) jsonStr, (char*) "abbreviation");
                                json.get(tzWeb.tranAbbr, sizeof(tzWeb.tranAbbr), (char*) jsonStr, (char*) "nextAbbreviation");
                                json.get(tzWeb.tranTime, (char*)jsonStr, (char*)"dstEnd");
                                if (atoi(dst) == 0) { 
                                    tzWeb.stdOffset = gmtOffset;
                                }
                                if (tzWeb.tranTime > 0) {
                                    tzWeb.tranTime++;
                                } else  {
                                    queryComplete = true;
                                }
                                break;
                            case 2:
                                tzWeb.tranOffset = gmtOffset;
                                if (atoi(dst) == 0) tzWeb.stdOffset = gmtOffset;
                                queryComplete = true;
                                break;
                        }
                    } else {
                        strncpy(this->statusMsg, "(E764) unable to parse timezonedb <gmtOffset>", sizeof(this->statusMsg));
                        queryError = true;
                    }
                } else {
                    char message[65] = "";
                    if (json.get(message, sizeof(message), (char*)jsonStr, (char*)"message") == EXIT_SUCCESS) {
                        strncpy(this->statusMsg, "(timezonedb) ", sizeof(this->statusMsg));
                        strncat(this->statusMsg, message, sizeof(this->statusMsg));
                    } else {
                        strncpy(this->statusMsg, "(E757) unable to parse timezonedb <message>", sizeof(this->statusMsg));
                    }
                }
            } else {
                strncpy(this->statusMsg, "(E751) unable to parse timezonedb <status>", sizeof(this->statusMsg));
                queryError = true;
            }
        } else { 
            queryError = true;
        }
        queryPass++;
    } // end of while()
    
    if (queryComplete && ( !queryError) && (tzEeprom != tzWeb)) {
        //if the 'tzWeb' TzBlock has new data, update EEPROM
		if (this->eepromStartByte == -1) { // <-- Occurs if no TzBlock was found in EEPROM, AND no startByte has been designated.
			this->eepromStartByte = 0;     // <-- Set the startByte to the default location (zero).
		}
        EEPROM.put(this->eepromStartByte,tzWeb);
        this->tzEepromExists = true;
        EEPROM.get(this->eepromStartByte,this->tzEeprom);
        #ifdef LOGGING
            Serial.println("tzCfg>\tTzEeprom Updated ...");
        #endif
    } else {
        #ifdef LOGGING
            Serial.println("tzCfg>\tTime zone settings unchanged");
        #endif
    }
   
    // update the devices local time settings & schedule the next EEPROM refresh
    updateDeviceSettings();
    if (queryError) {
        this->eepromRefreshTime = Time.now() + (tzBlockRetryInterval);
        return EXIT_FAILURE;
    } else {
        setEepromRefreshTime();
        return EXIT_SUCCESS;
    }
}

// ------------------------------------------------------------------------ setEepromRefreshTime()
// Schedule the next EEPROM refresh
void TzCfg::setEepromRefreshTime() {
    int refresh_multiplier = 1;
    
    // If the tz has no DST scheduled transitions, we will trebble the refresh Interval
    if ((this->tzEeprom.tranTime == 0) && (this->tzEeprom.stdOffset == this->tzEeprom.curOffset)) {
        refresh_multiplier = 3;
    }
    this->eepromRefreshTime = Time.now() + (tzBlockRefreshInterval * refresh_multiplier);
    return;
}

// ---------------------------------------------------------------------------- searchForTzBlock()
// Locates the TzBlock stored in EEPROM. 
//      Returns -1 when  TzBlock is NOT found
//      Returns the location of TzBlocks first byte when a TzBlock is found
int TzCfg::searchForTzEeprom(void) {
    // The usable EEPROM location ranges from zero to (EEPROM storage size - tzBlockSize).
    int endUsableRange = (EEPROM.length() - sizeof(this->tzEeprom));
    int rangeIndex = 0;
    // A small buffer is used to minimize memory usage.
    char buffer[128];
    char* cp;
    int startingLocation = -1 ;  // return value -1 if no tzBlock is found

    while ( !(rangeIndex > endUsableRange)) {
        EEPROM.get(rangeIndex,buffer);
        cp = strstr(buffer, TZ_SIGNATURE);
        if (cp == NULL) {
            rangeIndex = rangeIndex + (sizeof(buffer) - sizeof(TZ_SIGNATURE));
        } else {
            startingLocation = rangeIndex + (cp - (char*)buffer);
            rangeIndex = startingLocation +1;
            // Note: We continue to search the entire EEPROM ... so logging exposes 
            //       multiple tzBlocks ... should they exist. 
            #ifdef LOGGING
                Serial.printf("TzBlock found @ Location %d\r\n", startingLocation);
            #endif
        }
    }
    return startingLocation;
}

// ---------------------------------------------------------------------------- updateDeviceSettings()
// Updates the devices local time settings based on the TzBlock stored in EEPROM  (object name =  tzEeprom)
void TzCfg::updateDeviceSettings(void) {
    
    // called by tzSetup()
    // configures local time settings for Particle devices
    Time.zone(this->tzEeprom.stdOffset);
    int dstOffset = (this->tzEeprom.curOffset - this->tzEeprom.stdOffset);
    Time.setDSTOffset(dstOffset);
    if (dstOffset == 0) {
        Time.endDST();
    } else {
        Time.beginDST();
    }
    this->particleTimeSet = true;
    
    #ifdef LOGGING
        tzEeprom.log((char*)"tzEeprom");
        Serial.println("\r\ntzCfg>\tLocal Time Settings Complete ...");
        Serial.printf("tzCfg>\tTime.zone() ---> %.2f\r\n", Time.zone());
        Serial.printf("tzCfg>\tTime.getDSTOffset() --> %.2f\r\n", Time.getDSTOffset());
        Serial.printf("tzCfg>\tTime.isDST() ---> %s\r\n", Time.isDST()? "true" : "false");
        Serial.printf("tzCfg>\ttzCfg.getTimezone() ---> %s\r\n", getTimezone());
        Serial.printf("tzCfg>\ttzCfg.getTimezoneAbbr() ---> %s\r\n", getTimezoneAbbr());
        Serial.printf("tzCfg>\tTime.format(Time.now(), TIME_FORMAT_DEFAULT) ---> %s\r\n", (const char *)Time.format(Time.now(), TIME_FORMAT_DEFAULT));
    #endif
    
    return;
}

// -------------------------------------------------------------------- log()
// Displays the contents of a TzBlock when LOGGING
void TzBlock::log(char* name) {
    # ifdef LOGGING
		Serial.printf("TzBlock>\t-------- %s\r\n", name);
		Serial.printf("TzBlock>\tid         = %s\r\n", this->id);
		Serial.printf("TzBlock>\tstdOffset  = %.2f\r\n", this->stdOffset);
		Serial.printf("TzBlock>\tcurOffset  = %.2f\r\n", this->curOffset);
		Serial.printf("TzBlock>\tcurAbbr    = %s\r\n", this->curAbbr);
		Serial.printf("TzBlock>\ttranTime   = %d\r\n", this->tranTime);
		Serial.printf("TzBlock>\ttranOffset = %.2f\r\n", this->tranOffset);
		Serial.printf("TzBlock>\ttranAbbr   = %s\r\n", this->tranAbbr);
	#endif
}






























