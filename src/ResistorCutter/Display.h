/*
 * Functionality for displaying data on a Nokia 5110 LCD display
 *
 * Nathaniel Baird
 * bairdn@oregonstate.edu
 *
 * Started:      07/11/2023
 * Last updated: 07/18/2023
 */

/***********************************************************************************************************\
*                                                                                                           *
* WARNING: This class REQUIRES MODIFICATION to the Adafruit_PCD8544 library as follows:                     *
*                                                                                                           *
* Adafruit_PCD8544.h -- Add the following to the public section (eg line 89):                               *
*       void invertRect(int16_t x, int16_t y, int16_t w, int16_t h);                                        *
*                                                                                                           *
* Adafruit_PCD8544.cpp -- Add the following to the document (eg line 433):                                  *
*       void Adafruit_PCD8544::invertRect(int16_t x, int16_t y, int16_t w, int16_t h) {                     *
*           for(int i = x; i < x + w; i++) {                                                                *
*               for(int j = y; j < y + h; j++) {                                                            *
*                   if(getPixel(i, j, pcd8544_buffer)) setPixel(i, j, WHITE, pcd8544_buffer);               *
*                   else setPixel(i, j, BLACK, pcd8544_buffer);                                             *
*               }                                                                                           *
*           }                                                                                               *
*       }                                                                                                   *
*                                                                                                           *
\***********************************************************************************************************/

#include <Adafruit_PCD8544.h> // Version 2.0.1 -- For LCD screen

class Display {
    private:
        // 14chars x 6chars, 84px x 48px
        Adafruit_PCD8544 display;

    public:
        /**
         * @param sclk The ESP32 pin connected to the Nokia display's CLK pin
         * @param din  The ESP32 pin connected to the Nokia display's DIN pin
         * @param dc   The ESP32 pin connected to the Nokia display's DC pin
         * @param cs   The ESP32 pin connected to the Nokia display's CE pin
         * @param rst  The ESP32 pin connected to the Nokia display's RST pin
         */
        Display(int8_t sclk, int8_t din, int8_t dc, int8_t cs, int8_t rst) : display(sclk, din, dc, cs, rst) {
            display.begin();
            display.setTextSize(1);
        }

        /**
         * @brief Updates all parts of the Nokia display to reflect the machine's current state
         *
         * @param highlightNum Which input is highlighted/selected (eg # inputs, buttons, etc)
         * @param rPerKit      How many resistors per kit are currently wanted
         * @param kits         How many kits are currently wanted
         * @param percent      If running, the percentage of the job that is complete
         * @param running      The current running state (see Interface.h)
         */
        void updateAll(int highlightNum, int rPerKit, int kits, int percent, int running) {
            if(running == 2) {
                showPaused();
                return;
            }

            display.clearDisplay();

            printLn1(rPerKit, highlightNum == 0, running);
            printLn2(kits, highlightNum == 1, running);
            printProgress(percent, running);
            printButton(highlightNum == 2, running);
        }

        /** 
         * @brief Updates the first line of the UI (rPerKit)
         *
         * @note Usually just called by updateAll(), but also allows greater control over what is changed
         *
         * @param number      The number to display as the user input (for rPerKit)
         * @param highlighted Whether the input on this line should be highlighted (showing it's selected)
         * @param running     Whether to display in running mode (input field shown as static)
         */
        void printLn1(int number, bool highlighted, bool running) {
            display.fillRect(0, 0, 84, 11, WHITE);

            display.setTextColor(BLACK);
            display.setCursor(4, 2);
            display.print("R per kit:");
            
            if(!running) {
                if(highlighted) {
                    display.fillRect(64, 0, 15, 11, BLACK);
                    display.setTextColor(WHITE, BLACK); // invert text
                } else {
                    display.drawRect(64, 0, 15, 11, BLACK);
                    display.setTextColor(BLACK);
                }
            }

            if(number < 10) {
                display.setCursor(69, 2);
            } else {
                display.setCursor(65, 2);
            }
            display.print(number);
            
            display.display();
        }

