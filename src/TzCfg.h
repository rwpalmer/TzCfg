#ifndef __TZCFG_H_
#define __TZCFG_H_
#include "application.h"
//#define LOGGING true      // <-- true for debugging, false (or commented out) For production

const time_t tzBlockRefreshInterval = 1723680;  // <-- Specifies the interval between refreshes. (~3 weeks)
const time_t tzBlockRetryInterval =  40000;     // <-- Specifies the interval between retries if a refresh fails (~ 11 hours)
const char TZ_SIGNATURE[10] = "#!#TZ001a";      // <-- Used to identify the TzBlock in EEPROM.
const uint8_t BY_ZONEID = 0, BY_POSITION = 1, BY_IP = 2; // <-- type of time zone lookup

// ------------------------------------------------------------------- tzBlock Class
// Defines the data that TzCfg stores in EEPROM

class TzBlock {
	private:
	    // variable declaration
    	    char signature[10];                 //  <-- Identifies the TzBlock in EEPROM
    	    char id[65];                        //  <-- Selected time zone ID 
    		float stdOffset;                    //  <-- Standard offset for the selected time zone
    		float curOffset;                    //  <-- Current offset for the selected time zone
    		char curAbbr[6];                    //  <-- Current Abbreviation used for the selected time zone       
    		time_t tranTime;                    //  <-- Date/time for the next DST transition
    		float tranOffset;                   //  <-- Post-transition offset for the selected timezone
    		char tranAbbr[6];                   //  <-- Post-transition abbreviation for the selected timezone 
    		byte futureUse[20];                 //  <-- Reserved for future use
	    // constructor --------------
    	    TzBlock(void) {
                strncpy(this->signature, TZ_SIGNATURE, sizeof(this->signature));
	            strcpy(this->id,"UTC");
	            this->stdOffset = 0;
	            this->curOffset = 0;
	            strcpy(this->curAbbr,"UTC");
	            this->tranTime = 0;
	            this->tranOffset = 0;
	            strcpy(this->tranAbbr,"");
	            std::fill_n(this->futureUse,sizeof(this->futureUse),0xFF);
            }
		// method declaration ---------
    		void log(char*); // <-- Displays the contents of a TzBlock on the Serial console (when LOGGING)
    	// operator definitions
            bool operator==(const TzBlock& b) { 
            if ((strcmp(this->signature, b.signature) == 0) 
             && (strcmp(this->id, b.id) == 0)
             && (this->stdOffset == b.stdOffset)
             && (this->curOffset == b.curOffset)
             && (strcmp(this->curAbbr, b.curAbbr) == 0)
             && (this->tranTime == b.tranTime)
             && (this->tranOffset == b.tranOffset)
             && (strcmp(this->tranAbbr, b.tranAbbr) == 0))  return true;
            else return false;
        }
        
        bool operator!=(const TzBlock& b) {
            if ((strcmp(this->signature, b.signature) == 0) 
             && (strcmp(this->id, b.id) == 0)
             && (this->stdOffset == b.stdOffset)
             && (this->curOffset == b.curOffset)
             && (strcmp(this->curAbbr, b.curAbbr) == 0)
             && (this->tranTime == b.tranTime)
             && (this->tranOffset == b.tranOffset)
             && (strcmp(this->tranAbbr, b.tranAbbr) == 0))  return false;
            else return true;
        }
        
        TzBlock operator=(const TzBlock &rt) {
            if (this != &rt) {
                strncpy(this->signature, rt.signature, sizeof(this->signature)); 
                strncpy(this->id, rt.id, sizeof(this->signature));
                this->stdOffset = rt.stdOffset;
                this->curOffset = rt.curOffset;
                strncpy(this->curAbbr, rt.curAbbr, sizeof(this->curAbbr));
                this->tranTime = rt.tranTime;
                this->tranOffset = rt.tranOffset;
                strncpy(this->tranAbbr, rt.tranAbbr, sizeof(this->tranAbbr));
                std::fill_n(this->futureUse,sizeof(this->futureUse),0xFF);
            }
            return *this;
        }
		
		friend class TzCfg;
		friend class Json;
};

