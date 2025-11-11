#include "HttpReporter.h"
#include <WiFi.h>
#include <HTTPClient.h>

static String SERVER_URL = "";

void initHttpReporter(const char *serverUrl)
{
    SERVER_URL = serverUrl;
}

void sendLogToServer(const String &log)
{
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("⚠️ WiFi 未连接，无法上传日志");
        return;
    }

    if (SERVER_URL.isEmpty())
    {
        Serial.println("⚠️ 未设置服务器地址");
        return;
    }

    HTTPClient http;
    http.begin(SERVER_URL);
    http.addHeader("Content-Type", "application/json");

    String payload = "{\"log\":\"" + log + "\"}";
    int httpResponseCode = http.POST(payload);

    Serial.print("📡 上传日志结果: ");
    Serial.println(httpResponseCode);

    http.end();
}