        /** 
         * @brief Updates the second line of the UI (kits)
         *
         * @note Usually just called by updateAll(), but also allows greater control over what is changed
         *
         * @param number      The number to display as the user input (for kits)
         * @param highlighted Whether the input on this line should be highlighted (showing it's selected)
         * @param running     Whether to display in running mode (input field shown as static)
         */
        void printLn2(int number, bool highlighted, bool running) {
            display.fillRect(0, 12, 84, 11, WHITE);

            display.setTextColor(BLACK);
            display.setCursor(20, 14);
            display.print("Kits:");
            
            if(!running) {
                if(highlighted) {
                    display.fillRect(50, 12, 15, 11, BLACK);
                    display.setTextColor(WHITE, BLACK); // invert text
                } else {
                    display.drawRect(50, 12, 15, 11, BLACK);
                    display.setTextColor(BLACK);
                }
            }

            if(number < 10) {
                display.setCursor(55, 14);
            } else {
                display.setCursor(52, 14);
            }
            display.print(number);
            
            display.display();
        }

        /** 
         * @brief Updates the progress bar when the machine is running
         *
         * @param percent     The percentage of completion to display
         * @param running     Verifies that the progress bar should be printed (fn returns if false)
         */
        void printProgress(int percent, bool running) {
            if(!running) return;
            if(percent > 100) percent = 100;

            display.fillRect(0, 24, 84, 11, WHITE);

            display.drawRect(4, 24, 77, 11, BLACK);
            display.setTextColor(BLACK);

            if(percent < 10) {
                display.setCursor(36, 26);
            } else if(percent < 100) {
                display.setCursor(33, 26);
            } else {
                display.setCursor(30, 26);
            }

            display.printf("%d%%", percent);

            /***********************************************************************************************************\
            *                                                                                                           *
            * NOTE: The following line changes the appropriate pixels in the progress bar to high contrast mode. This   *
            *            allows the percentage text to still be visible as the bar fills in.                            *
            * --HOWEVER--, it also REQUIRES MODIFICATION to the Adafruit_PDC8544 library! Changes are as follows:       *
            *                                                                                                           *
            * Adafruit_PCD8544.h -- Add the following to the public section (eg line 89):                               *
            *       void invertRect(int16_t x, int16_t y, int16_t w, int16_t h);                                        *
            *                                                                                                           *
            * Adafruit_PCD8544.cpp -- Add the following to the document (eg line 433):                                  *
            *       void Adafruit_PCD8544::invertRect(int16_t x, int16_t y, int16_t w, int16_t h) {                     *
            *           for(int i = x; i < x + w; i++) {                                                                *
            *               for(int j = y; j < y + h; j++) {                                                            *
            *                   if(getPixel(i, j, pcd8544_buffer)) setPixel(i, j, WHITE, pcd8544_buffer);               *
            *                   else setPixel(i, j, BLACK, pcd8544_buffer);                                             *
            *               }                                                                                           *
            *           }                                                                                               *
            *       }                                                                                                   *
            *                                                                                                           *
            \***********************************************************************************************************/
            display.invertRect(5, 25, percent*75/100, 9);
            
            display.display();
        }

        /**
         * @brief Updates the start/stop button
         *
         * @param highlighted Whether the button is currently highlighted (showing it's selected)
         * @param running     Whether the machine is currently running (to display "Start" vs "Stop")
         */
        void printButton(bool highlighted, bool running) {
            display.fillRect(25, 37, 6*5+3, 8*1+3, WHITE);

            if(highlighted) {
                display.fillRect(25, 37, 6*5+3, 8*1+3, BLACK);
                display.setTextColor(WHITE, BLACK); // invert text
            } else {
                display.drawRect(25, 37, 6*5+3, 8*1+3, BLACK);
                display.setTextColor(BLACK);
            }

            if(running) {
                display.setCursor(30, 39);
                display.print("Stop");
            } else {
                display.setCursor(27, 39);
                display.print("Start");
            }
            
            display.display();
        }

        /**
         * @brief Displays a screen recognizing that the safety interlock switch has been pressed
         */
        void showPaused() {
            display.clearDisplay();

            display.setTextColor(BLACK);
            display.setTextSize(2);
            
            display.setCursor(7, 0);
            display.print("PAUSED");

            display.drawLine(0, 15, 84, 15, BLACK);

            display.setTextSize(1);
            display.setCursor(0, 17);
            display.print("Safety switch flipped;      please resolve\nthe issue!");

            display.display();
        }
};
