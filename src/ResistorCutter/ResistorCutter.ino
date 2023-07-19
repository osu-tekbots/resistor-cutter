/*
 * Resistor Reel Cutter for TekBots resistor reels
 *
 * Nathaniel Baird
 * bairdn@oregonstate.edu
 *
 * Started:      07/10/2023
 * Last updated: 07/18/2023
 */

/**
 * @todo change Interface.h to define macros for RUNNING, PAUSED, NOT_RUNNING?
 */

#include "pins.h"       // List of all pin connections
#include "Interface.h"  // For I/O using the LCD and joystick

Interface interface(CLK_PIN, DIN_PIN, DC_PIN, CE_PIN, RST_PIN, VRx_PIN, VRy_PIN, SW_PIN, SAFE_PIN);

void handleStateChange(int state);
void checkPause(void);

void setup() {
    Serial.begin(115200);
    while(!Serial); // Wait for serial port to connect
    Serial.println("\n\nResistor cutter, compiled " __DATE__ " " __TIME__ " by bairdn");

    interface.setup();
    interface.setButtonListener(handleStateChange);
}

void loop() {
    interface.update();

    checkPause();
}

/**
 * @brief Checks if the user passed a pause/resume command through the Serial interface
 */
void checkPause(void) {
    if(Serial.available()) {
        String input = Serial.readStringUntil('\n');

        if(input.equalsIgnoreCase("pause")) {
            interface.setPausedStatus(true);
        } else if(input.equalsIgnoreCase("resume")) {
            interface.setPausedStatus(false);
        }
    }
}

/**
 * @brief Handles UI button press (start/stop the machine)
 *     - Prints diagnostic info based on running state
 *     - Gradually fills the progress bar when the resistor cutter is started
 *
 * @param state The current running state: 0 for stopped, 1 for running, 2 for paused
 */
void handleStateChange(int state) {
    Serial.printf("Button pressed! Machine is %s.", interface.getRunningStatus() ? "on" : "off");
    if(state == 2) {
        // Shouldn't be possible
        Serial.println("\nERR: Somehow, handleStateChange() was called while paused");
    } else if(state == 1) {
        Serial.printf(" Cutting groups of %d resistors for %d kits.\n", interface.getResistorsPerKit(), interface.getKits());
    } else {
        Serial.println();
        return;
    }

    for(int i = 0; i <= 100; i++) {
        interface.update(i);// call function to update completion bar

        if(interface.getRunningStatus() == 2) i--;
        checkPause();

        delay(50);
    }

    // Have to tell Interface.h that the machine is no longer running
    interface.doneRunning();
}
