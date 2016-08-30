#include <myVector.h>

/**

    *Simple_CLI*

    This program implements a CLI to control the GPIOs of an Arduino
    Nano. When new serial data arrives, this sketch adds it to a String.
    When a newline is received, the loop parses the string and
    activates GPIOs according to the commands.

    @version 11, 22/16/16
    @author Varun Malhotra

*/



#define UP_ARROW 65
#define DOWN_ARROW 66
#define RIGHT_ARROW  67
#define LEFT_ARROW  68
#define ESC 27


String inputString = "";         // a string to hold incoming data
bool stringComplete = false;  // whether the string is complete
int count = 0;  // for keeping track of the number of characters in inputString
int pwmPinStatus [] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // 0: low, 255: high
    //, 256: input, 1-254: pwm
    // used to generate a formated output string

int upNdx = 0;
int downNdx = 0;
myVector myVec;

/**
    fuction to check if a string has digits

    @param string input
    @return boolean value, true if string has digits in it
*/
bool isNum(String str) {

    bool ret = true;

    for (int i = 0; i < str.length(); i++) {

        if (!(isDigit(str[i]))) {

            ret = false;
            break;

        }
    }

    return (ret);

}

/**
    function to read input values

    @param pin number to read input values from
    @param represents if the pin is digital or analog
*/
void readInput(int pin, char type) {

    char strOut[100] = "";
    int valRead = 0;
    if (type == 'D') {

        valRead = digitalRead(pin);

    } else if (type == 'A') {

        analogRead(pin);
        delay(10);
        valRead = analogRead(pin);
    }
    sprintf(strOut, "\n\rPin %c%d: %d", type, pin % 14, valRead);
    Serial.print(strOut);

}

void reset () {

    //directly accessing pin regesters and setting them to 0 (INPUT)
    DDRD = DDRD & 0;
    DDRB = DDRB & 0;
    DDRC = DDRC & 0;
    Serial.print(F("\n\rAll pins set to INPUT"));

}

/**
    funtion to set all pins to LOW

    @param pin number to set to LOW
    @param represents if the pin is digital or analog
*/
void setToLow(int pin, char type) {

    char strOut[50] = "";
    int i = pin;
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);

    if (type == 'A')
        i -= 14;
    sprintf(strOut, "\n\rPin %c%d set to LOW", type, i);
    Serial.print(strOut);

}

/**
    funtion to set all pins to HIGH

    @param pin number to set to HIGH
    @param represents if the pin is digital or analog
*/
void setToHigh(int pin, char type) {

    char strOut[50] = "";
    int i = pin;
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);

    if (type == 'A')
        i -= 14;
    sprintf(strOut, "\n\rPin %c%d set to HIGH", type, i);
    Serial.print(strOut);
}

/*
    funtion to set all pins to LOW or HIGH

    @param indicating setting (LOW or HIGH)
*/
void setAll(String level) {

    bool val = level.equals("high");
    int intVal = val ? 255 : 0;

    for (int i = 2; i <= 19; i++) {

        if (i == 13) {
            continue;
        }

        pinMode(i, OUTPUT);
        digitalWrite(i, val);


        if (i < 12)
            pwmPinStatus[i] = intVal;    // to keep track of pwm pins
        }

}


void setup() {
    // initialize serial
    Serial.begin(115200);
    // reserve 200 bytes for the inputString
    inputString.reserve(100);

    /*
    DDRD controls digital pins 0-7 (0 & 1 not to be used)
    DDRB controls digital pins  8-13 (13 not to be used)
    DDRC controls analog pins 0-5
    binary value of 0 indicates INPUT, 1: OUTPUT
    */

    // reseting pins to INPUT
    DDRD = DDRD & 0;
    DDRB = DDRB & 0;
    DDRC = DDRC & 0;
    // prompt
    Serial.print(F("GPIO: "));
}


