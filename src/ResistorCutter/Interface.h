/*
 * Functionality for providing a user interface with a Display object and Joystick object
 *
 * Nathaniel Baird
 * bairdn@oregonstate.edu
 *
 * Started:      07/12/2023
 * Last updated: 07/18/2023
 */

#include "Display.h"      // For LCD screen
#include "Joystick.h"     // For joystick control
#include "SafetySwitch.h" // For detecting an emergency interrupt
#include "LocalHost.h"    // For sending info to connected devices

class Interface {
private:
    Display       display;
    Joystick      joystick;
    SafetySwitch  safetySwitch;
    LocalHost     localHost;
    unsigned long lastUpdate;
    uint8_t       currentSelection;
    unsigned int  rPerKit, kits, percent, prevRunning, running; // running: 0=not, 1=yes, 2=paused
    bool          debounced, switchPressed;
    void (*callbackFn)(int);

    /**
     * @brief Updates object properties (ie rPerKit, kits) & displays based on inputs from joystick movements
     */
    void handleJoystick() {
        if(joystick.getVertical() && (debounced || millis() - lastUpdate >= 500)) {
            lastUpdate = millis();
            debounced = false;

            if (joystick.getUp()) {
                currentSelection = (currentSelection + 2) % 3;
            } else if (joystick.getDown()) {
                ++currentSelection %= 3;
            }

            display.updateAll(currentSelection, rPerKit, kits, percent, running);
            localHost.updatePageInfo(rPerKit, kits, running);
        } else if(joystick.getHorizontal() && (debounced || millis() - lastUpdate >= 250) && !joystick.getVertical()) {
            lastUpdate = millis();
            debounced = false;

            if (currentSelection == 0) {
                // Max of 10 per kit
                if (joystick.getLeft()) {
                    rPerKit = (rPerKit + 8) % 10 + 1;
                } else {
                    rPerKit = rPerKit % 10 + 1;
                }
            } else if (currentSelection == 1) {
                // Max of 50 kits
                if (joystick.getLeft()) {
                    kits = (kits + 48) % 50 + 1;
                } else {
                    kits = kits % 50 + 1;
                }
            }

            display.updateAll(currentSelection, rPerKit, kits, percent, running);
            localHost.updatePageInfo(rPerKit, kits, running);
        }
    }

    /** 
     * @brief Responds to the start/stop button being pressed using the joystick
     *     - Updates object properties to reflect the new running state
     *     - Updates the display to reflect the new running state
     *     - Calls the callback function `callbackFn` to allow further response to the state change
     */
    void handleSwitch() {
        if(!switchPressed && currentSelection == 2) {
            switchPressed = true;
        
            running = !running;

            display.updateAll(currentSelection, rPerKit, kits, percent, running);
            localHost.updatePageInfo(rPerKit, kits, running);

            callbackFn(running);
        }
    }

    /**
     * @brief Responds to the safety switch being pressed/ released
     *
     * @warning This function is non-blocking, ie it does NOT prevent code execution while the button is depressed!
     *              getRunning() MUST be called REGULARLY while running the machine to ensure that execution is allowed!
     */
    void handleSafetySwitch () {
        setPausedStatus(safetySwitch.getSwitch());
    }

public:
    /**
     * @param dispClk The ESP32 pin connected to the Nokia display's CLK pin
     * @param dispDin The ESP32 pin connected to the Nokia display's DIN pin
     * @param dispDc  The ESP32 pin connected to the Nokia display's DC pin
     * @param dispCe  The ESP32 pin connected to the Nokia display's CE pin
     * @param dispRst The ESP32 pin connected to the Nokia display's RST pin
     *
     * @param jstkX   The ESP32 pin connected to the joystick's VRx pin
     * @param jstkY   The ESP32 pin connected to the joystick's VRy pin
     * @param jstkSw  The ESP32 pin connected to the joystick's SW pin
     *
     * @param safeSw  The ESP32 pin connected to the safety interlock switch
     */   
    Interface(int8_t dispClk, int8_t dispDin, int8_t dispDc, int8_t dispCe, int8_t dispRst,
        int8_t jstkX, int8_t jstkY, int8_t jstkSw, int8_t safeSw)
        : display(dispClk, dispDin, dispDc, dispCe, dispRst), joystick(jstkX, jstkY, jstkSw), safetySwitch(safeSw) {
            lastUpdate = currentSelection = percent = prevRunning, running = 0;
            rPerKit = kits = 1;
            debounced = true;
            switchPressed = false;
            display.updateAll(currentSelection, rPerKit, kits, percent, running);
            localHost.updatePageInfo(rPerKit, kits, running);
    }
    
    /**
     * @brief Handles setup that must happen after the setup() function in the main .ino script has begun
     *
     * @warning MUST call this function in/after the main .ino script's setup function, NOT before
     */
    void setup() {
        localHost.setup();
    }

    /**
     * @brief Handles all UI updates
     *     - When not running, processes input from the joystick and saftey interlock switch
     *     - When running, updates the progress bar on the Nokia display
     *
     * @param percent The current completion percentage
     *     - @note Only needed while running
     */
    void update(int percent = -1) {
        if(safetySwitch.getChange())
            handleSafetySwitch(); 

        if(running == 2) return;

        if(running == 0) { 
            if(joystick.getUncentered()) {
                handleJoystick();
            } else {
                debounced = true;
            }
        } else /* running == 1 */{
            if(percent != -1) this->percent = percent;
            display.printProgress(percent, running);
        }

        if(joystick.getSwitch()) {
            handleSwitch();
        } else {
            switchPressed = false;
        }
    }

    /**
     * @brief Sets the event listener function to call when the Start/Stop button is pressed
     *
     * @param callbackFn A pointer to the function to call
     */
    void setButtonListener(void (*callbackFn)(int)) {
        this->callbackFn = callbackFn;
    }

    /**
     * @brief Sets the paused status & handles the necessary updates
     *
     * @param paused Whether the device is paused
     */
    void setPausedStatus(bool paused) {
        log_i("Detected pause change! Now %s\n", paused ? "pausing" : prevRunning ? "resuming" : "waiting to start");
        if(paused) {
            if(running != 2) prevRunning = running;
            running = 2;
        } else {
            running = prevRunning;
        }
        display.updateAll(currentSelection, rPerKit, kits, percent, running);
        localHost.updatePageInfo(rPerKit, kits, running);
    }

    /**
     * @brief Handles the necessary updates when running finishes
     *
     * @note MUST call or the UI will remain locked & showing that the device is running
     */
    void doneRunning() {
        if(running) {
            if(running != 2) running = 0;
            else prevRunning = 0;

            percent = 0;
            display.updateAll(currentSelection, rPerKit, kits, percent, running);
            localHost.updatePageInfo(rPerKit, kits, running);
        }
    }

    /**
     * @return The desired number of resistors for each kit
     */
    int getResistorsPerKit() {
        return rPerKit;
    }

    /**
     * @return The number of kits resistors should be cut for
     */
    int getKits() {
        return kits;
    }
    
    /**
     * @brief The current running status is 0 if not running, 1 if running, or 2 if paused
     *
     * @return The current running status 
     */
    int getRunningStatus() {
        return running;
    }
};