/*  TzBlock will be instantiated as follows:
        tzEeprom  ... will be instantiated as TzCfg::tzEeprom, and represents the TzBlock stored in EEPROM
        tzWeb ...     will be instantiated in TzCfg::setLocalTime and represents the TzBlock that is built
                      from the information provided by an HTTP server.
        Other methods also create TzBlocks for temporary use. 
*/
// ------------------------------------------------------------------- TzCfg Class
// Defines TzCfg core data and references
class TzCfg {
    private:
        TzBlock tzEeprom;                           // <-- tzBlock object that stores time zone data in EEPROM
        bool tzEepromExists;                        // <-- Indicates if the tzBlock currently exists in EEPROM
        int eepromStartByte;                        // <-- The starting location of the tzBlock in EEPROM
        time_t eepromRefreshTime;                   // <-- Specifies when the next tzBlock refresh will take place
        bool particleTimeSet;                       // <-- set false by tzCfg.begin(), and true when settings have been updated.
        char tzdbApiKey[16];                        // <== Stores the API key for timezonedb
        char statusMsg[65];                         // <-- Records processing status messages
        float latitude;                             // <-- Contains the latitude for time zone lookups by position
        float longitude;                            // <-- Contains the longitude for time zone lookups by position
        char newZoneID[65];                         // <-- Contains the time zone name for time zone lookups by name
        char localIP[16];                           // <-- Contains the local IP address for time zone lookups by IP (format: nnn.nnn.nnn.nnn)
        int setLocalTime(uint8_t);             	    // <-- Sets the devices local time settings for a specified time zone
        void setEepromRefreshTime();                // <-- Calculates the time when tzCfg will attempt to refresh the TzBlock in EEPROM
	public:
        void begin();                               // <-- TzCfg constructor renamed for code placenent flexibility.
        void setApiKey_timezonedb(char*);           // <-- Sets the timezonedb API key (tzdbApiKey)
        void maintainLocalTime(void);               // <-- Maintains the devices local time settings
        char* getTimezone(void);                    // <-- Returns the current time zone ID
        char* getTimezoneAbbr(void);                // <-- Returns the current time zone Abbreviation
        int setTimezoneByID(char* id);              // <-- Changes the current time zone ID and device settings
        int setTimezoneByGPS(float,float);          // <-- Sets the timezone based on GPS coordinates
        int setTimezoneByIP(void);                  // <-- Queries for the IP address & timezoneID, then invokes setZoneByID
        void setEepromStartByte(int sb);            // <-- Sets the location of the tzBlock in EEPROM
        int setNextTransitionTime(time_t time);     // <-- Allows testers to schedule test DST transitions
		int setNextRefreshTime(time_t time);		// <-- Allows testers to schedule test EEPROM refreshes
        void transitionNow(void);                   // <-- For testing: Performs a a pending transition instantly 
		void eraseTzEeprom(void);                   // <-- Overwrites a TzBlock in EEPROM with '0xFF'
		char* getHttpStatus(void);                  // <-- Returns the HTTP Status Message
		char* getLocalIP(void);                     // <-- Returns the IP address used to set the time zone
        time_t getNextTransitionTime(void);         // <-- Returns the time when the next DST transition will take place
        time_t getNextRefreshTime(void);            // <-- Returns the time when the next EEPROM refresh will take place
		
    private:
        void updateDeviceSettings(void);            // <-- Updates the device's local time settings
        int searchForTzEeprom(void);                // <-- Searches EEPROM for the presence of a TzBlock
};


// ------------------------------------------------------------------- Http Class
// Handles communication with the HTTP server
class Http {
    private:
        TCPClient client;                   // <-- Stores the TCPClient object used to perform the HTTP POST transaction
        char buffer[768];                   // <-- Buffer where the HTTP server's response is stored
        bool error;                         // <-- Error flag used while processing HTTP
        int statusCode;                     // <-- HTTP status code returned
        unsigned int bufferIndex;           // <-- Index to the buffer defined above
        unsigned long lastReadMillis;       // <-- Time when the last character was received from the host
        unsigned long startMillis;          // <-- Time when the POST transaction was sent to the server
        char* cpJson;
        Http();
        int getJson(char* hostName, int hostPort, char* hostPath, char*& jsonStr, uint& jsonSize, char* errorMsg, int errMsgSize);  // <-- Performs the HTTP processing
        
        friend class TzCfg;
};

// Http is instantiated in TzCfg.cpp --- 


// ------------------------------------------------------------------- Json Class
// Provides methods used to encode and decode JSON
class Json {
    private:
        Json();
        void fix(char* json, int jsonSize);                     // <-- prepares JSON for parsing
        int get(float& n, char* json, char* name);              // <-- Gets a floating-point value for the associated JSON name
        int get(time_t& n, char* json, char* name);             // <-- Gets a time_t value for the associated JSON name
        int get(char* ca, int caSize, char* json, char* name);  // <-- Gets a character array for the associated JSON name

        friend class TzCfg;
};

#endif






















