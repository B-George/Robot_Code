#include <stdio.h>
#include <wiringPi.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>

int main(void)
{
	char letter = 0x54; /* the ASCII for the letter 'T' */
	char parity;
	pid_t parent_pid;
	pid_t child_pid;

	parent_pid = getpid();

	wiringPiSetup();

	pinMode(0,OUTPUT);  /* This will drive the infrared transmitter. (Not directly, some sort of transistor for current gain needed.) */
	pinMode(2,INPUT);   /* This will read the infrared receiver. */

	child_pid = fork();
	if (child_pid == 0)
	{	/* The parent transmits */

		while(1)
		{
		digitalWrite(0,HIGH);	/* digitalWrite maintains this value at the output until changed by a subsequent write */
		delay(10);
		parity = 0;		/* parity will be used to count the ones. */
		for (int i=0; i< 8; i++)
		{
			bool bit = (letter >> (7-i)) & 0x01;
			digitalWrite(0,bit);
			parity = parity + bit;
			delay(5);
		}
		digitalWrite(0,(parity & 0x01));	/* if parity is an odd number, bit0 = '1' and a 1 is sent.  If it is even, bit0 = '0' and 0 is sent. */
		delay(5);

		//5ms between each transmission so receiver can stay in sync.
		digitalWrite(0,LOW);
		delay(10);
		} /* end of while(1) */
	}
	else
	{	/* the child receives */
		int j = 0;
		while (1)
		{
		while(digitalRead(2)==HIGH)
		{
			delay(1);
		}

		/* we're out of the while.  We must have gotten a 1.  Let's see if it is the beginning pulse. */
		delay(2);	/* should put us 2 to 3 mSecs into the first 10mSec pulse of a reply */
		if (!digitalRead(2))
		{	/* still a 1 */
			delay(5);	/* should put us 7 to 8 mSecs into the first 10mSec pulse of a reply */
			if (!digitalRead(2))
			{	/* still a 1. We got the beginning pulse. */
				parity = 0;
				for (int i=0;i<8;i++)
				{
					delay(5);	/* move to the next pulse period, roughly centered */
					bool bit = !digitalRead(2);
					parity = parity + bit;
					letter = (letter << 1) | (bit & 0x01);
				}
				delay(5);	/* wait for the parity bit */
				if ( (parity & 0x01) != digitalRead(2) ) {  /* if the parity calculated from the data matches the received parity bit */
					printf("Character %d received is %c.\n", j, letter);
					j++;
				}
				//else
				//	printf("Parity error encountered. Sorry, bro.\n");

			}
			//else break;
		}
		} /* end of while (1) */
	}
	return 0;
}