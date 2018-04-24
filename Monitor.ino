//Temperature reading service
#include "temperatureService.h"

//OLED headers
#include <Arduino.h>
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

//PUBLISH DATA headers
#include <WiFi.h>
#include <PubSubClient.h>

/*
 *
 * OLED DEFINITIONS
 *
 */

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/15, /* data=*/4, /* reset=*/
		16);

void u8g2_prepare(void) {
	u8g2.setFont(u8g2_font_6x10_tf);
	u8g2.setFontRefHeightExtendedText();
	u8g2.setDrawColor(1);
	u8g2.setFontPosTop();
	u8g2.setFontDirection(0);
}

void draw(void) {
	u8g2_prepare();

	//WIFI status
	int ss = 0; //wifi signal strength, 1 to 4

	//Display connected AP SSID (or connection status)
	if (WiFi.status() != WL_CONNECTED) {
		String msg;
		switch (WiFi.status()) {
		case WL_IDLE_STATUS:
			msg = "IDLE";
			break;
		case WL_NO_SSID_AVAIL:
			msg = "NO SSID";
			break;
		case WL_SCAN_COMPLETED:
			msg = "SCAN FINISHED";
			break;
		case WL_CONNECT_FAILED:
			msg = "CONN FAILED";
			break;
		case WL_CONNECTION_LOST:
			msg = "CONN LOST";
			break;
		case WL_DISCONNECTED:
			msg = "DISCONNECTED";
			break;
		default:
			msg = "CONNECTING";
		}
		u8g2.drawStr(15, 0, msg.c_str());
		ss = 0;
	} else {
		u8g2.drawStr(13, 0, WiFi.SSID().substring(0, 15).c_str());
		ss = (WiFi.RSSI() > -30 ?
				4 : (WiFi.RSSI() > -64 ? 3 : (WiFi.RSSI() > -70 ? 2 : 1)));
	}

	//Display WIFI strength symbols (or number, debug)
	const int x_wifi = 1;
	const int y_wifi = 7;

	for (int i = 0; i < 4; i++) {
		int x = x_wifi + 9 - 3 * i;
		int y = y_wifi;
		int h = 7 - 2 * i;
		u8g2.drawBox(x, y - (ss >= (4 - i) ? h - 1 : 0), 2,
				(ss >= (4 - i) ? h : 1));
	}

	//Temperature
	if (!temp_started) {
		//Not initialized
	} else {

		for (int i = 0; i < numberOfDevices; i++) {

			char addrStr[5];
			char buff[100];
			sprintf(addrStr, "%02X%02X", temp_addr[i][6], temp_addr[i][7]);

			sprintf(buff, "T[%s]:%.2f C", addrStr, temp[i]);
			u8g2.drawStr(0, 12 + 8 * i, buff);
		}
	}

	//Status messages

}

/*
 *
 * PUBLISH DATA definitions
 *
 */

const char* ssid = "GVT-D528";
const char* password = "1965003487";
//const char* ssid     = "AndroidAP";
//const char* password = "zbzm1164";
const char* mqtt_server = "mqtt.thingspeak.com";

const char* mqttUserName = "TSArduinoMQTTDemo";
const char* mqttPass = "38CL4LAGMYQSN1IN";
const char* writeAPIKey = "B6QDSL36MYYO71VW";
long channelID = 456493;

WiFiClient espClient;
PubSubClient client(espClient);
long lastConnectionTime = 0;
#define POSTING_INTERVAL 300L * 1000L
char msg[50];
int value = 0;

void setup_wifi() {

	delay(10);
	// We start by connecting to a WiFi network
	Serial.println();
	Serial.print("Connecting to ");
	Serial.println(ssid);

	WiFi.begin(ssid, password);

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}

	randomSeed(micros());

	Serial.println("");
	Serial.println("WiFi connected");
	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());
}

void mqttpublish(float t1, float t2) {

	// Create data string to send to ThingSpeak
	String data = String(
			"field1=" + String(t1, DEC) + "&field2=" + String(t2, DEC));
	int length = data.length();
	char msgBuffer[length];
	data.toCharArray(msgBuffer, length + 1);
	Serial.println(msgBuffer);

	// Create a topic string and publish data to ThingSpeak channel feed.
	String topicString = "channels/" + String(channelID) + "/publish/"
			+ String(writeAPIKey);
	length = topicString.length();
	char topicBuffer[length];
	topicString.toCharArray(topicBuffer, length + 1);

	client.publish(topicBuffer, msgBuffer);

	lastConnectionTime = millis();
}