void loop() {
    // parse the string when a newline arrives:
    if (stringComplete) {

        stringComplete = false;
        count = 0;  // used to keep track of input characters in serialEvent()
        int indexEnd = 0;   // used to split multiple pins seperated by ','
        int pin = 2;    // pin number defaults to digital pin 2
        String level = "";  // the setting of pin, parsed out from inputString
        String pinStr = "";   // pin number (str), parsed out from inputString
        String tempPinStr = "";  // used to extract multiple pins from pinStr
        bool pwmMode = false;   //  true if level is a pwm number (0-255)
        bool lowMode = false;   //  true if level is LOW
        bool highMode = false;  //  true if level is HIGH
        bool inputMode = false;  //  true if level is INPUT
        bool readMode = false;  //  true if mode is read
        bool keepReading = inputString.equals("read all continuous");
        bool readContinuousMode = false;
        char strOut[100] = "";  // used for formated string outputs
        char isOutput;  // used to check pin status
        char type = 'D';    // used to indicate pin type, for function args
        upNdx = 0;
        downNdx = 0;
        if (myVec.size() > 10) {
            myVec.flushFirst();
        }

        // formats inputString to make it easier to parse
        inputString.trim();
        inputString.replace(", ", ",");

        // breaks the string into pin# & level using whitespace as a delimiter
        int i = 0;
        for (; i < inputString.length(); i++) {

            // ignore extra whitespace
            indexEnd = i + 1;
            if (inputString.substring(i, i + 1) == " ") {

                indexEnd = i;
                while (inputString[i] == ' ') {

                    i++;

                }
                break;
            }
        }
        // extract until first whitesapce (usually pin no eg. d3)
        pinStr = inputString.substring(0, indexEnd);
        // estract pin number and convert to int
        pin = pinStr.substring(1, indexEnd).toInt();
        // extract level/mode
        level = inputString.substring(i);



        tempPinStr = pinStr;
        tempPinStr.trim();
        tempPinStr += ",";

        // used to index str to parse out multiple pins seperated by ','
        //(eg. d2,d3,d4)
        int iEnd = 0;
        int iStart = 0;


        // the whole code is in a while loop to handle multiple pins
        do {

            iStart = iEnd;
            // parses the string using ',' as a delimiter
            while (tempPinStr[iEnd] != ',') {
                iEnd++;
            }
            // extracts individual pins from multiple pins
            pinStr = tempPinStr.substring(iStart, iEnd);
            pin = tempPinStr.substring(iStart + 1, iEnd).toInt();

            iEnd++;


            // Converts string to int. If invalid, returns 0
            int pwm = level.toInt();

            pwmMode = (pwm >= 1 && pwm <= 255) || level.equals("0");
            lowMode = level.equals("low");
            highMode = level.equals("high");
            inputMode = level.equals("input");
            readMode = level.equals("read");
            readContinuousMode = level.equals("read continuous");

            // if all is used instead of pin nos
            if (pinStr.equals("all") && (level.equals("high") ||
                level.equals("low") || level.equals("input"))) {

                if (level.equals("input")) {

                    reset();

                } else {

                    setAll(level);
                    Serial.print(F("\n\rAll pins set to "));
                    Serial.print(level);

                }
                break;

            } else if(pinStr.equals("all")) {

                Serial.print(F("\n\rError: Enter a valid setting"
                    " (high, low, input)"));
                break;

            }





            // pins starting with d are digtal, starting with a are analog
            switch (pinStr[0]) {
                case 'd':
                if (pin >= 2 && pin <= 12) {

                    if (lowMode) {

                        // sets pin to low
                        setToLow(pin, 'D');
                        pwmPinStatus[pin] = 0;

                    } else if (highMode) {

                        // sets pin to high
                        setToHigh(pin, 'D');
                        pwmPinStatus[pin] = 255;

                    } else if (pwmMode) {

                        // checks for appropriate pin and sets in the PWM value
                        if (pin == 3 || pin == 5 || pin == 6 || pin == 9 ||
                            pin == 10 || pin == 11) {

                            pinMode(pin, OUTPUT);
                            analogWrite(pin, pwm);
                            pwmPinStatus[pin] = pwm;
                            sprintf(strOut, "\n\rpin D%d set to PWM value: %d",
                                pin, pwm);
                            Serial.print(strOut);

                        } else {

                            sprintf(strOut, "\n\rpin D%d does not support PWM"
                                " outputs", pin);
                            Serial.print(strOut);

                        }

                    } else if (inputMode) {

                        // reads in digital values from the pin
                        pinMode(pin, INPUT);
                        sprintf(strOut, "\n\rPin %d set to INPUT", pin);
                        Serial.print(strOut);

                    } else if (readMode || readContinuousMode) {

                        isOutput = (DDRD >> pin) & 1;
                        if (pin > 7) {

                            isOutput = (DDRB >> (pin - 8)) & 1;

                        }

                        //if pin was not set to input, set to input then read
                        if (isOutput) {

                            pinMode(pin, INPUT);
                            sprintf(strOut, "\n\rpin D%d set to INPUT", pin);
                            Serial.print(strOut);

                        }

                        delay(10);

                        do {

                            readInput(pin, 'd');
                            if (readContinuousMode){

                                // delay by 10th of a second
                                delay(100);

                            }

                            // break out if user inputs a character
                            if (Serial.available()){
                                Serial.read();
                                break;
                            }

                        // if read continuous, loop again
                        } while(readContinuousMode);


                    } else {

                        Serial.print(F("\n\rError: Enter a valid"
                            " setting (high, low, input, read, read continuous"
                            " or 0-255) "));
                    }

                } else if ((level.equals("low") || level.equals("high") ||
                    level.equals("input")) || (pwm >= 1 && pwm <= 255) ||
                    level.equals("0")) {

                    Serial.print(F("\n\rError: pin does not exist. Enter a valid"
                        " pin (digital: d2-d12, analog: a0-a7)"));

                } else {

                    Serial.print(F("\r\nError: Invalid input"));

                }

                break;

                // handles pins starting with a (analog pins)
                case 'a':

                if ((pin >= 1 && pin <= 7) || pinStr.equals("a0")) {

                    if (lowMode) {

                        // sets pin to low. A6 and A7 are not supported
                        if (pin != 6 && pin != 7) {

                            setToLow(pin + 14, 'A');

                        } else {

                            Serial.print(F("\n\rError: Pins A6 and A7 can't be"
                                " used as digital pins in the Arduino Nano"));

                        }

                    } else if (highMode) {

                        // sets pin to high. A6 and A7 are not supported
                        if (pin != 6 && pin != 7) {

                            setToHigh(pin + 14, 'A');

                        } else {

                            Serial.print(F("\n\rError: Pins A6 and A7 can't be"
                                " used as digital pins in the Arduino Nano"));

                        }

                    } else if (inputMode) {

                        // reads in analog values from the pin
                        pinMode(pin + 14, INPUT);
                        sprintf(strOut, "\n\rPin %d set to input", pin);
                        Serial.print(strOut);

                    } else if (readMode || readContinuousMode) {

                        //if pin was not set to input, set to input then read

                        if ((DDRC >> pin) & 1) {

                            pinMode(pin + 14, INPUT);
                            sprintf(strOut, "\n\rpin A%d set to INPUT", pin);
                            Serial.print(strOut);
                        }

                        delay(10);

                        do {

                            readInput(pin + 14, 'A');
                            if (readContinuousMode){
                                delay(100);
                            }

                            if (Serial.available()){
                                Serial.read();
                                break;
                            }
                        // if read continuous, loop again
                        } while(readContinuousMode);


                    } else if (pwmMode) {

                        Serial.print(F("\n\rError: Analog pins cannot be set to"
                            " output PWM signals"));

                    } else {

                        Serial.print(F("\n\rError: Enter a valid setting "
                        "(high, low, input, read, read continuous)"));
                    }

                } else if ((level.equals("low") || level.equals("high") ||
                    level.equals("input")) || (pwm >= 1 && pwm <= 255) ||
                    level.equals("0")) {

                    Serial.print(F("\n\rError: pin does not exist. Input a valid"
                        " pin (digital: d2-d12, analog: a0-a7)"));

                } else {

                    Serial.print(F("\r\nError: Invalid input"));

                }

                break;


                default:

                // clear command
                if (inputString.equals("clear")) {

                    Serial.write(27);     //Print "esc"
                    Serial.print(F("[2J"));  // clear screen
                    Serial.write(27);       //Print "esc"
                    Serial.print(F("[;H"));  // move cursor to home


                //if command is status, print the status of all pins
                } else if (inputString.equals("status")) {

                    // status of digital pins
                    int val = 0;

                    for (int i = 2; i < 22; i++) {

                        // pin 13 is not used
                        if (i == 13)
                            continue;

                        // check if pin is set to OUTPUT
                        isOutput = (DDRD >> i) & 1;
                        val = (PIND >> i) & 1;

                        if (i > 7) {
                            isOutput = (DDRB >> (i-8)) & 1;
                            val = (PINB >> (i - 8)) & 1;
                        }

                        if (i > 13) {
                            isOutput = (DDRC >> (i-14)) & 1;
                            val = (PINC >> (i - 14)) & 1;
                            type = 'A';

                        }

                        // check if pin is LOW, HIGH or INPUT
                        if (isOutput) {

                            //retrieve pwm values if any
                            if (i ==3 || i== 5 || i == 6 || i == 9 || i== 10 ||
                                i == 11) {

                                    val = pwmPinStatus[i];

                            }
                            // print HIGH
                            if (val == 1 || val == 255) {

                                sprintf(strOut, "\n\rpin %c%-2d : HIGH", type,
                                i % 14);
                                Serial.print(strOut);

                            // print LOW
                            } else if (val == 0) {

                                sprintf(strOut, "\n\rpin %c%-2d : LOW", type,
                                 i % 14);
                                Serial.print(strOut);

                            // print pwm value
                            } else {

                                sprintf(strOut, "\n\rpin %c%-2d : %d ", type,
                                i % 14, val);
                                Serial.print(strOut);

                            }

                        } else {

                            sprintf(strOut, "\n\rpin %c%-2d : INPUT",
                                type, i % 14);
                            Serial.print(strOut);

                        }
                    }




                // help command
                } else if (inputString.equals("?") || inputString.equals("help")
                    || inputString.equals("-h")) {



                    Serial.print(F("\n\n\r===========================Simple CLI"
                        "======================"));
                    Serial.print(F("\n\n\rCommands:"));
                    Serial.print(F("\n\rPin: d2-d12 or a0-17\n\rmode: 'high'"
                        " 'low', 'input', 'read' or 'read continuous'"));
                    Serial.print(F("\n\n\r1. <pin> <mode>: controls GPIO"));
                    Serial.print(F("\n\r2. all <'high', 'low', 'input'>:"
                        " controls GPIO"));
                    Serial.print(F("\n\r3. status: displays pin setting"));
                    Serial.print(F("\n\r4. clear: clears screen"));
                    Serial.print(F("\n\r5. reset: sets all pins to 'input'"));
                    Serial.print(F("\n\r6. read all: reads values from all pins"
                        " set to input"));
                    Serial.print(F("\n\r7. read all continuous: reads until"
                    " serial input"));
                    Serial.print(F("\n\r8. <decimal number>:"
                    " converts number (0-127) to binary outputs\n\r"));


                // decimal to binary outputs
                } else if (isNum(inputString) && (inputString.equals("0")
                    || (inputString.toInt() >= 1 &&
                    inputString.toInt() <= 127))) {

                    int i = 2;
                    pin = inputString.toInt();

                    // converts number to binary outputs & sets pins accordigly
                    for (; i <= 8; i++) {

                        pinMode(i, OUTPUT);

                        if (pin & 1) {

                            setToHigh(i, 'D');
                            pwmPinStatus[i] = 255;

                        } else {

                            setToLow(i, 'D');
                            pwmPinStatus[i] = 0;

                        }

                        pin = pin >> 1;

                    }

                // read all command
                } else if (inputString.equals("read all") || keepReading) {

                    type = 'D';

                    do {

                        for (int i = 2; i <= 21; i++) {

                            // pin 13: forbidden!
                            if (i == 13)
                                continue;

                            isOutput = (DDRD >> i) & 1;

                            if (i > 7) {
                                isOutput = (DDRB >> (i-8)) & 1;
                            }

                            if (i > 13) {
                                isOutput = (DDRC >> (i-14)) & 1;
                                type = 'A';
                            }

                            if (!(isOutput)) {

                                readInput(i, type);

                            }


                        }



                        if (Serial.available()) {
                            keepReading = false;
                            //flush the buffer
                            Serial.read();
                        }

                        if (keepReading)
                            delay(100);


                    } while (keepReading);

                } else if (inputString.equals("reset")) {

                    reset();

                } else if (inputString.length() > 0) {

                    Serial.print(F("\n\rError: Invalid input"));

                }


            }

        }  while ((iEnd < tempPinStr.length()));




        // End of GPIO code
        if (!(inputString.equals("clear"))) {
            Serial.print('\n');
        }
        Serial.print('\r');
        Serial.print(F("GPIO: "));
        inputString = "";

    }
}

