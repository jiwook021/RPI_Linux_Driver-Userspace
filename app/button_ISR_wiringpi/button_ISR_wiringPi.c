#include <stdio.h>
#include <wiringPi.h>


#define BUTTON_PIN 0

int count = 0;


void myInt()
{
  printf("haha(%d)\n", count++);
  
}

void isr()
{
    int in_data = digitalRead(BUTTON_PIN) ;
    if (in_data == 0)
    {
        printf("0\n");
    }
    else
    {
        printf("1\n");
    }
}
void setup()
{
  printf ("Raspberry Pi EXTI\n") ;

  pinMode (BUTTON_PIN, INPUT) ;
  //wiringPiISR (BUTTON_PIN, INT_EDGE_RISING, &myInt);
//  wiringPiISR (BUTTON_PIN, INT_EDGE_BOTH, &isr);
   wiringPiISR (BUTTON_PIN, INT_EDGE_FALLING, &isr);
  
}


void loop()
{

}



//=========================================

int main (void)
{
  wiringPiSetup () ;
  setup();

  for (;;)
  {
    loop();
  }
  return 0 ;
}