#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

// --- PORT B JOYSTICK PIN DEFINITIONS ---
#define JOYSTICK_PIN_PORT PINB
#define JOYSTICK_DDR      DDRB
#define JOYSTICK_PORT     PORTB

#define RIGHT_BIT  0  // PB0: Right
#define DOWN_BIT   1  // PB1: Down
#define LEFT_BIT   2  // PB2: Left
#define UP_BIT     3  // PB3: Up
#define CENTER_BIT 4  // PB4: Center Press

volatile char latestCommand = 'S';

// (Using UBRR = 47 for 7.3728MHz copying Lab 5/6)
void UART_init(unsigned int ubrr) {
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;
    UCSR0B = (1 << TXEN0);                  // Enable transmitter
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // 8-bit data, 1 stop bit
}

void UART_transmit(unsigned char data) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = data;
}

int main(void) {
    // 9600 baud
    UART_init(47);

    // Internal Pull-up resistors
    JOYSTICK_DDR  &= ~((1 << RIGHT_BIT) | (1 << DOWN_BIT) | (1 << LEFT_BIT) | (1 << UP_BIT) | (1 << CENTER_BIT));
    JOYSTICK_PORT |=  ((1 << RIGHT_BIT) | (1 << DOWN_BIT) | (1 << LEFT_BIT) | (1 << UP_BIT) | (1 << CENTER_BIT));

    // Enable interrupts for stability
    sei();

    char lastTransmitted = '\0';

    while (1) {
        char currentInput = 'S'; // Default to stop

        // Read active-low buttons from Port B
        if (!(JOYSTICK_PIN_PORT & (1 << UP_BIT))) {
            currentInput = 'F';      // Up -> Forward
        } 
        else if (!(JOYSTICK_PIN_PORT & (1 << DOWN_BIT))) {
            currentInput = 'B';      // Down -> Backward
        } 
        else if (!(JOYSTICK_PIN_PORT & (1 << LEFT_BIT))) {
            currentInput = 'L';      // Left -> Turn Left
        } 
        else if (!(JOYSTICK_PIN_PORT & (1 << RIGHT_BIT))) {
            currentInput = 'R';      // Right -> Turn Right
        } 
        else if (!(JOYSTICK_PIN_PORT & (1 << CENTER_BIT))) {
            currentInput = 'S';      // Center Press -> Stop
        }

        latestCommand = currentInput;

        // Stream over Bluetooth whenever the command changes
        if (latestCommand != lastTransmitted) {
            UART_transmit(latestCommand);
            lastTransmitted = latestCommand;
        }

        _delay_ms(30); // Loop pacing and button debounce delay
    }
}
