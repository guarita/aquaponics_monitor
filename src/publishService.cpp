#include "publishService.h"
#include "parameters.h"

/*
 *
 * PUBLISH DATA definitions
 *
 */

const char* mqtt_server = "mqtt.thingspeak.com";
const char* mqttUserName = "TSArduinoMQTTDemo";
const char* mqttPass = "38CL4LAGMYQSN1IN";
const char* writeAPIKey = "B6QDSL36MYYO71VW";
long channelID = 456493;

WiFiClient espClient;
PubSubClient client(espClient);

#define POSTING_INTERVAL 300L * 1000L

char msg[50];
int value = 0;

void mqttpublish(float t1, float t2, float t3) {

	// Create data string to send to ThingSpeak
	String data = String(
			"field1=" + String(t1, DEC) + "&field2=" + String(t2, DEC) + "&field3=" + String(t3, DEC));
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

	}

void callback(char* topic, byte* payload, unsigned int length) {
	Serial.print("Message arrived [");
	Serial.print(topic);
	Serial.print("] ");
	for (int i = 0; i < length; i++) {
		Serial.print((char) payload[i]);
	}
	Serial.println();
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
			const TickType_t xDelay = 2000 / portTICK_PERIOD_MS;
			vTaskDelay(xDelay);
		}
	}
}

/*
 * task loops definitions
 */

void vPublishServiceTask(void * pvParameters) {

	// AUXILIARY VARS
	TickType_t xLastWakeTime; // store clock to guarantee periodic operation
	xParams_t * pxParams = (xParams_t *) pvParameters; // convert from void* to xParams_t *

	/* PUBLISH DATA SETUP */
	// Check if connected.
  if(WiFi.status() != WL_CONNECTED){

    Serial.println("PUBLISH SERVICE: Not connected, waiting for Wifi.");

    while ( WiFi.status() != WL_CONNECTED) {
      const TickType_t xDelay = 1000 / portTICK_PERIOD_MS;
      vTaskDelay( xDelay );
    }

    if(WiFi.status() == WL_CONNECTED)
      Serial.println("PUBLISH SERVICE: Reconnected.");
  }

	client.setServer(mqtt_server, 1883);
	client.setCallback(callback);

	for (;;) {

		/* PUBLISH DATA LOOP */
		vTaskDelayUntil(&xLastWakeTime, POSTING_INTERVAL); // Set periodic

		// Take access to temperature data
		xSemaphoreTake(pxParams->xTemperatureDataMutex, 3000);

		if (!client.connected()) reconnect();

		client.loop();

		mqttpublish(
			pxParams->pfTemperature[0],
			pxParams->pfTemperature[1],
			pxParams->pfTemperature[2]
		);

		// Release access to temperature data
		xSemaphoreGive(pxParams->xTemperatureDataMutex);

	}

}
