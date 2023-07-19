/*
 * Functionality for reading input from a safety interlock switch
 *
 * Nathaniel Baird
 * bairdn@oregonstate.edu
 *
 * Started:      07/17/2023
 * Last updated: 07/18/2023
 */

class SafetySwitch {
    private:
        int8_t pin;
        bool   prevState;

    public:
        /**
         * @param pin The ESP32 pin connected to the safety interlock switch
         */
        SafetySwitch(int8_t pin) {
            this->pin = pin;

            pinMode(pin, INPUT_PULLUP);
            
            prevState = !digitalRead(pin);
        }
        
        /**
         * @return Whether the switch is pressed
         */
        bool getSwitch() {
            return digitalRead(pin);
        }

        /**
         * @todo Change to allow calling from different places (using calling ID param)
         *
         * @return Whether the switch state has changed since the function was last called
         */
        bool getChange() {
            bool currentState = digitalRead(pin);

            if(currentState != prevState) {
                prevState = currentState;
                return true;
            }

            return false;
        }
};
