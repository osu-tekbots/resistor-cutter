/*
 * Encapsulation for the functions that process HTML for each page that's served 
 *
 * Nathaniel Baird
 * bairdn@oregonstate.edu
 *
 * Started: 07/13/2023
 * Last updated: 07/13/2023
 */

class Webpages {
    private:
        const char* captiveHTML = R"=====(
                <!DOCTYPE html> 
                <html>
                    <head>
                    <title>ESP32 Captive Portal</title>
                    <meta name="viewport" content="width=device-width, initial-scale=1.0">
                    <meta http-equiv="refresh" content="0; url=http://www.tekbots.com/?redirect=true">
                    </head>
                    <body>
                    <h1><a href="http://www.tekbots.com/?redirect=true">Click me to view resistor cutter status</a></h1>
                    </body>
                </html>
            )=====";
        const char* successHTML = R"=====(
                <!DOCTYPE html>
                <html>
                    <head>
                        <title>Success</title>
                        <meta http-equiv="refresh" content="0; url=http://www.tekbots.com/?redirect=true">
                    </head>
                    <body>
                        Success
                    </body>
                </html>
            )=====";
        const char* mainHTML = R"=====(
                <!DOCTYPE html> 
                <html>
                    <head>
                    <title>ESP32 Captive Portal</title>
                    <meta name="viewport" content="width=device-width, initial-scale=1.0">
                    <meta http-equiv="refresh" content="5" url="http://www.tekbots.com/?redirect=true">
                    <style>
                        .cutting {
                            background: #98ff98;
                        }

                        .notCutting {
                            background: #ff9898;
                        }

                        #container {
                            display: flex;
                            justify-content: center;
                            align-items: center;
                        }

                        div > div {
                            width: 200px;
                            height: 125px;

                            display: flex;
                            justify-content: center;
                            align-items: center;
                            text-align: center;
                        }
                    </style>
                    </head>
                    <body>
                    <h1 style="text-align: center; width: 100%;">Resistor Cutter Status</h1>
                    <div id="container">
                        <div id="status" class="cutting"><p>Cutting</p></div>
                        <div id="status2" class="notCutting"><p>Not Cutting</p></div>
                    </div>
                    </body>
                </html>
            )=====";

        int rPerKit, kits, percent;
        bool running;
    
    public:
        Webpages(int rPerKit = 0, int kits = 0, int percent = 0, bool running = false) {
            this->rPerKit = rPerKit;
            this->kits = kits;
            this->percent = percent;
            this->running = running;
        }
        
        void setRPerKit(int rPerKit) {
            this->rPerKit = rPerKit;
        }
        void setKits(int kits) {
            this->kits = kits;
        }
        void setPercent(int percent) {
            this->percent = percent;
        }
        void setRunning(bool running) {
            this->running = running;
        }

        const char* getCaptiveHTML() {
            return captiveHTML;
        }

        const char* getSuccessHTML() {
            return successHTML;
        }

        const char* getMainHTML() {
            return mainHTML;
        }
};
