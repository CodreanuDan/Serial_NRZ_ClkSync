/*
 * SSCC - Serial Sync Clock Communication
 * Receiver:
 * - It is using NRZ encoding/decoding: HIGH 5V = 1; LOW 0V = 0
 * - TX rate is set to 5 bits per second for better visualisation on the LED
 * - Interrupt driven RX mechanism 
 */

/* 
 * Includes 
 */
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

/* 
 * Variables 
 */

#define RX_CLOCK 2 /* TX data pin */
#define RX_DATA  3 /* RX data pin */

#define SCL     19 /* LCD SCL pin */
#define SDA     18 /* LCD SDA pin */

#define LCD_ADDR 0x3F
#define LCD_LINES 16u
#define LCD_ROWS 2u

#define MONITOR false
#define PLOTTER true

char message[16];
volatile byte rx_byte;
volatile uint8_t bit_position;
volatile bool update_lcd = true;

/* Reset buffer if no message is received for more than x ms */
volatile unsigned long last_pulse_time = 0;

/* 
 * Function Prototypes
 */

void setup();
void loop();

void SSCC_SetPins(uint8_t data_pin, uint8_t clk_pin);
void SSCC_ReceiveData();
void SSCC_ResetRxBuffer();

void LCD_Init(uint8_t lines, uint8_t rows);
void LCD_Update(const char* msg);

/* Initialize the LCD screen */
LiquidCrystal_I2C lcd(LCD_ADDR, LCD_LINES, LCD_ROWS);

/*********************************************_Main_Functions_*****************************************************/

void setup()
{
    Serial.begin(9600);

    /* Init I2C */
    Wire.begin();

    /* Setup for LCD */
    LCD_Init(LCD_LINES, LCD_ROWS);

    /* Setup for Serial data transmission*/
    SSCC_SetPins(RX_DATA, RX_CLOCK);

    /* Reset message buffer */
    strcpy(message, "");

    /* Set last pulse time for safety reasons */
    last_pulse_time = millis();

}

void loop()
{
    unsigned long now = millis();

    /* Receive data */
    SSCC_ReceiveData();

    /* Trigger buffer reset */
    if (strlen(message) >= 16)
    {
        SSCC_ResetRxBuffer();
    }
    if(now - last_pulse_time > 1500 && (bit_position != 0 || strlen(message) >= 16))
    {
        SSCC_ResetRxBuffer();
    }

    delay(100);
        
}

/********************************************_Component_Functions_**************************************************/

/*
 * RX_DATA ISR
 */
void SSCC_OnClockPulse()
{
    bool rx_bit = digitalRead(RX_DATA);

    /* If byte reading is over */
    if (bit_position == 8)
    {
      rx_byte = 0;
      bit_position = 0;
    }

    /* Compute byte */
    if (rx_bit)
    {
      rx_byte |= (0x80 >> bit_position);
    }

    bit_position += 1;

    /* Concatenate byte to message */
    if (bit_position == 8)
    {
      strncat(message, (const char *)&rx_byte, 1);
    }

    update_lcd = true;

    /******_PLOTTER/MONITOR_VIEW_******/
    if (MONITOR)
    {
        Serial.print("DATA:\t");
        Serial.print(rx_bit);
        Serial.print("\tCLK:\t");
        Serial.println(digitalRead(RX_CLOCK));
    }
    if (PLOTTER)
    {
        Serial.print(rx_bit);       
        Serial.print('\t');         
        Serial.println(digitalRead(RX_CLOCK)); 
    }

    /* Set Last pulse time */
    last_pulse_time = millis();

}

void SSCC_ReceiveData()
{
    if (update_lcd)
    {
        update_lcd = false;

        /* Update LCD */
        LCD_Update(message);

        /* Display data on LCD */
        for(uint8_t i = 0; i < 8; i++)
        {
            if (i < bit_position)
            {
                lcd.print(rx_byte & (0x80 >> i) ? "1" : "0");
            }
            else 
            {
                lcd.print(" ");
            }
        }
        lcd.setCursor(strlen(message), 0);
        lcd.cursor();
    }
}

void SSCC_SetPins(uint8_t data_pin, uint8_t clk_pin)
{
    pinMode(data_pin, INPUT);
    attachInterrupt(digitalPinToInterrupt(clk_pin), SSCC_OnClockPulse, RISING);
}

void SSCC_ResetRxBuffer()
{   
    /* Reset message buffer */
    strcpy(message, "");
    bit_position = 0;
    rx_byte = 0;

    /* Set last pulse time for prevent accidental reset */
    last_pulse_time = millis();

    /* Clear both LCD lines */
    lcd.noCursor();
    lcd.clear();  

    /* Set cursor to start of first line */
    lcd.setCursor(0, 0);
    lcd.cursor();
}

void LCD_Init(uint8_t lines, uint8_t rows)
{
    lcd.begin(lines, rows);
    lcd.backlight(); 
    delay(250);
}

void LCD_Update(const char* msg)
{
    lcd.noCursor();
    lcd.setCursor(0, 0);
    lcd.print(msg);
    lcd.setCursor(0, 1);
}

