/*
 * Functionality for creating a WiFi AP and a captive portal for displaying a webpage automatically on network connection
 *
 * CREDIT: Adapted from an example from CD_FER:
 *     https://github.com/CDFER/Captive-Portal-ESP32/blob/main/src/main.cpp
 * Their comments have been preserved with the preface "---"
 *
 * Nathaniel Baird
 * bairdn@oregonstate.edu
 *
 * Started:      07/12/2023
 * Last updated: 07/13/2023
 */

#include <AsyncTCP.h>          // --- https://github.com/me-no-dev/AsyncTCP using the latest dev version from @me-no-dev
#include <DNSServer.h>
#include <ESPAsyncWebServer.h> // --- https://github.com/me-no-dev/ESPAsyncWebServer using the latest dev version from @me-no-dev
#include <esp_wifi.h>		   // --- Used for mpdu_rx_disable android workaround
#include "Webpages.h"

// --- Pre reading on the fundamentals of captive portals https://textslashplain.com/2022/06/24/captive-portals/

#define SSID "Resistor_Cutter"  // NOTE: The SSID can't have a space in it.
#define PASSWORD "tekb0ts!" // NOTE: Password MUST be >=8 chars && <= 62 chars -- esp_wifi.h requirement

#define MAX_CLIENTS 1	// --- Define the maximum number of clients that can connect to the server -- ESP32 supposedly supports up to 10
                        // WARNING: Set to 1 to allow proper handling of captive portal escape for JS
#define WIFI_CHANNEL 6	// --- 2.4ghz channel 6 https://en.wikipedia.org/wiki/List_of_WLAN_channels#2.4_GHz_(802.11b/g/n/ax)
#define DNS_INTERVAL 30 // --- Define the DNS interval in milliseconds between processing DNS requests

class LocalHost {
    private: 
        const IPAddress localIP;		   // the IP address the web server, Samsung requires the IP to be in public space
        const IPAddress gatewayIP;		   // IP address of the network should be the same as the local IP for captive portals
        const IPAddress subnetMask;  // no need to change: https://avinetworks.com/glossary/subnet-mask/

        const String localIPURL;	 // a string version of the local IP with http, used for redirecting clients to your webpage

        DNSServer dnsServer;
        AsyncWebServer server;
        bool portalOpened;

        Webpages webpages;
    
        // Credit: CD_FER
        void setUpDNSServer(DNSServer &dnsServer, const IPAddress &localIP) {
            // --- Set the TTL for DNS response and start the DNS server
            dnsServer.setTTL(3600);
            dnsServer.start(53, "*", localIP);
        }

        // Credit: CD_FER
        void startSoftAccessPoint(const char *ssid, const char *password, const IPAddress &localIP, const IPAddress &gatewayIP) {
            // --- Set the WiFi mode to access point
            WiFi.mode(WIFI_MODE_AP);

            // --- Define the subnet mask for the WiFi network
            const IPAddress subnetMask(255, 255, 255, 0);

            // --- Configure the soft access point with a specific IP and subnet mask
            WiFi.softAPConfig(localIP, gatewayIP, subnetMask);

            // --- Start the soft access point with the given ssid, password, channel, max number of clients
            WiFi.softAP(ssid, password, WIFI_CHANNEL, 0, MAX_CLIENTS);

            // --- Disable AMPDU RX on the ESP32 WiFi to fix a bug on Android
            esp_wifi_stop();
            esp_wifi_deinit();
            wifi_init_config_t my_config = WIFI_INIT_CONFIG_DEFAULT();
            my_config.ampdu_rx_enable = false;
            esp_wifi_init(&my_config);
            esp_wifi_start();
            vTaskDelay(100 / portTICK_PERIOD_MS);  // --- Add a small delay

        }