void callback(char* topic, byte* payload, unsigned int length) {
	Serial.print("Message arrived [");
	Serial.print(topic);
	Serial.print("] ");
	for (int i = 0; i < length; i++) {
		Serial.print((char) payload[i]);
	}
	Serial.println();

	// Switch on the LED if an 1 was received as first character
	if ((char) payload[0] == '1') {
		digitalWrite(BUILTIN_LED, LOW); // Turn the LED on (Note that LOW is the voltage level
		// but actually the LED is on; this is because
		// it is acive low on the ESP-01)
	} else {
		digitalWrite(BUILTIN_LED, HIGH); // Turn the LED off by making the voltage HIGH
	}

}

void reconnect() {
	// Loop until we're reconnected
	while (!client.connected()) {
		Serial.print("Attempting MQTT connection...");
		// Create a random client ID
		String clientId = "ESP32Client-";
		clientId += String(random(0xffff), HEX);
		// Attempt to connect
		if (client.connect(clientId.c_str(), mqttUserName, mqttPass)) {
			Serial.println("connected");

		} else {
			Serial.print("failed, rc=");
			Serial.print(client.state());
			Serial.println(" try again in 2 seconds");
			// Wait 2 seconds before retrying
			delay(2000);
		}
	}
}

/*
 * task loops definitions
 */


void draw_loop(void *params) {

	/* OLED SETUP */
	u8g2.begin();


	for (;;) {
		/* OLED PICTURE LOOP */
		u8g2.clearBuffer();
		draw();
		u8g2.sendBuffer();

		delay(1000);

	}
}

void publish_loop(void *params) {

	/* PUBLISH DATA SETUP */
	pinMode(BUILTIN_LED, OUTPUT); // Initialize the BUILTIN_LED pin as an output
	setup_wifi();
	client.setServer(mqtt_server, 1883);
	client.setCallback(callback);

	for (;;) {

		/* PUBLISH DATA LOOP */
		if (!client.connected()) {
			reconnect();
		}
		client.loop();

		if (millis() - lastConnectionTime > POSTING_INTERVAL) {
			if (temp_started)
				mqttpublish(temp[0], temp[1]);
		}

		delay(500);

	}

}

struct context {
	int a;
} * pctx;

void setup(void) {

	//start serial communication
	Serial.begin(115200);

	//Setup interprocess communication
	SemaphoreHandle_t xSemaphoreBinTemperatureReady = xSemaphoreCreateBinary();
	SemaphoreHandle_t xSemaphoreMutTempAddress = xSemaphoreCreateMutex();


	//Start tasks
	xTaskCreatePinnedToCore(&draw_loop, // pvTaskCode – Pointer to the task function.
			"DRAW LOOP",   // pcName – Debugging name of the task
			2048,             // usStackDepth – Size of the stack for the task.
			NULL, // pvParameters – Parameters for the task instance. This may be NULL.
			4,                // uxPriority – Priority of the task instance.
			NULL, // pxCreatedTask – Reference to the newly created task instance. This may be
			1 // xCoreID - Which core to run on. Choices are: 0 (PRO CPU) or 1 (APP CPU) to "tskNO_AFFINITY".
			);

	/* READ TEMPERATURE LOOP TASK SETUP */
	xTaskCreatePinnedToCore(&temp_loop, // pvTaskCode – Pointer to the task function.
			"TEMPERATURE LOOP",   // pcName – Debugging name of the task
			2048,             // usStackDepth – Size of the stack for the task.
			NULL, // pvParameters – Parameters for the task instance. This may be NULL.
			6,                // uxPriority – Priority of the task instance.
			NULL, // pxCreatedTask – Reference to the newly created task instance. This may be
			0 // xCoreID - Which core to run on. Choices are: 0 (PRO CPU) or 1 (APP CPU) to "tskNO_AFFINITY".
			);

	/* PUBLISH DATA LOOP TASK SETUP */
	xTaskCreatePinnedToCore(&publish_loop, // pvTaskCode – Pointer to the task function.
			"PUBLISH LOOP",   // pcName – Debugging name of the task
			2048,             // usStackDepth – Size of the stack for the task.
			NULL, // pvParameters – Parameters for the task instance. This may be NULL.
			5,                // uxPriority – Priority of the task instance.
			NULL, // pxCreatedTask – Reference to the newly created task instance. This may be
			1 // xCoreID - Which core to run on. Choices are: 0 (PRO CPU) or 1 (APP CPU) to "tskNO_AFFINITY".
			);

}

void loop(void) {

	delay(10000);

}
