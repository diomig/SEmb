#include <msp430g2553.h>


void initSpeaker(void);
void setSpeaker(int period);
void initMotorControl(void);
void setMotorDutyCycle(int duty_cycle);
void initSPI();
int SPIcomm(int data);


int main(void) {

    WDTCTL = WDTPW + WDTHOLD; //Disable the Watchdog timer for our convenience.

    /*
    int period=2000; //freq = 500Hz

    initSpeaker();

    while(1){
    setSpeaker(period);
    _delay_cycles(1000000);
    setSpeaker(0);
    _delay_cycles(1000000);
    }
*/

}



void initSpeaker(void){

    P1DIR |= BIT2; //Set pin 1.2 to the output direction.
    P1SEL |= BIT2; //Select pin 1.2 as our PWM output.
}


void setSpeaker(int period) {
    TA0CCR0 = period; //Set the period in the Timer A0 Capture/Compare 0 register to 1000 us.
    TA0CCTL1 = OUTMOD_7;
    TA0CCR1 = period>>1; //The period in microseconds that the power is ON. It's half the time, which translates to a 50% duty cycle.
    TA0CTL = TASSEL_2 + MC_1; //TASSEL_2 selects SMCLK as the clock source, and MC_1 tells it to count up to the value in TA0CCR0.
    //__bis_SR_register(LPM0_bits); //Switch to low power mode 0.
}


void initMotorControl(void){
    P2DIR |= BIT1; //Set pin 2.1 to the output direction.
    P2SEL |= BIT1; //Select pin 2.1 as our PWM output.
}


void setMotorDutyCycle(int duty_cycle){
    int period = 80000;
    int timeHigh = (period * duty_cycle) / 100;
    TA1CCR0 |= period - 1;
    TA1CCTL1 |= OUTMOD_7;
    TA1CCR1 |= timeHigh;
    TA1CTL |= TASSEL_2 + MC_1;
}



void initSPI(){
    P1OUT |= BIT5;
    P1DIR |= BIT5;
    P1SEL = BIT1 | BIT2 | BIT4;
    P1SEL2 = BIT1 | BIT2 | BIT4;

    UCA0CTL1 = UCSWRST;
    UCA0CTL0 |= UCCKPH + UCMSB + UCMST + UCSYNC; // 3-pin, 8-bit SPI master
    UCA0CTL1 |= UCSSEL_2; // SMCLK
    UCA0BR0 |= 0x02; // prescaler for baud-rate
    UCA0BR1 = 0; //
    UCA0MCTL = 0; // No modulation
    UCA0CTL1 &= ~UCSWRST; // **Initialize USCI state machine**
}


int SPIcomm(int data){
    int received_ch;

    P1OUT &= (~BIT5); // Select Device

    while (!(IFG2 & UCA0TXIFG)); // USCI_A0 TX buffer ready?
    UCA0TXBUF = data; // Send 0xAA over SPI to Slave
    while (!(IFG2 & UCA0RXIFG)); // USCI_A0 RX Received?
    received_ch = UCA0RXBUF; // Store received data

    P1OUT |= (BIT5); // Unselect Device

    return received_ch;
}