        // Credit: CD_FER
        void setUpWebserver(AsyncWebServer &server, const IPAddress &localIP) {
            // --- ======================== Webserver ========================
            // --- WARNING IOS (and maybe macos) WILL NOT POP UP IF IT CONTAINS THE WORD "Success" https://www.esp8266.com/viewtopic.php?f=34&t=4398
            // --- SAFARI (IOS) IS STUPID, G-ZIPPED FILES CAN'T END IN .GZ https://github.com/homieiot/homie-esp8266/issues/476 this is fixed by the webserver serve static function.
            // --- SAFARI (IOS) there is a 128KB limit to the size of the HTML. The HTML can reference external resources/images that bring the total over 128KB
            // --- SAFARI (IOS) popup browser has some severe limitations (javascript disabled, cookies disabled)

            // --- Required
            server.on("/connecttest.txt", [&](AsyncWebServerRequest *request) { request->redirect("http://logout.net"); });	// --- windows 11 captive portal workaround
            server.on("/wpad.dat", [&](AsyncWebServerRequest *request) { request->send(404); });								// --- Honestly don't understand what this is but a 404 stops win 10 keep calling this repeatedly and panicking the esp32 :)

            // --- Background responses: Probably not all are Required, but some are. Others might speed things up?
            // --- A Tier (commonly used by modern systems)
            server.on("/generate_204", [&](AsyncWebServerRequest *request) { request->redirect(localIPURL); });		   // --- android captive portal redirect
            server.on("/redirect", [&](AsyncWebServerRequest *request) { request->redirect(localIPURL); });			   // --- microsoft redirect
            server.on("/hotspot-detect.html", [&](AsyncWebServerRequest *request) { 
                Serial.println("\"/hotspot-detect.html\" called");
                request->redirect(localIPURL); 
            });  // --- apple call home
            server.on("/canonical.html", [&](AsyncWebServerRequest *request) { request->redirect(localIPURL); });	   // --- firefox captive portal call home
            server.on("/success.txt", [&](AsyncWebServerRequest *request) { request->send(200); });					   // --- firefox captive portal call home
            server.on("/ncsi.txt", [&](AsyncWebServerRequest *request) { request->redirect(localIPURL); });			   // --- windows call home

            // --- return 404 to webpage icon
            server.on("/favicon.ico", [&](AsyncWebServerRequest *request) { request->send(404); });	// webpage icon

            // Serve the appropriate webpage
            server.on("/", HTTP_ANY, [&](AsyncWebServerRequest *request) {this->processRequest(request);});

            // --- the catch all
            server.onNotFound([&](AsyncWebServerRequest *request) {
                request->redirect(localIPURL);
                Serial.print("onnotfound ");
                Serial.print(request->host());
                Serial.print(" ");
                Serial.print(request->url());
                Serial.print(" sent redirect to " + localIPURL + "\n");
            });
        }
        
        void processRequest(AsyncWebServerRequest *request) {
            Serial.println("Recieved HTTP request");
            Serial.print("Requested host: ");
            Serial.print(request->host());
            Serial.print("; URL requested: ");
            Serial.print(request->url());
            Serial.print("; Parameters:\n");

            if(request->host().indexOf("citrix") > -1) {
                request->send(404);
                return;
            }

            for(int i = 0; i < request->params(); i++) {
                AsyncWebParameter* p = request->getParam(i);
                Serial.printf("    Name: %s; Value: %s", p->name().c_str(), p->value().c_str());
            }

            if(request->hasParam("redirect")) {
                portalOpened = true;
                AsyncWebServerResponse *response = request->beginResponse(200, "text/html", webpages.getMainHTML());
                response->addHeader("Cache-Control", "public,no-store");  // don't save this file to cache
                request->send(response);
                Serial.println("Served Main HTML Page\n");
            } else if(portalOpened) {
                AsyncWebServerResponse *response = request->beginResponse(200, "text/html", webpages.getSuccessHTML());
                response->addHeader("Cache-Control", "public,no-store");  // don't save this file to cache
                request->send(response);
                Serial.println("Served Success HTML Page\n");
            } else {
                AsyncWebServerResponse *response = request->beginResponse(200, "text/html", webpages.getCaptiveHTML());
                response->addHeader("Cache-Control", "public,no-store");  // don't save this file to cache
                request->send(response);
                Serial.println("Served Captive HTML Page\n");
            }
        }

    public:
        LocalHost() : localIP(4, 3, 2, 1), gatewayIP(4, 3, 2, 1), subnetMask(255, 255, 255, 0), localIPURL("http://4.3.2.1/"), server(80) {
            portalOpened = false;
        }

        // WARNING: Setup fn is MANDATORY to prevent obscure issues when setting up WiFi AP
        void setup() {
            startSoftAccessPoint(SSID, PASSWORD, localIP, gatewayIP);

            setUpDNSServer(dnsServer, localIP);

            setUpWebserver(server, localIP);
            server.begin();
        }

        void update() {
            dnsServer.processNextRequest();	 // --- I call this atleast every 10ms in my other projects (can be higher but I haven't tested it for stability)
        }
};