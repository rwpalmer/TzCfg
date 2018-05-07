#include "application.h"
#include "TzCfg.h"
#include "spark_wiring_string.h"
#include "spark_wiring_tcpclient.h"
#include "spark_wiring_usbserial.h"

/*      Library: TzCfg
        Module: Http.cpp contains the method that this library needs to perform
        an HTTP POST query to obtain time zone data from an HTTP server. 
        
        The Http class is defined and instantiated in TzCfg.
        
        Methods are called using the http object. Format: http.method();

        Note:   Class methods are tailored specifically to meet TzCfg
                requirements as efficiently as possible. Since this project
                encompasses both the library and the HTTP server code, a 
                bare-bones approach can be used. 
*/
// ----------------------------------------------------------- Class Constructor
Http::Http() {
    this->error = false;
    this->statusCode = 0;
    memset(&this->buffer[0], 0, sizeof(this->buffer));  // clears the response buffer
    this->bufferIndex = 0;
};

// ----------------------------------------------------------- getJson()
int Http::getJson(char* hostName, int hostPort, char* hostPath, char*& jsonStr, uint& jsonSize, char* statusMsg, int statusMsgSize) {
    client.connect(hostName, hostPort);
    if (client.connected()) {
        client.print("POST ");
        client.print(hostPath);
        client.print(" HTTP/1.0\r\n");
    	client.println("Connection: close");
    	client.print("HOST: ");
    	client.println(hostName);
    	client.println("Content-Length: 0");
    	client.println("Accept: application/json");
        client.println();
        client.flush();
    } else {
        this->error = true;
        strncpy(statusMsg, "(E621) Unable to connect to ", statusMsgSize);
        strncat(statusMsg, hostName, statusMsgSize);
        client.stop();
    }

    #ifdef LOGGING
        Serial.println("\r\nHttp>\t---------- Start Http.getJson() ----------");
        Serial.printf("Http>\thostName = %s\r\n", hostName);
        Serial.printf("Http>\thostPort = %d\r\n", hostPort);
        Serial.printf("Http>\thostPath = %s\r\n", hostPath);
        if (jsonStr != NULL) Serial.printf("Http>\tjsonStr  = %s\r\n", jsonStr);
        Serial.printf("Http>\tjsonSize = %d\r\n", jsonSize);
    	if (client.connected()) {
    		Serial.println("Http>\tClient is Connected to the HTTP server ...");
    		Serial.println("Http>\tHTTP POST Request has been Sent ...");
    	} else {
    		Serial.println(statusMsg);
        }
    #endif
    
    if (this->error) return -1;
       
    // ------------------------------------------------------------------------------ Buffer the  Response
    this->bufferIndex = 0;
    this->startMillis = millis();
    this->lastReadMillis = millis();

    
    while (client.connected() && !this->error) {
        while (client.available() && !this->error) {
                char c = client.read();
           this->lastReadMillis = millis();
            if (this->bufferIndex == sizeof(this->buffer)-1) {
                // buffer overflow
                this->error = true;
                strncpy(statusMsg, "(E653) Response Buffer Overflow", statusMsgSize);
            } else {
                this->buffer[this->bufferIndex] = c;
                this->bufferIndex++;
            }
        }
        buffer[bufferIndex] = '\0';
        if ((millis() - lastReadMillis) > 5000) {
            // timeout after 5 seconds of inactivity
            this->error = true;
            strncpy(statusMsg, "(E668) Timeout waiting for server to respond", statusMsgSize);
 
        } else if ( !this->error) {
            delay(200);
        }
    }
    client.stop();

    #ifdef LOGGING
        Serial.printf("\r\nHttp>\tResponse Time (ms): %d\r\n", (millis() - startMillis));
        Serial.println("---------------------------------------------------- Start of HTTP Response");
        Serial.println(this->buffer);
        Serial.println("---------------------------------------------------- End of HTTP Response");
        if(this->error) {
            Serial.println(statusMsg);
        } else {
            Serial.println("\r\nHttp>\tHTTP client processing terminated normally ...");
        }
    #endif
	if (error) return -1;
	
    // parse the HTTP status code
    char* cp;
    char httpMsg[65] = "";
    // parse HTTP status code
    cp = strchr(this->buffer, ' ');
    if (cp != NULL) {
        uint ui;
        this->statusCode = atoi(cp+1);
        // parse HTTP status message
        cp = cp + 5;
        for(ui = 0; ui < sizeof(httpMsg); ui++, cp++) {
            if (*cp == '\n') {
                httpMsg[ui] = '\0'; 
                break;
            }
            httpMsg[ui] = *cp;
        }
        httpMsg[sizeof(httpMsg) -1] = '\0';
    } else {
        this->statusCode = 0;
    }
    if (this->statusCode == 200) {
        
        // Find where the JSON starts
        cp = strstr(this->buffer, "{\"");
	    if (cp == NULL) {
	        this->error = true;
	        strncpy(statusMsg, "(E677) JSON not found in buffer", statusMsgSize);
	    } else {
	        //find where the JSON ends
	        this->cpJson = cp;
	        char* lcp = strrchr(this->buffer, '}');
	        if ((lcp == NULL) || (cp > lcp) || ((lcp - this->buffer) <= (int)(sizeof(this->buffer - 2)))) {
	        this->error = true;
	        strncpy(statusMsg, "(E679) JSON truncated in buffer", statusMsgSize);
	        }
	        lcp++;
	        memset(lcp,'\0',1);
	        // return a pointer to the JSON string within the HTTP buffer, along with the string's size
	        jsonStr = cp;
	        jsonSize = (lcp - cp);
	    }
    }
    #ifdef LOGGING
        Serial.printf("Http>\tHTTP Status Code: %s\r\n", String(this->statusCode));
        Serial.printf("Http>\tHTTP Message: %s\r\n", httpMsg); 
        Serial.print("Http>\tJSON: ");
        if (jsonStr == NULL) Serial.println("NULL"); else Serial.println(String(jsonStr));
        if (this->error) Serial.printf("Http>\tStatusMsg: %s\r\n", statusMsg);
     #endif

	if (this->error) {
	    return -1;
	} else if (this->statusCode == 200) {
        strncpy(statusMsg, "HTTP Processing Completed Normally", statusMsgSize);
	} else { 
        strncpy(statusMsg, "(H", statusMsgSize);
        strncat(statusMsg, String(this->statusCode), statusMsgSize);
        strncat(statusMsg, ") ", statusMsgSize);
        strncat(statusMsg, httpMsg, statusMsgSize);
    }
    #ifdef LOGGING
        Serial.printf("Http>\tStatusMsg: %s\r\n", statusMsg);
    #endif
    return this->statusCode;
}

