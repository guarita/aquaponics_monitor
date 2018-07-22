/*

	Created by Fábio Chiocchetti Guarita

*/

#include <Arduino.h>

#include "temperatureService.h"// Temperature reading service
#include "displayService.h" // Display managing service
#include "publishService.h" // Publish service
#include "httpdService.h" // HTTP daemon service
#include "parameters.h" // Common data/definitions shared between services

xParams_t xParams;
xParams_t * pxParams = &xParams;

void setup_wifi() {

	const char* ssid = "GVT-D528";
	const char* password = "1965003487";
	//const char* ssid     = "AndroidAP";
	//const char* password = "zbzm1164";

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

void setup(void) {

	// Create inter process communication VARS
	// These semaphores/mutexes are initialized before any task is run so they
	// should never be NULL in the tasks. (NULL verification not needed.)
	pxParams->xTemperatureDataAvailableForDisplay = xSemaphoreCreateBinary();
	pxParams->xTemperatureDataAvailableForPublishing = xSemaphoreCreateBinary();
	pxParams->xTemperatureDataMutex = xSemaphoreCreateMutex();

	// Start serial communication
	Serial.begin(115200);

	// Start wifi Connection
	setup_wifi();

	// Create tasks
	xTaskCreatePinnedToCore(&vTemperatureServiceTask, // pvTaskCode – Pointer to the task function.
			"TEMPERATURE LOOP",   // pcName – Debugging name of the task
			2048,             // usStackDepth – Size of the stack for the task.
			(void *) pxParams, // pvParameters – Parameters for the task instance. This may be NULL.
			6,                // uxPriority – Priority of the task instance.
			NULL, // pxCreatedTask – Reference to the newly created task instance. This may be
			0 // xCoreID - Which core to run on. Choices are: 0 (PRO CPU) or 1 (APP CPU) to "tskNO_AFFINITY".
			);

	xTaskCreatePinnedToCore(&vDisplayServiceTask, // pvTaskCode – Pointer to the task function.
			"DISPLAY LOOP",   // pcName – Debugging name of the task
			2048,             // usStackDepth – Size of the stack for the task.
			(void *) pxParams, // pvParameters – Parameters for the task instance. This may be NULL.
			4,                // uxPriority – Priority of the task instance.
			NULL, // pxCreatedTask – Reference to the newly created task instance. This may be
			1 // xCoreID - Which core to run on. Choices are: 0 (PRO CPU) or 1 (APP CPU) to "tskNO_AFFINITY".
			);

	xTaskCreatePinnedToCore(&vPublishServiceTask, // pvTaskCode – Pointer to the task function.
			"PUBLISH LOOP",   // pcName – Debugging name of the task
			2048,             // usStackDepth – Size of the stack for the task.
			(void *) pxParams, // pvParameters – Parameters for the task instance. This may be NULL.
			5,                // uxPriority – Priority of the task instance.
			NULL, // pxCreatedTask – Reference to the newly created task instance. This may be
			1 // xCoreID - Which core to run on. Choices are: 0 (PRO CPU) or 1 (APP CPU) to "tskNO_AFFINITY".
			);

	xTaskCreatePinnedToCore(&vHttpdServiceTask, // pvTaskCode – Pointer to the task function.
			"HTTPD LOOP",   // pcName – Debugging name of the task
			2048,             // usStackDepth – Size of the stack for the task.
			(void *) pxParams, // pvParameters – Parameters for the task instance. This may be NULL.
			5,                // uxPriority – Priority of the task instance.
			NULL, // pxCreatedTask – Reference to the newly created task instance. This may be
			1 // xCoreID - Which core to run on. Choices are: 0 (PRO CPU) or 1 (APP CPU) to "tskNO_AFFINITY".
			);

}

void loop(void) {

	delay(500);

}
