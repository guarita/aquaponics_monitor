#ifndef PUBLISHSERVICE_H
#define PUBLISHSERVICE_H

//PUBLISH DATA headers
#include <WiFi.h>
#include <PubSubClient.h>

// publish service task function
void publish_loop(void * pvParameters);

#endif
