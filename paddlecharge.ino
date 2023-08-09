#include "j1850vpw.h"

#define TX 11
#define RX 10
#define CPCT 13 // when this goes high, it pulls the yellow J-clip up to +12v

void handleError(J1850_Operations op, J1850_ERRORS err);
J1850VPW vpw;

void setup()
{
    pinMode(CPCT, OUTPUT); // it's connected to a 2N7000 so we need to control the gate, there is no pulldown resistor
    Serial.begin(115200);           // start serial port
    Serial.println("paddlecharge.ino");
    vpw.onError(handleError); // listen for errors
    vpw.init(RX, TX);         // init transceiver
    Serial.setTimeout(1000); // default timeout for Serial.parseInt() is 1000 ms
    vpw.setActiveLevel(LOW); // the default is LOW
}

void loop()
{
    static uint8_t buff[BS];                   // buffer for read message

    uint8_t dataSize; // size of message

    // read all messages with valid CRC from cache
    while (dataSize = vpw.tryGetReceivedFrame(buff))
    {
        String s;

        // convert to hex string
        uint8_t *pData = buff;
        for (int i = 0; i < dataSize; i++)
        {
            if (i > 0)
            {
                s += " ";
            }

            if (buff[i] < 0x10)
            {
                s += '0';
            }

            s += String(pData[i], HEX);
        }

        Serial.println(String(millis())+"	"+s);
    }
    int16_t commandCode = Serial.parseInt(); // read a number from the serial port
    if (commandCode == 12) {
      Serial.println("12: turning on CPCT yellow J-clip to 12v");
      digitalWrite(CPCT, HIGH);
    }
    if (commandCode == 10) {
      Serial.println("10: SHUTTING OFF CPCT yellow J-clip to 12v");
      digitalWrite(CPCT, LOW);
    }
    if (commandCode == 1) {
      Serial.println("1: sending BUS WAKEUP, CHARGER WAKEUP messages");
      static uint8_t bus_wakeup[4] = {0x88, 0xFE, 0xF2, 0x82}; // message to send, CRC will be read and appended to frame on the fly
      vpw.send(bus_wakeup, 4);
      delay(154);
      static uint8_t charger_status[5] = {0x88, 0x73, 0xF2, 0x23, 0x00};
      vpw.send(charger_status, 5);
    }
    if (commandCode == 2) {
      Serial.println("2: sending BUS WAKEUP, CHARGER INIT messages");
      static uint8_t bus_wakeup[4] = {0x88, 0xFE, 0xF2, 0x82}; // message to send, CRC will be read and appended to frame on the fly
      vpw.send(bus_wakeup, 4);
      delay(154);
      static uint8_t charger_status[5] = {0x88, 0x73, 0xF2, 0x23, 0x01};
      vpw.send(charger_status, 5);
    }
    if (commandCode == 3) {
      Serial.println("3: sending CHARGER INIT, CHARGER CAPABILITY messages");
      static uint8_t charger_status[5] = {0x88, 0x73, 0xF2, 0x23, 0x01};
      vpw.send(charger_status, 5);
      delay(154);
      static uint8_t charger_capability[8] = {0xA8, 0x73, 0xF2, 0x24, 0x02, 0x58, 0x00, 0x00}; // 6000 watts
      vpw.send(charger_capability, 8);
    }

// delay(1000);
}


void handleError(J1850_Operations op, J1850_ERRORS err)
{
    if (err == J1850_OK)
    {
        // skip non errors if any
        return;
    }

    String s = String(millis()) +"	"+ (op == J1850_Read ? "READ " : "WRITE ");
    switch (err)
    {
    case J1850_ERR_BUS_IS_BUSY:
        Serial.println(s + "J1850_ERR_BUS_IS_BUSY");
        break;
    case J1850_ERR_BUS_ERROR:
        Serial.println(s + "J1850_ERR_BUS_ERROR");
        break;
    case J1850_ERR_RECV_NOT_CONFIGURATED:
        Serial.println(s + "J1850_ERR_RECV_NOT_CONFIGURATED");
        break;
    case J1850_ERR_PULSE_TOO_SHORT:
        Serial.println(s + "J1850_ERR_PULSE_TOO_SHORT");
        break;
    case J1850_ERR_PULSE_OUTSIDE_FRAME:
        Serial.println(s + "J1850_ERR_PULSE_OUTSIDE_FRAME");
        break;
    case J1850_ERR_ARBITRATION_LOST:
        Serial.println(s + "J1850_ERR_ARBITRATION_LOST");
        break;
    case J1850_ERR_PULSE_TOO_LONG:
        Serial.println(s + "J1850_ERR_PULSE_TOO_LONG");
        break;
    case J1850_ERR_IFR_RX_NOT_SUPPORTED:
        Serial.println(s + "J1850_ERR_IFR_RX_NOT_SUPPORTED");
        break;
    default:
        // unknown error
        Serial.println(s + "ERR: " + String(err, HEX));
        break;
    }
}
