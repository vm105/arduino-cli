"""
Created on June 22 2016
by Varun Malhotra

"""

from echostar_automation import *
from time import sleep


class ArduinoGPIO(DUT):
    PROMPT = "GPIO: "

    def __init__(self, host: str, debug_in):
        """
        Connects to the device
        :param host: The device's com port
        :param debug_in: A Debug object or Debug.decoy
        :raises ReferenceError: Wrong device connected
        """
        global debug
        debug = debug_in

        super().__init__(host=host, serial_baud=115200, debug_in=debug)

        debug("Establishing connection to Arduino Nano..")

        self.read_until(ArduinoGPIO.PROMPT)

        debug("Connection to Arduino Nano  established", force=True)

    #  Lower Level Functions
    # ------------------------------------------------------------------------------------------------------------------

    def _send_cmd(self, cmd: str) -> str:
        """
        Sends a command to the Arduino and gets its response
        :param cmd: The command to send
        :return: The switch's response
        """
        cmd = cmd.lower()
        debug("Sending '%s'" % cmd)
        self.write(cmd + '\r')
        resp = self.read_until(ArduinoGPIO.PROMPT, timeout=5)
        debug("Serial Response:\n %s" % resp)

        if "Error" in resp:
            raise Exception(resp)
        sleep(.2)  # Necessary for consecutive commands
        return resp

    def _set_pin(self, pin: str, mode: str = "input") -> str:
        """
        Sets Arduino pin to the desired mode
        :param pin: The pin number
        :param mode: The pin setting
        :return: Response from the Arduino

        """
        pin = pin.lower()
        mode = mode.lower()

        debug("Setting %s to %s" % (pin, mode))

        temp_pins = pin.split(",")
        for pin_no in temp_pins:

            if pin_no[0] != 'd' and pin_no[0] != "a":
                raise Exception(" %s not a valid pin" % pin)

            if (pin_no is "a6" or pin_no is "a7") and (mode is "low" or mode is "high"):
                raise Exception("%s doesn't support digital outputs")

        resp = self._send_cmd("%s %s" % (pin, mode))
        return resp

    def _set_all(self, mode="input"):
        """
        Sets All Arduino pins to the desired mode
        :param pin: The pin setting

        """
        mode = mode.lower()
        self._send_cmd("all %s" % mode)

    # Higher Level Functions
    # ------------------------------------------------------------------------------------------------------------------

    def set_high(self, pin: str):
        """
        Sets  Arduino pin to high
        :param pin: The pin number

        """

        self._set_pin(pin=pin, mode="high")
        debug("%s set to high" % pin)

    def set_low(self, pin: str):
        """
        Sets  Arduino pin to low
        :param pin: The pin number

        """
        self._set_pin(pin=pin, mode="low")
        debug("%s set to low" % pin)

    def set_to_input(self, pin: str, ):
        """
        Sets Arduino pin to input
        :param pin: The pin number

        """

        self._set_pin(pin=pin, mode="input")
        debug("%s set to input" % pin)

    def get_input(self, pin: str) -> str:
        """
        Reads in pin input
        :param pin: The pin to be read
        :return: Pin input

        """

        resp = self._set_pin(pin=pin, mode="read")
        resp = resp.split("GPIO:")[0].split(":")[1].strip()
        return (resp)

    def set_all_high(self):
        """
        Sets All pins to high

        """
        self._set_all(mode="high")
        debug("All pins set to high")

    def set_all_low(self):
        """
        Sets All pins to low

        """
        self._set_all(mode="low")
        debug("All pins set to low")

    def set_all_to_input(self):
        """
        Sets All pins to input

        """
        self._set_all()
        debug("All pins set to input")

    def get_all_status(self) -> str:
        """
        Gets pin status for all pins
        :return pin status

        """
        resp = self._send_cmd("status")
        return resp

    def reset_all(self):
        """
        Resets All pins to input

        """

        self._send_cmd("reset")
        debug("Pins reset to input")

    def set_to_pwm(self, pin: str, pwm_value: int = 0):
        """
        Sets pin to pwm value
        :param pin: The pin number to
        :param pwm_value: The PWM value to set the pin to

        """

        if pwm_value > 255 or pwm_value < 0:
            raise Exception("PWM exceeds default range (0-255)")

        temp_pins = pin.split(",")

        for pin_no in temp_pins:

            pin_no = int(pin_no[1:])
            if pin_no != 3 and pin_no != 5 and pin_no != 6 and pin_no != 9 and pin_no != 10 and pin_no != 11:
                raise Exception("Invalid PWM pin")

        self._set_pin(pin=pin, mode=str(pwm_value))
        debug("%s set to %d" % (pin, pwm_value))

    def set_to_binary_output(self, val: int):
        """
        Sets pins to represent binary outputs (eg. 5-> 101, will set d2 & d4 high)
        :param val: Decimal input to be represented in binary form

        """

        if val < 0 or val > 127:
            raise Exception("%d is not in range 0-127" % val)

        self._send_cmd(str(val))
        debug("Pins set to binary output %d" % val)


# ======================================================================================================================

if __name__ == "__main__":

    # demo
    debug = Debug(0)
    my_arduino = ArduinoGPIO("COM4", debug_in=debug)
    my_arduino.set_high(pin="d3,d2")
    my_arduino.set_to_pwm(pin="d3,d5", pwm_value=255)
    my_arduino.set_all_to_input()
    my_arduino.reset_all()
    my_arduino.set_to_binary_output(5)
    print(my_arduino.get_all_status())

    for x in range(5):
        print(my_arduino.get_input(pin="a3"))
