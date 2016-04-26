#include <stdbool.h>

#include <msp430.h>

#include "gpio.h"
#include "state.h"

// Global variables
// Raw ADC10 value from the most recent conversion
static volatile unsigned int gTemp = 0;

// Total number of seconds
static volatile unsigned int gTimerTicks = 0;

// Keep track of which operations need to be done while in the main loop
static volatile char gEvents = 0;

// Various event bits, used in main event handling loop.
enum event {
  kADCMathPending = 1 << 0,
  kTATick = 1 << 1,

  // Makes it easy to check for any pending event or no pending events.
  kNoEvents = 0x00,
  kAllEvents = 0xff,
};

// Returns true if the events mask contains the event e.
static inline bool check_events(enum event events, enum event e) {
  return (events & e) != 0;
}

// Returns whether or not the given event bit is set at the time of function
// call.
static inline enum event check_event() {
  return gEvents;
}

// Sets the given event bit safely from outside interrupt context.
static inline void event_set(enum event e) {
  __disable_interrupt();
  gEvents |= e;
  __enable_interrupt();
}

// Clear the given event bit safely from outside interrupt context.
static inline void event_clear(enum event e) {
  __disable_interrupt();
  gEvents &= ~e;
  __enable_interrupt();
}

int main(void) {
  // The temperature reading from the external temperature sensor (after
  // conversion from ADC counts)
  unsigned int temperature = 25;
  unsigned int timeLimit = 480;
  
  // Stop watchdog timer to prevent time out reset
  WDTCTL = WDTPW + WDTHOLD;  
  
  // Disable interrupts for the setup
  //__disable_interrupt();

  // Enable the crystal osc fault. The xtal startup process
  IE1 |= OFIE;     // An immedate Osc Fault will occur next

  BCSCTL1 |= XT2OFF; //Turn off XT2, it is not used for MCLK or SMCLK
  BCSCTL2 &= RSEL0;  
  BCSCTL2 |= SELM_2 + SELS; //XT2CLK for MCLK and SMCLK
  BCSCTL3 |= XCAP_1;
  
  gpio_setup();

  // Set up the timer A - Use the system master crystal and have it count up
  // With this crystal TACCR will be 32768
  TACTL = TACLR | TASSEL0; // Clear timer A, Select clock source 0 (LFXTAL)
  TACCR0 = 32768;          // One Second PLZ! 
  
  // Setting up the ADC, will make some changes later
  ADC10CTL0 = REFON | REFBURST | ADC10SR | ADC10SHT0 | ADC10SHT1 | SREF0 ;
  ADC10CTL1 = 0x0000; // THIS SETS INCHx TO A0
  ADC10AE0 = BIT0; //ENABLE THE 0 PIN FOR ADC USE

  //Setup is done------------------------
  
  //Enable interrupts now that setup is done
  __enable_interrupt();
  
  // Enable the compare ISR on TimerA CCR0
  TACCTL0 |= CCIE;

  // Enable the compare ISR on TimerA CCR1
  TACCTL1 |= CCIE;
  
  //Starting the timer in up mode
  TACTL |= MC0;   // going into up mode starts the (one second) timer!
  
  // ADC STUFF---------------------------
  // Turn on the ADC and ref voltage
  // this will also take readings and start conversion, ADC interrupts when
  // conversion is done
  ADC10CTL0 |=  ADC10ON + REFON + ENC + ADC10SC;
  
  // Button interrupt enable.
  P1IES |= (1 << 3); // interrupt on low-to-high transition
  P1IE |= (1 << 3);  // interrupt enable

  while(1) {
    char events = check_event();
    if (events == kNoEvents) {
       _BIS_SR(LPM0_bits);
    }
    if (check_event(events, kADCMathPending)) {
      // Add all the conversion math goes here!!
      temperature = gTemp;
      continue;
    }
    if (check_event(events, kTATick)) {
      if (gTimerTicks >= timeLimit) {
        // XXX
      }
      continue;
    }
  }
  return 0;
}

#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void) {
  // Store the ADC result in the global variable gTemp
  gTemp = ADC10MEM;

  // Set this state so that the result can be processed in the main loop
  gEvents |= kADCMathPending;

  // Wake the processor when this ISR exits
  __bic_SR_register_on_exit(LPM0_bits);
}

#pragma vector=TIMERA0_VECTOR
__interrupt void TIMERA0_ISR(void) {
  gTimerTicks++;

  // Start another conversion
  ADC10CTL0 |= ADC10SC;

  // Clear the timer interrupt flag
  TACCTL0 &= ~CCIFG;
  gEvents |= kTATick;

  // Wake the processor when this ISR exits
  __bic_SR_register_on_exit(LPM0_bits);
}

#pragma vector=TIMERA1_VECTOR
__interrupt void TIMERA1_ISR(void) {
  toggle_led();
  switch (raw_current_state()) {
    case kWaitingForShortPress:
      break;
    case kDebouncingShortPress:
      break;
    case kWaitingForLongPress:
      break;
    case kDebouncingLongPress:
      break;
    case kTiming:
      break;
  }
}

#pragma vector=PORT1_VECTOR
__interrupt void port1_isr(void) {
  // Clear interrupt flag.
  P1IFG &= ~(1 << 3);

  switch (raw_current_state()) {
    case kWaitingForShortPress:
      set_state(kDebouncingShortPress);
      break;
    case kDebouncingShortPress:
      break;
    case kWaitingForLongPress:
      set_state(kDebouncingLongPress);
      break;
    case kDebouncingLongPress:
      break;
    case kTiming:
      break;
  }
}

// The LF XTAL error flag is going to get set when the crystal starts up. Catch
// it and clear it here.
// 
// (Warning: We arent checking for any other sources of NMIs!)
#pragma vector=NMI_VECTOR
__interrupt void nmi_(void) {
  unsigned int i;
  do {
    IFG1 &= ~OFIFG;              // Clear OSCFault flag
    for (i = 0xFFF; i > 0; i--); // Time for flag to set
  } while (IFG1 & OFIFG);        // OSCFault flag still set?
  IE1 |= OFIE;                   // Enable Osc Fault
}
