// Benjamin George & Cameron Carlson
// Seattle University
// ECEGR Junior Lab Robot Code
// 2015

#include <wiringPi.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <ostream>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#define LM 26		// Right motor test control ---------------- pin 32
#define RM 23		// Left motor test control ----------------- pin 33
#define R_PWM 26 	// Left motor speed control GPIO 26 PWM 0 -- pin 32
#define L_PWM 23 	// Right motor speed control GPIO 23 PWM 1 - pin 33
#define R_REV 27 	// Left motor direction control GPIO 27 ---- pin 36
#define L_REV 25  	// Right motor direction control GPIO 25 --- pin 37
#define RUN_SW 1  	// Run switch control ---------------------- pin 12
#define SD_SW 6  	// Shutdown switch control ----------------- pin 22
#define US_TRIG 0 	// Ultrasound trigger ---------------------- pin 11
#define US_ECHO_1 2	// Ultrasound echo 1 ----------------------- pin 13
#define US_ECHO_2 3 	// Ultrasound echo 2 ----------------------- pin 15
#define IR_SEND 4	// IR out ---------------------------------- pin 16
#define IR_REC 5	// IR in ----------------------------------- pin 18
#define LED_PIN 15	// LED tester ------------------------------ pin 08

#define SPEED 1000	// speed for our PWM
#define SLOWSPEED 700 // slower speed for PWM
#define STOP 0		// speed == 0

#define REP(a, b)	for(int i = a; i < b; i++) // define for loop -- remove ;

#define ORIENT 0	// orientation phase
#define MOVING 1	// movement phase
#define ZFOUND 2	// confront zombie phase
#define ZKILL 3		// kill zombie phase

using namespace std;

// motor movement functions
void turn_left(int speed);
void turn_right(int speed);
void spin_left(int speed);
void spin_right(int speed);
void move_fwd(int speed);
void move_bwd(int speed);
void full_stop();
void move_fwd_direct();
void stop_direct();

// get distance from front of bot in CM
int getCM();

// sets all pins as defined above
void setPins();

// ultrasonic sensor timing
void pulse_start_ISR(void);
void pulse_end_ISR(void);

// IR  fxn
void IRrec();
void IRtrn();

struct timeval start, end;
double intervals[2048];
int i = 0;
int count = 0;
int twiddle, pfds[2], report;
char parityt, parityr, bit, buf[2];
char letter = 0x42; // ASCII for letter 'B'
int phase = ORIENT; 


int main()
{

full_stop();

// wait for run switch to be activated
while(!RUN_SW){}

pid_t parent_pid, child_pid;
parent_pid = getpid();
cout << "parent_pid = " << parent_pid << endl;
int distance = 0;

child_pid = fork();
if(child_pid == 0) { // child pid -- go to transmit fxn
	child_pid = getpid();
	printf("Imma child, going to IRtrn");
	IRtrn();
}else{ // parent_pid
	while(SD_SW){ // run while shutdown switch is closed
		if(phase == ORIENT) {	// Face away from walls -- top level if

			while(distance < 100) {
				distance = getCM();
				if(distance < 25){
					move_bwd(SPEED);
				}else{
					turn_right(SPEED);
				}
			}
			full_stop();
			// if facing away from walls
			phase = MOVING;

		} else if(phase == MOVING) { // top level if

			// moving down hall, look for zombies
			move_fwd(SPEED);
			// if we see something, check if it's a zombie
			if(getCM() <= 75){
				full_stop();
				turn_left(SPEED);
				delay(100);
				full_stop();
				if(getCM() <= 75){ // must be a wall
					turn_right(SPEED);
					delay(200);
					full_stop();
				}else{			// probably was a zombie
					turn_right(SPEED);
					delay(100);
					full_stop();
				}
			}
			// if US finds Zombie 
			phase = ZFOUND;

		} else if(phase == ZFOUND) {// top level if

				// aproach zombie, stop at 75 cm
				
				// if dist == 60 cm phase = ZKILL

		} else if(phase == ZKILL) { // top level if

				// message transmitter start
				// for 5 sec 
					// maintain 60 cm dist
					// if ipc r2n = my char
					// audio cheer
				// attempt audio lure
				// phase = ZFOUND
				
		} // end if (phase)
	} // end while(1)
// US sensor test code
/*
	int num;
	FILE *fp;

	wiringPiSetup();
	setPins();
	wiringPiISR( US_ECHO_1, INT_EDGE_RISING, &pulse_start_ISR );
	wiringPiISR( US_ECHO_2, INT_EDGE_FALLING, &pulse_end_ISR );

	full_stop();
	delay(60000);

	spin_left();

	REP(0,512)
	{
		delay(100);
		digitalWrite(US_TRIG, HIGH);
		delay(1);
		digitalWrite(US_TRIG, LOW);	
	}
	delay(100);
	full_stop();

	fp = fopen("lab5data.bin", "wb");
	num = fwrite(intervals, 8, 512, fp);
	fclose(fp);
	printf("wrote %d intervals\n", num);
	REP(0,20)
	{
		printf("%f0\n", intervals[i]);
	} 

	cout << "program complete";
*/
	

// test code for finding H(s)
/*
	wiringPiSetup();
	setPins();
	cout << "stop all for 10 sec\n\n";
	full_stop();
	delay(20000);
	digitalWrite(LED_PIN, HIGH);
	cout << "move fwd .5 sec\n\n";
	move_fwd();
	delay(500);
	digitalWrite(LED_PIN, LOW);
	full_stop();
	cout << "stop all\n\nend program";

*/

// motor function test code

/*	turn_left();
	std::cout << "turn left .5 sec\n";
	delay(500);
	std::cout << "wait .5 sec\n";
	full_stop();
	delay(500);
	turn_right();
	std::cout << "turn right .5 sec\n";
	delay(500);
	full_stop();
	std::cout << "wait .5 sec\n";
	delay(500);
*/
	}
	system("sudo shutdown -h now");
	return 0;	
 
}

