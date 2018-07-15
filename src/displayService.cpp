#include "displayService.h"

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

void draw(void * pvParameters) {

  // Auxiliary vars
  xParams_t * pxParams = (xParams_t *) pvParameters; // convert from void* to xParams_t *

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
	if (!pxParams->bTempStarted) {
		//Not initialized
	} else {

		for (int i = 0; i < pxParams->ucNumberOfDevices; i++) {

			char addrStr[5];
			char buff[100];
			sprintf(addrStr, "%02X%02X", pxParams->pucTemperatureAddress[i][6], pxParams->pucTemperatureAddress[i][7]);

			sprintf(buff, "T[%s]:%.2f C", addrStr, pxParams->pfTemperature[i]);
			u8g2.drawStr(0, 12 + 8 * i, buff);
		}
	}

	//Status messages

}

void draw_loop(void * pvParameters) {

  xParams_t * pxParams = (xParams_t *) pvParameters; // convert from void* to xParams_t *

	/* OLED SETUP */
	u8g2.begin();

	for (;;) {
		/* OLED PICTURE LOOP */
		u8g2.clearBuffer();
		draw(pvParameters);
		u8g2.sendBuffer();

		delay(1000);

	}
}
