#include "httpdService.h"

#define HTTPD_SERVICE_REFRESH_INTERVAL 5L * 1000L

void vHttpdServiceTask(void * pvParameters) {

	// AUXILIARY VARS
	TickType_t xLastWakeTime; // store clock to guarantee periodic operation
	xParams_t * pxParams = (xParams_t *) pvParameters; // convert from void * to xParams_t *

  // Definitions
  WiFiServer server(80);

  // Check if connected.
  if(WiFi.status() != WL_CONNECTED){

    Serial.println("HTTPD: Not connected, waiting for Wifi.");

    while ( WiFi.status() != WL_CONNECTED) {
      const TickType_t xDelay = 1000 / portTICK_PERIOD_MS;
      vTaskDelay( xDelay );
    }

    if(WiFi.status() == WL_CONNECTED)
      Serial.println("HTTPD: Reconnected.");

  }

  server.begin();

  for(;;){

    /* HTTP DAEMON SERVICE LOOP */
		vTaskDelayUntil(&xLastWakeTime, HTTPD_SERVICE_REFRESH_INTERVAL); // Set periodic

    // listen for incoming clients
    WiFiClient client = server.available();
    if (client) {
      Serial.println("new client");
      // an http request ends with a blank line
      boolean currentLineIsBlank = true;
      while (client.connected()) {
        if (client.available()) {
          char c = client.read();
          Serial.write(c);
          // if you've gotten to the end of the line (received a newline
          // character) and the line is blank, the http request has ended,
          // so you can send a reply
          if (c == '\n' && currentLineIsBlank) {
            // send a standard http response header
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: close");  // the connection will be closed after completion of the response
            client.println("Refresh: 5");  // refresh the page automatically every 5 sec
            client.println();
            client.println("<!DOCTYPE HTML>");
            client.println("<html>");

            // output the value of each temperature
            if(xSemaphoreTake(pxParams->xTemperatureDataMutex, 1000) == pdTRUE){
              // Temperature data available
              // Print each temperature
              for (int i = 0; i < pxParams->ucNumberOfDs18Devices; i++) {

                char buf[10]; // Auxiliar buffer for string operation
                client.print("Temperature[");
                client.print(i,DEC);
                client.print("]: ");
                sprintf(buf, "%.1f", pxParams->pfTemperature[i]);
                client.print("°C");
                client.println("<br />");

              }

              xSemaphoreGive(pxParams->xTemperatureDataMutex);

            } else {
              // Temperature data not available
              client.print("Temperature data not available. Try again soon. <br />");

            }

            // end page
            client.println("</html>");
             break;

          }

          if (c == '\n') {
            // you're starting a new line
            currentLineIsBlank = true;
          }

          else if (c != '\r') {

            // you've gotten a character on the current line
            currentLineIsBlank = false;

          }

        }

      }

      // give the web browser time to receive the data
      const TickType_t xDelay = 1;
      vTaskDelay(xDelay);

      // close the connection:
      client.stop();
      Serial.println("HTTPD: client disonnected");

    }

  }

}
