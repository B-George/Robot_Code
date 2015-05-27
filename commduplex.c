#include <stdio.h>
#include <wiringPi.h>
#include <sys/types.h>
#include <unistd.h>

/***********************************************************************************************************************************/
/*
/* commduplex.c  Pseudocode
/*
/* configure wiringPi for one GPIO pin output and one GPIO pin input
/* do forever
/*    wait 100 mSec
/*    set output pin to high for 10 mSec (start pulse)
/*    parity = 0
/*    for each bit in the letter 'A' in the order bits 7 downto 0
/*       extract bit from letter
/*       set output pin to the bit value
/*       add bit to parity
/*       wait 5 mSec
/*    end for
/*    if parity is odd
/*       set output pin to '1'
/*    else
/*       set output pin to '0'
/*    wait for 5 mSec
/*    set output pin to '0'
/* end do forever
/*
/* do forever
/*    wait until input pin = '1'
/*    wait 2 mSec
/*    if input pin is still '1'
/*       wait 5 mSec
/*       if input pin is still '1'
/*          for 8 bits in the order bits 7 downto 0
/*             wait 5 mSec
/*             read input bit into letter
/*	       add bit to parity
/*          input pin is received parity
/*          if received parity = calculated parity
/*             print letter
/*          else
/*             print error message
/* end do forever
/*
/*************************************************************************************************************/

int main(void)
{
        char letter = 0x41; /* the ASCII for the letter 'A' */
        char parityt, parityr, bit;
        int i;
        pid_t parent_pid, child_pid;

        parent_pid = getpid();

        wiringPiSetup();

        pinMode(0,OUTPUT);  /* This will drive the infrared transmitter. (Not
                               directly, some sort of transistor for current
                               gain needed.)                                  */
        pinMode(2,INPUT);   /* This will read the infrared receiver.          */
        pullUpDnControl(2,PUD_UP);      /* Scope showed weak signal.  Maybe needed. */

        child_pid = fork();
        if (child_pid == 0)
        {  /* we are child.  We transmit  */
           child_pid = getpid();
           while(1)
           {
                delay(100);     /* slow the rate of letter transmission */
                digitalWrite(0,HIGH);   /* digitalWrite maintains this value
                                           at the output until changed by a
                                           subsequent write                     */
                delay(10);
                parityt = 0;            /* used to count the ones               */
                for (i=0; i<8; i++)
                {
                        bit = (letter >> (7-i)) & 0x01;
                        digitalWrite(0,bit);
                        parityt = parityt + bit;
                        delay(5);
                }
                digitalWrite(0, (parityt & 0x01));      /* if parity is an odd
                                                           number, bit 0 is a 1
                                                           and a 1 is sent.
                                                           Otherwise a 0 is sent */
                delay(5);
                digitalWrite(0, LOW);
           }
        }
        else
        {       /* the parent reeives  */
                while(1)
                {
                        while (digitalRead(2) == LOW)
                        {
                                delay(1);
                        }
                        /* Escaped the while with a high.  Could be start bit  */
                        delay(2);       /* should put us 2-3 mSecs into a start  */
                        if (digitalRead(2) )
                        {       /* still a 1  */
                                delay(5);       /* should put us 7-8 mSecs in  */
                                if (digitalRead(2))
                                {       /* 3 1's in a row. Start bit found.  */
                                        parityr = 0;
                                        for (i=0; i<8; i++)
                                        {
                                                delay(5);       /* wait for next pulse */
                                                bit = digitalRead(2);
                                                parityr = parityr + bit;
                                                letter = (letter << 1) | (bit & 0x01);
                                        }
                                        delay(5);
                                        if  ( (parityr  & 0x01) == digitalRead(2) )
                                                printf("Character received is %c.\n", letter);
                                        else
                                                printf("Parity error encountered. Sorry, dude.\n");
                                }
                                else break;
                        }
                } /* end of while(1)  */
        }
return 0;
}