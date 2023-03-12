/*
 * GSMmodem.h
 *
 *  Created on: Jan 24, 2023
 *      Author: antoine
 */

#ifndef GSMMODEM_H_
#define GSMMODEM_H_

#define TINY_GSM_MODEM_SIM7600
#include <TinyGsmClient.h>
#include <ArduinoHttpClient.h>

class GSMmodem: public TinyGsm{
public:
    GSMmodem(Stream& st,Stream& debug);
    String httpGet(String url);
    String httpPost(String url,String data);
    virtual ~GSMmodem();

private:
    TinyGsmClient client;
    Stream *debug;
    const char * apn="gprs.swisscom.ch";

    String httpAction(int action, String url,String data);
    //HttpClient http;
};

#endif /* GSMMODEM_H_ */