/**
    SerialEvent is called when data is available at the end of loop()
*/

void serialEvent() {


    while (Serial.available()) {
        // get the new byte
        char inChar = (char)Serial.read();
        // filter characters
        bool arrowKey = inChar == LEFT_ARROW || inChar == RIGHT_ARROW ||
            inChar == UP_ARROW || inChar == DOWN_ARROW;
        bool validChar = (inChar >= 'a' && inChar <= 'z') ||
        (inChar >= '0' && inChar <= '9') || inChar == ' ' || arrowKey ||
        inChar == ',' || inChar == '(' || inChar == ')' || inChar == '-' ||
        inChar == '?';




        if (inChar != '\b' && validChar) {

            if (!(arrowKey)) {

                // add char to inputString

                if (count == inputString.length()) {
                    inputString += inChar;
                    Serial.print(inChar);
                    count++;

                } else {
                    Serial.print(inChar);

                    // save cursor position
                    Serial.write(27);
                    Serial.print("[s");

                    String tempStr = inputString.substring(count);
                    Serial.print(tempStr);
                    inputString = inputString.substring(0, count) + inChar
                        + tempStr;

                    //restore cursor position
                    Serial.write(27);
                    Serial.print("[u");
                    count++;


                }

            } else if (inChar == LEFT_ARROW && count > 0) {

                Serial.write(27);
                Serial.print("[");
                Serial.print(inChar);
                count--;

            } else if (inChar == RIGHT_ARROW && count < inputString.length()) {
                Serial.write(27);
                Serial.print("[");
                Serial.print(inChar);
                count++;

            } else if (inChar == UP_ARROW && myVec.size() > 0) {

                if (myVec[myVec.size() - 1 - upNdx] != "0") {
                    inputString = myVec[myVec.size() - 1 - upNdx] ;



                    Serial.print("\r");
                    Serial.write(27);
                    Serial.print('[');
                    Serial.print('J');
                    Serial.print("GPIO: ");
                    Serial.print(inputString);

                    downNdx = upNdx;
                    upNdx++;
                    count = inputString.length();


                }

            } else if (inChar == DOWN_ARROW && myVec.size() > 0) {

                if (myVec[myVec.size() - downNdx] != "0") {
                    inputString = myVec[myVec.size() - downNdx] ;



                    Serial.print("\r");
                    Serial.write(27);
                    Serial.print('[');
                    Serial.print('J');
                    Serial.print("GPIO: ");
                    Serial.print(inputString);

                    upNdx = downNdx;
                    downNdx--;
                    count = inputString.length();
                }

            }




        } else if (inChar == '\b' && count > 0) {
            // echo backspace on serial monitor
            Serial.print(F("\b"));
            //count--;
            // remove char from inputString

            count--;
            inputString.remove(count, 1);

            Serial.write(27);
            Serial.print('[');
            Serial.print('J');

            // save cursor position
            Serial.write(27);
            Serial.print("[s");

            Serial.print(inputString.substring(count));

            // resotore cursor position
            Serial.write(27);
            Serial.print("[u");

        // if the incoming character is a newline, set a flag
        // so the main loop can do something about it:
        }

        if (inChar == '\r') {

            if (inputString.length() > 0) {

                myVec.insert(inputString);

            }
            stringComplete = true;
        }
    }
}
