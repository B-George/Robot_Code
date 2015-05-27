#include <stdio.h>
#include <wiringPi.h>

/***********************************************************************************************************************************/
/*
/* protocomm.c  Pseudocode
/*
/* configure wiringPi for one GPIO pin output and one GPIO pin input
/* set output pin to high for 10 mSec (start pulse)
/* parity = 0
/* for each bit in the letter 'A' in the order bits 7 downto 0
/*    extract bit from letter
/*    set output pin to the bit value
/*    add bit to parity
/*    wait 5 mSec
/* end for
/* if parity is odd
/*    set output pin to '1'
/* else
/*    set output pin to '0'
/* wait for 5 mSec
/* set output pin to '0'
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
	char parity;

	wiringPiSetup();

	pinMode(0,OUTPUT);  /* This will drive the infrared transmitter. (Not directly, some sort of transistor for current gain needed.) */
	pinMode(2,INPUT);   /* This will read the infrared receiver. */

	digitalWrite(0,HIGH);	/* digitalWrite maintains this value at the output until changed by a subsequent write */
	delay(10);
	parity = 0;		/* parity will be used to count the ones. */
	for (i=0; i< 8; i++)
	{
		bit = (letter >> (7-i)) & 0x01;
		digitalWrite(0,bit);
		parity = parity + bit;
		delay(5);
	}
	digitalWrite(0,(parity & 0x01));	/* if parity is an odd number, bit0 = '1' and a 1 is sent.  If it is even, bit0 = '0' and 0 is sent. */
	delay(5);
	digitalWrite(0,LOW);

	while (1)
	{
	while(digitalRead(2)==LOW)
	{ 
		delay(1);
	}

	/* we're out of the while.  We must have gotten a 1.  Let's see if it is the beginning pulse. */
	delay(2);	/* should put us 2 to 3 mSecs into the first 10mSec pulse of a reply */
	if (digitalRead(2))
	{	/* still a 1 */
		delay(5);	/* should put us 7 to 8 mSecs into the first 10mSec pulse of a reply */
		if (digitalRead(2))
		{	/* still a 1. We got the beginning pulse. */
			parity = 0;
			for (i=0;i<8;i++)
			{
				delay(5);	/* move to the next pulse period, roughly centered */
				bit = digitalRead(2);
				parity = parity + bit;
				letter = (letter << 1) | (bit & 0x01);
			}
			delay(5);	/* wait for the parity bit */
			if ( (parity & 0x01) == digitalRead(2) )  /* if the parity calculated from the data matches the received parity bit */
				printf("Character received is %c.\n", letter);
			else
				printf("Parity error encountered. Sorry, dude.\n");
		}
		else break;
	}
	} /* end of while (1) */
	return 0;
}
	