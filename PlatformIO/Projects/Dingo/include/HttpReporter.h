#ifndef HTTP_REPORTER_H
#define HTTP_REPORTER_H

#include <Arduino.h>

void initHttpReporter(const char *serverUrl);
void sendLogToServer(const String &log);

#endif