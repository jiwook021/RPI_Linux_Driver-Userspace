#include <stdio.h>
#include <wiringPi.h>

// LED Pin - wiringPi pin 0 is BCM_GPIO 17.


#define	BUTTON	0

int main (void)
{
  printf ("Raspberry Pi blink\n") ;

  wiringPiSetup () ;
  pinMode (BUTTON, INPUT) ;

  for (;;)
  {
    int in_data = digitalRead(BUTTON) ;
    delay (100) ;		// mS
    if (in_data == 0)
    {
        printf("0\n");
    }
    else
    {
        printf("1\n");
    }

  }
  return 0 ;
}