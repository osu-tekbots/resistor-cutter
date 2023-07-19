/*
 * Encapsulation for the functions that process HTML for each page that's served 
 *
 * Nathaniel Baird
 * bairdn@oregonstate.edu
 *
 * Started:      07/13/2023
 * Last updated: 07/18/2023
 */

class Webpages {
    private:
        const char* captiveHTML = R"=====(
            <!DOCTYPE html> 
            <html>
                <head>
                    <title>ESP32 Captive Portal</title>
                    <meta name="viewport" content="width=device-width, initial-scale=1.0">
                    <meta http-equiv="refresh" content="0; url=http://www.neverssl.com/?redirect=true">
                </head>
                <body>
                    <h1><a href="http://www.neverssl.com/?redirect=true">Click me to view resistor cutter status</a></h1>
                </body>
            </html>
            )=====";
        const char* successHTML = R"=====(
            <!DOCTYPE html>
            <html>
                <head>
                    <title>Success</title>
                    <meta http-equiv="refresh" content="0; url=http://www.neverssl.com/?redirect=true">
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
                    <meta http-equiv="refresh" content="1" url="http://www.neverssl.com/?redirect=true">
                    <style>
                        body {
                            display: flex;
                            align-items: center;
                            flex-direction: column;
                        }

                        .container {
                            display: flex;
                            justify-content: center;
                            align-items: center;
                            margin: 5px;
                        }

                        div > div {
                            width: 200px;
                            height: 125px;

                            display: flex;
                            flex-direction: column;
                            justify-content: center;
                            align-items: center;
                            text-align: center;
                        }

                        #status > div {
                            border: 2px solid black;
                            border-radius: 8px;
                        }

                        .cutting {
                            background: #98ff98;
                        }

                        .notCutting {
                            background: #ff9898;
                        }

                        .paused {
                            background: #ffcc98;
                        }

                        #data {
                            border: 2px solid black;
                            border-radius: 8px;
                            max-width: 400px;
                        }

                        #data h2 {
                            margin-bottom: -10px;
                            color: darkgrey;
                        }
                    </style>
                </head>
                <body>
                    <h1>Resistor Cutter Status</h1>

                    <div id="data" class="container">
                        <div id="rPerKit" style="border-right: 1px solid black;">
                            <h2>Resistors Per Kit</h2>
                            <h1>{{rPerKit}}</h1>
                        </div>
                        <div id="kits">
                            <h2>Kits</h2>
                            <h1>{{kits}}</h1>
                        </div>
                    </div>
                    
                    <div id="status" class="container">
                        <div class="{{cuttingClass}}"><h1>{{cuttingText}}</h1></div>
                    </div>
                </body>
            </html>
            )=====";

        int rPerKit, kits, percent, running;
    
    public:
        /**
         * @brief Initialization allows the webpages to accurately display information from the moment they're first created
         * 
         * @param rPerKit How many resistors per kit are currently wanted
         * @param kits    How many kits are currently wanted
         * @param percent If running, the percentage of the job that is complete
         * @param running The current running state (see Interface.h)
         */
        Webpages(int rPerKit = 0, int kits = 0, int percent = 0, int running = 0) {
            this->rPerKit = rPerKit;
            this->kits = kits;
            this->percent = percent;
            this->running = running;
        }
        
        /**
         * @param rPerKit How many resistors per kit are currently wanted
         */
        void setRPerKit(int rPerKit) {
            this->rPerKit = rPerKit;
        }
        /**
         * @param kits    How many kits are currently wanted
         */
        void setKits(int kits) {
            this->kits = kits;
        }
        /**
         * @param running The current running state (see Interface.h)
         */
        void setRunning(int running) {
            this->running = running;
        }
        /**
         * @param percent If running, the percentage of the job that is complete
         */
        void setPercent(int percent) {
            this->percent = percent;
        }

        /**
         * @return The HTML needed to generate the captive portal
         */
        const char* getCaptiveHTML() {
            return captiveHTML;
        }

        /**
         * @return The HTML needed to resolve the captive portal
         *
         * @note This is particularly necessary on iOS, to change the "cancel" button to a "Done" 
         *           button after the portal has been generated
         */
        const char* getSuccessHTML() {
            return successHTML;
        }

        /**
         * @return The filled HTML for the main status page
         *
         * @note This fills the mainHTML const with the data (eg rPerKit, running, etc) currently saved
         *           in THIS CLASS, NOT Interface.h
         */
        const char* getMainHTML() {
            static std::string htmlCopy;
            size_t index = 0;

            htmlCopy = mainHTML;

            index = htmlCopy.find("{{rPerKit}}");
            if(index != std::string::npos) htmlCopy.replace(index, strlen("{{rPerKit}}"), String(rPerKit).c_str());

            index = htmlCopy.find("{{kits}}");
            if(index != std::string::npos) htmlCopy.replace(index, strlen("{{kits}}"), String(kits).c_str());
            
            index = htmlCopy.find("{{cuttingClass}}");
            if(index != std::string::npos) htmlCopy.replace(index, strlen("{{cuttingClass}}"), running == 1 ? "cutting" : running == 0 ? "notCutting" : "paused");
            
            index = htmlCopy.find("{{cuttingText}}");
            if(index != std::string::npos) htmlCopy.replace(index, strlen("{{cuttingText}}"), running == 1 ? "Cutting" : running == 0 ? "Not Cutting" : "Paused");

            return htmlCopy.c_str();
        }
};
