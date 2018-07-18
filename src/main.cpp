/*

	Created by Fábio Chiocchetti Guarita

*/

#include <Arduino.h>

#include "temperatureService.h"// Temperature reading service
#include "displayService.h" // Display managing service
#include "publishService.h" // Publish service
#include "parameters.h" // Common data/definitions shared between services

xParams_t xParams;
xParams_t * pxParams = &xParams;

void setup(void) {

	// Create inter process communication VARS
	pxParams->xTemperatureDataAvailableForDisplay = xSemaphoreCreateBinary();
	pxParams->xTemperatureDataAvailableForPublishing = xSemaphoreCreateBinary();
	pxParams->xTemperatureDataMutex = xSemaphoreCreateMutex();

	// Start serial communication
	Serial.begin(115200);

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

}

void loop(void) {

	delay(500);

}