void setPins()
{
		// outputs
		pinMode(L_PWM, PWM_OUTPUT);
		pinMode(R_PWM, PWM_OUTPUT);
		//pinMode(RM, OUTPUT);
		//pinMode(LM, OUTPUT);
		pinMode(L_REV, OUTPUT);
		pinMode(R_REV, OUTPUT);
		pinMode(IR_SEND, OUTPUT);
		pinMode(US_TRIG, OUTPUT);
		pinMode(LED_PIN, OUTPUT);
		// inputs
		pinMode(RUN_SW, INPUT);
		pinMode(SD_SW, INPUT);
		pinMode(US_ECHO_1, INPUT);
		pinMode(US_ECHO_2, INPUT);
		pinMode(IR_REC, INPUT);
		digitalWrite(US_TRIG, LOW); // trig pin must start low
	std::cout << "pins set\n\n";
}

void turn_left(int speed)
{
	pwmWrite(L_PWM, speed);
	pwmWrite(R_PWM, STOP);
	digitalWrite(L_REV, LOW); 
}

void turn_right(int speed)
{
	pwmWrite(L_PWM, STOP);
	pwmWrite(R_PWM, speed);
	digitalWrite(R_REV, LOW); 
}

void spin_left(int speed)
{
	pwmWrite(L_PWM, speed);
	pwmWrite(R_PWM, speed);
	digitalWrite(L_REV, LOW);
	digitalWrite(R_REV, LOW);
}

void spin_right(int speed)
{
	pwmWrite(L_PWM, speed);
	pwmWrite(R_PWM, speed);
	digitalWrite(L_REV, HIGH);
	digitalWrite(R_REV, LOW);
}

void move_fwd(int speed)
{
	pwmWrite(L_PWM, speed);
	pwmWrite(R_PWM, speed);
	digitalWrite(L_REV, LOW);
	digitalWrite(R_REV, LOW);
}

void move_bwd(int speed)
{
	pwmWrite(L_PWM, speed);
	pwmWrite(R_PWM, speed);
	digitalWrite(L_REV, HIGH);
	digitalWrite(R_REV, HIGH);
}

void full_stop()
{
	pwmWrite(L_PWM, STOP);
	pwmWrite(R_PWM, STOP); 
}

void move_fwd_direct()
{
	digitalWrite(L_REV, HIGH);
	digitalWrite(R_REV, HIGH);
	digitalWrite(RM, HIGH);
	digitalWrite(LM, LOW);
}

void stop_direct()
{
	digitalWrite(RM, LOW);
	digitalWrite(LM, HIGH);
	digitalWrite(L_REV, LOW);
	digitalWrite(R_REV, LOW);
}

