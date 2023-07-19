/*
 * Functionality for reading input from a SMAKN Fr4 Ky-023 joystick
 *
 * Nathaniel Baird
 * bairdn@oregonstate.edu
 *
 * Started:      07/12/2023
 * Last updated: 07/18/2023
 */

class Joystick {
    private:
        int8_t vrX, vrY, sw;

    public:
        /**
         * @param vrX   The ESP32 pin connected to the joystick's VRx pin
         * @param vrY   The ESP32 pin connected to the joystick's VRy pin
         * @param sw    The ESP32 pin connected to the joystick's SW pin
         */
        Joystick(int8_t vrX, int8_t vrY, int8_t sw) {
            this->vrX = vrX;
            this->vrY = vrY;
            this->sw  = sw;

            pinMode(sw, INPUT_PULLUP);
        }
        
        /**
         * @return Whether the joystick switch is pressed down
         */
        bool getSwitch() {
            return !digitalRead(sw);
        }

        /**
         * @return Whether the joystick is angled left
         */
        bool getLeft() {
            return analogRead(vrX) < 0 + 500;
        }


        /**
         * @return Whether the joystick is angled right
         */
        bool getRight() {
            return analogRead(vrX) > 4095 - 500;
        }


        /**
         * @return Whether the joystick is angled up
         */
        bool getUp() {
            return analogRead(vrY) < 0 + 500;
        }


        /**
         * @return Whether the joystick is angled down
         */
        bool getDown() {
            return analogRead(vrY) > 4095 - 500;
        }

        /**
         * @return Whether the joystick is angled any direction
         */
        bool getUncentered() {
            return getUp() || getDown() || getLeft() || getRight();
        }

        /**
         * @return Whether the joystick is tilted vertically (up/down)
         */
        bool getVertical() {
            return getUp() || getDown();
        }

        /**
         * @return Whether the joystick is tilted horizontally (left/right)
         */
        bool getHorizontal() {
            return getLeft() || getRight();
        }
};
