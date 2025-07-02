/*
 * SSCC - Serial Sync Clock Communication
 * Transmiter:
 * - It is using NRZ encoding/decoding: HIGH 5V = 1; LOW 0V = 0
 * - TX rate is set to 5 bits per second for better visualisation on the LED
 */

/* 
 * Includes 
 */
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

/* 
 * Variables 
 */

#define TX_RATE  5 /* Transmit rate in bps */ 
#define TX_CLOCK 2 /* TX data pin */
#define TX_DATA  3 /* RX data pin */

#define SCL     19 /* LCD SCL pin */
#define SDA     18 /* LCD SDA pin */

#define LCD_ADDR 0x3F
#define LCD_LINES 16u
#define LCD_ROWS 2u

#define MONITOR false
#define PLOTTER true

const char *message = "Hello, world!"; /* Test message */

/* 
 * Function Prototypes
 */

void setup();
void loop();

void SSCC_SetPins(uint8_t data_pin, uint8_t clk_pin);
void SSCC_SendData(const char *msg, uint8_t tx_rate, uint8_t data_pin, uint8_t clk_pin);
void SSCC_SendByte(byte tx_byte, uint8_t tx_rate, uint8_t data_pin, uint8_t clk_pin);
void SSCC_ClkPulse(uint8_t clk_pin, uint8_t tx_rate, bool bit);

void LCD_Init(const char* msg, uint8_t lines, uint8_t rows);
void LCD_UpdateByte(uint8_t idx);
void LCD_UpdateBits(uint8_t bit, uint8_t bit_idx, uint8_t byte_idx);

/* Initialize the LCD screen */
LiquidCrystal_I2C lcd(LCD_ADDR, LCD_LINES, LCD_ROWS);

/*********************************************_Main_Functions_*****************************************************/

void setup()
{
    Serial.begin(9600);

    /* Init I2C */
    Wire.begin();

    /* Setup for LCD */
    LCD_Init(message, LCD_LINES, LCD_ROWS);

    /* Setup for Serial data transmission*/
    SSCC_SetPins(TX_DATA, TX_CLOCK);

    /* Send data */
    SSCC_SendData(message, TX_RATE, TX_DATA, TX_CLOCK);

}

void loop()
{
  /* Empty loop */
}

/********************************************_Component_Functions_**************************************************/

void SSCC_SetPins(uint8_t data_pin, uint8_t clk_pin)
{
    pinMode(data_pin, OUTPUT);
    pinMode(clk_pin, OUTPUT);
}

void SSCC_SendData(const char *msg, uint8_t tx_rate, uint8_t data_pin, uint8_t clk_pin)
{
    /* Iterate trough each character in the message */
    for(uint8_t byte_idx = 0; byte_idx < strlen(msg); byte_idx++)
    {
        byte tx_byte = msg[byte_idx];

        /* Prepare LCD to display specific byte and each bit aparat */
        LCD_UpdateByte(byte_idx);

        /* Send data byte trough data pin */
        SSCC_SendByte(tx_byte, tx_rate, data_pin, clk_pin, byte_idx);

        /* Stop TX */
        digitalWrite(data_pin, LOW);
    }

}

// void SSCC_SendByte(byte tx_byte, uint8_t tx_rate, uint8_t data_pin, uint8_t clk_pin, uint8_t idx)
// {
//     uint8_t _byte_idx = idx; 
  
//     for(uint8_t bit_idx = 0; bit_idx < 8; bit_idx++)
//     {
//         /* Extract bit at position (7 - bit_idx), MSB first */
//         bool tx_bit = tx_byte & (0x80 >> bit_idx);

//         /* Write data */
//         digitalWrite(TX_DATA, tx_bit);

//         /* 1000 / 5 = 200ms -> Low  -> 100 ms _100ms_| 100ms |*/
//         delay((1000 / TX_RATE) / 2);

//         /* Update the LCD */
//         LCD_UpdateBits(tx_bit, bit_idx, _byte_idx);

//         /* Pulse clock */
//         SSCC_ClkPulse(clk_pin, tx_rate, tx_bit);

//     }
// }

void SSCC_SendByte(byte tx_byte, uint8_t tx_rate, uint8_t data_pin, uint8_t clk_pin, uint8_t idx)
{
    for (uint8_t bit_idx = 0; bit_idx < 8; bit_idx++)
    {
        /* Extract bit at position (7 - bit_idx), MSB first */
        bool tx_bit = tx_byte & (0x80 >> bit_idx);

        //  Set DATA pin
        digitalWrite(data_pin, tx_bit);

        //  Update LCD for visualization
        LCD_UpdateBits(tx_bit, bit_idx, idx);

        //  Immediately pulse clock (rising edge = valid sampling)
        SSCC_ClkPulse(clk_pin, tx_rate, tx_bit);
    }

    // Stop transmission (idle low)
    digitalWrite(data_pin, LOW);
}


// void SSCC_ClkPulse(uint8_t clk_pin, uint8_t tx_rate, bool bit)
// {
//     digitalWrite(clk_pin, HIGH);
//     /******_PLOTTER/MONITOR_VIEW_******/
//     if (MONITOR)
//     {
//         Serial.print("DATA:\t");
//         Serial.print(bit);
//         Serial.print("\tCLK:\t");
//         Serial.println(1);
//     }
//     if (PLOTTER)
//     {
//         uint8_t clk_signal = 1;
//         Serial.print(bit);       
//         Serial.print('\t');         
//         Serial.println(clk_signal); 
//     }
//     /* 1000 / 5 = 200ms -> High -> 100 ms _100ms_| 100ms |*/
//     delay((1000 / tx_rate) / 2);
//     digitalWrite(clk_pin, LOW);
//     /******_PLOTTER/MONITOR_VIEW_******/
//     if (MONITOR)
//     {
//         Serial.print("DATA:\t");
//         Serial.print(bit);
//         Serial.print("\tCLK:\t");
//         Serial.println(0);
//     }
//     if (PLOTTER)
//     {
//         clk_signal = 0;
//         Serial.print(bit);       
//         Serial.print('\t');         
//         Serial.println(clk_signal); 
//     }
// }

void SSCC_ClkPulse(uint8_t clk_pin, uint8_t tx_rate, bool bit)
{
    digitalWrite(clk_pin, HIGH);

    if (MONITOR || PLOTTER)
    {
        Serial.print(bit);
        Serial.print('\t');
        Serial.println(1); // CLK HIGH
    }

    delay((1000 / tx_rate) / 2); // menține CLK HIGH

    digitalWrite(clk_pin, LOW);

    if (MONITOR || PLOTTER)
    {
        Serial.print(bit);
        Serial.print('\t');
        Serial.println(0); // CLK LOW
    }

    delay((1000 / tx_rate) / 2); // menține CLK LOW
}


void LCD_Init(const char* msg, uint8_t lines, uint8_t rows)
{
    lcd.begin(lines, rows);
    lcd.backlight(); 
    delay(250);
    
    lcd.setCursor(0, 0);
    lcd.print(msg);
}

void LCD_UpdateByte(uint8_t idx)
{
    /* Clear the second line of the display */
    lcd.noCursor();
    lcd.setCursor(0, 1);
    lcd.print("        ");  /* 8 spaces - 8 bits */
    lcd.setCursor(idx, 0);
    lcd.cursor();
}

void LCD_UpdateBits(uint8_t bit, uint8_t bit_idx, uint8_t byte_idx)
{
    lcd.noCursor();
    lcd.setCursor(bit_idx, 1);
    lcd.print(bit ? "1" : "0");
    lcd.setCursor(byte_idx, 0);
    lcd.cursor();
}

