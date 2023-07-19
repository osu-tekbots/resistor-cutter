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
        SafetySwitch(int8_t pin) {
            this->pin = pin;

            pinMode(pin, INPUT_PULLUP);
            
            prevState = !digitalRead(pin);
        }
        
        bool getSwitch() {
            return digitalRead(pin);
        }

        bool getChange() {
            bool currentState = digitalRead(pin);

            if(currentState != prevState) {
                prevState = currentState;
                return true;
            }

            return false;
        }
};
