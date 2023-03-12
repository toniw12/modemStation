/*
 * GSMthis->cpp
 *
 *  Created on: Jan 24, 2023
 *      Author: antoine
 */

#include <GSMmodem.h>
#include "Arduino.h"

#define MODEM_EN PD4

//char apn[]="gprs.swisscom.ch";

enum actionNum{
    HTTP_GET,
    HTTP_POST
};

GSMmodem::GSMmodem(Stream &st, Stream &debug) :
        TinyGsm(st) {
    client.init(this);
    this->debug = &debug;

}

String GSMmodem::httpPost(String url,String data) {
    return httpAction(HTTP_POST,url,data);
}

String GSMmodem::httpGet(String url) {
    return httpAction(HTTP_GET,url,"");
}
String GSMmodem::httpAction(int action, String url,String data) {
    debug->println("------Http GET-----");
    HttpClient http(client, "cinter.wiedmer-electro.ch", 80);

    if (this->isNetworkConnected()) {
        debug->println("Network connected");
    } else {
        debug->println("ERROR Network not connected");
        return "";
    }

    if (this->isGprsConnected()) {
        debug->println("Gprs connected");
    }

    debug->print(F("Connecting to "));
    debug->print(apn);
    if (!this->gprsConnect(apn)) {
        debug->println(" fail");
        return "";
    }
    debug->println(" ok");
    int err;
    switch(action){
    case HTTP_GET:
        debug->print(F("Performing HTTP GET request... "));
        err = http.get(url);
        if (err != 0) {
            debug->println(F("failed to connect"));
            return "";
        }
        break;
    case HTTP_POST:
        debug->print(F("Performing HTTP POST request... "));
        err = http.post(url,"application/json",data);
        if (err != 0) {
            debug->println(F("failed to connect"));
            return "";
        }
        break;
    }


    int status = http.responseStatusCode();
    debug->print(F("Response status code: "));
    debug->println(status);
    if (status<0) {
        return "";
    }

    debug->println(F("Response Headers:"));
    while (http.headerAvailable()) {
        String headerName = http.readHeaderName();
        String headerValue = http.readHeaderValue();
        debug->println("    " + headerName + " : " + headerValue);
    }

    int length = http.contentLength();
    if (length >= 0) {
        debug->print(F("Content length is: "));
        debug->println(length);
    }
    if (http.isResponseChunked()) {
        debug->println(F("The response is chunked"));
    }

    String body = http.responseBody();
    debug->println(F("Response:"));
    debug->println(body);

    debug->print(F("Body length is: "));
    debug->println(body.length());

    // Shutdown

    http.stop();
    debug->println(F("Server disconnected"));
    return body;
}

GSMmodem::~GSMmodem() {

}

