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
        Joystick(int8_t vrX, int8_t vrY, int8_t sw) {
            this->vrX = vrX;
            this->vrY = vrY;
            this->sw  = sw;

            pinMode(sw, INPUT_PULLUP);
        }
        
        bool getSwitch() {
            return !digitalRead(sw);
        }

        bool getLeft() {
            return analogRead(vrX) < 0 + 500;
        }

        bool getRight() {
            return analogRead(vrX) > 4095 - 500;
        }

        bool getUp() {
            return analogRead(vrY) < 0 + 500;
        }

        bool getDown() {
            return analogRead(vrY) > 4095 - 500;
        }

        bool getUncentered() {
            return getUp() || getDown() || getLeft() || getRight();
        }

        bool getVertical() {
            return getUp() || getDown();
        }

        bool getHorizontal() {
            return getLeft() || getRight();
        }
};