int getCM() // with thanks to https://ninedof.wordpress.com
{
	// send pulse
	digitalWrite(US_TRIG, HIGH);
	delayMicroseconds(20);
	digitalWrite(US_TRIG, LOW);
	
	// wait for echo to start
	while(digitalRead(US_ECHO_1) == LOW);
	
	// wait for echo end
	long startTime = micros();
	while(digitalRead(US_ECHO_1) == HIGH);
	long travelTime = micros() - startTime;
	
	// get distance in cm
	int dist = travelTime / 58;
	
	return dist;
	
}

void pulse_start_ISR(void)
{
	gettimeofday(&start, NULL);
	return;
}

void pulse_end_ISR(void)
{
	double t_start, t_end;

	gettimeofday(&end, NULL);
	t_start = start.tv_sec + (start.tv_usec/1000000.0);
	t_end =  end.tv_sec + (end.tv_usec/1000000.0);
	intervals[i] = t_end - t_start;
	return;
}

void IRrec()
{
	pid_t parent_pid, child_pid;
	parent_pid = getpid();
	if(pipe(pfds) == -1) { // call pipe and center in pfds
		perror("pipi"); // call failed
		exit(1);
	}
	if (fcntl(pfds[0], F_SETFL, O_NONBLOCK) == -1){
		printf("call to fcntl failed.\n\n");
			exit(1);
		}
		srand(543216345);
		
		child_pid = fork();
		if(child_pid == 0) {
			child_pid = getpid();
			printf("I am child -- forking to IRtrn\n\n");
			IRtrn();
		}else{
			// parent receives
			printf("I am the parent with pid %d\n\n", parent_pid);
			if(close(pfds[0]) == -1) { // relinquish ownership of child end of pipe
				printf("PARENT: Couldn't close the child end of the pipe.\n\n");
				fflush(stdout);
				exit(1);
			}
			while(1){
				while(digitalRead(IR_REC == LOW)){
					delay(1); // wait for IR signal
				}
				delay(2); // 2-3 ms into a start
				if(digitalRead(IR_REC)){ // found 3 1's in a row(start bit)
					parityr = 0;
					for(i = 0; i < 8; i++){
						delay(5); // wait for next bit
						bit = digitalRead(IR_REC);
						parityr = parityr + bit;
						letter = ((letter << 1) | (bit & 0x01));
					}
					if ((parityr & 0x01) == digitalRead(IR_REC)) {
						printf("PARENT: Character received is %c.\n\n", letter);
						if (letter == '?')
							write(pfds[1], "?", 2);
					}else{
						printf("PARENT: Parity error encountered.\n\n");
					}
				}
			else break;
			} // end of while(1);
		}
}

void IRtrn()
{
	if(close(pfds[1]) == -1) {// relinquish ownership of parent's end of pipe
		printf("CHILD: Couldn't close end of pipe\n\n");
		exit(1);
	}
	while(1){
		switch (report = read(pfds[0], buf, 2))
		{
			case -1:
				if(errno == EAGAIN)
					twiddle = 1000; // keep rate of letter transmission
				fflush(stdout);
				break;
			case 0:
				printf("CHILD: Pipe writing closed from parent end!\n\n");
				fflush(stdout);
				exit(0);
				break;
			default:
				if(buf[0] == '?'){
					twiddle = (((rand()%10) + 1) * 600); // vary the rate of the next transmission
					buf[0] = 0;
					fflush(stdout);
			}else{
					twiddle = 2000;
					printf("CHILD: Read returned %i\n", report);
					fflush(stdout);
				}
				break;

		}
		printf("CHILD: twiddle = %i, count = %i\n", twiddle, count);
		delay(twiddle);
		digitalWrite(IR_SEND, HIGH); // maintain value at output until changed by subsequent write
		delay(10);
		parityt = 0; // count the ones
		if(count%5 == 0) {
				for (i = 0; i < 8; i++) {
					bit = ('?' >> (7-i)) & 0x01;
					digitalWrite(IR_SEND, bit);
					parityt = (parityt + bit);
					delay(5);
				}
		}else{
				for(i = 0; i < 8; i++) {
					bit = (letter >> (7 - i)) & 0x01;
					digitalWrite(IR_SEND, bit);
					parityt = parityt + bit;
					delay(5);
				}
		}
		digitalWrite(IR_SEND, (parityt & 0x01)); /* if parity is odd, bit 0 is 1 
													and 1 is sent else 0 is sent */
		delay(5);
		digitalWrite(IR_SEND, LOW);
		if(++count >= 20)
			exit(0);
	}// end of while
}

