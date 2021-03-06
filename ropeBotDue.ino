//  RopeBot Due
//
//  Code for Arduino Due running game controllers for Ocean State Maker Mill Ropebot race.
//
//  The Due controls:
//  * A starter button, 
//  * Two up down controllers for players, 
//  * Two motors through Sparkfun HBridge breakout board for the bots 
//  * Communicates with Raspberry Pi which manages the race and displays results.

#include "pins.h"
#include "player.h"
#include <SparkFun_TB6612.h>

const int playerCount = 2;
Player* players[playerCount];

Motor motor0 = Motor( AIN1, AIN2, PWMA, 1, STBY );
Motor motor1 = Motor( BIN1, BIN2, PWMB, 1, STBY );

enum RaceState {
	initiating,
	playersReady,
	falseStart,
	begun,
	complete
} raceState, lastRaceState;

long startTime;

void setup() {
	players[0] = new Player(PLAYER_0_TOP, PLAYER_0_BOTTOM, PLAYER_0_OUT, PLAYER_0_UP, PLAYER_0_DOWN, &motor0 );
	players[1] = new Player(PLAYER_1_TOP, PLAYER_1_BOTTOM, PLAYER_1_OUT, PLAYER_1_UP, PLAYER_1_DOWN, &motor1 );

	players[0]->ID = "Zero";
	players[1]->ID = "One";

	pinMode( RESET_BUTTON, INPUT );
	pinMode( PLAYERSREADY_OUT, OUTPUT );
	pinMode( COUNTDOWNOVER_IN, INPUT );
	
	startTime = millis();
	
    digitalWrite( PLAYERSREADY_OUT, HIGH );
	delay(500);
	for(int i = 0; i < 4; i++) {
		digitalWrite( PLAYERSREADY_OUT, LOW );
		delay(200);
		digitalWrite( PLAYERSREADY_OUT, HIGH );
		delay(100);
	}
	
	resetRace();
	
	Serial.begin(9600);
	Serial.println("Start up");
}

void loop() {
	if( digitalRead( RESET_BUTTON ) == HIGH ) {
		resetRace();
	}
	
	//Update player state
	for( int i = 0; i < playerCount; i++) {
		players[i]->update();
	}

	updateRace();
	
	debug();
}

void resetRace() {
	//RESET RACE
	digitalWrite( PLAYERSREADY_OUT, LOW);
	raceState = initiating;

	//RESET PLAYERS
	for( int i = 0; i < playerCount; i++) {  
		players[i]->reset();
	}
}

void setControls(bool enabled) {
	for( int i = 0; i < playerCount; i++) {
		players[i]->setControls(enabled);
	}
}

void updateRace() {
	bool allplayersReady;
	int finishedCount;
	
	switch( raceState ) {
		case initiating:
			allplayersReady = true;
			for( int i = 0; i < playerCount; i++) {
				if( players[i]->isOnMark() == false ) {
					allplayersReady = false;
					break;
				}
			}
			if( allplayersReady ) {
				raceState = playersReady;
				digitalWrite( PLAYERSREADY_OUT, HIGH );
				setControls( true );
			}
			break;
		case playersReady:
			//Enable motors
			if( digitalRead( COUNTDOWNOVER_IN ) == HIGH ) {
				raceState = begun;
				digitalWrite( PLAYERSREADY_OUT, LOW );
			} else {
				for( int i = 0; i < playerCount; i++) {
					if( players[i]->isOnMark() == false ) {
						raceState = falseStart;
						players[i]->setOut(HIGH);
						break;
					}
				}
			}			
			break;
		case falseStart:
			for( int i = 0; i < playerCount; i++) {
				 players[i]->setControls(false);
				 players[i]->stopMotors();
			}
			delay(3000);
			resetRace();
			break;	
		case begun:
			finishedCount = 0;
			for( int i = 0; i < playerCount; i++) {
				if( players[i]->isFinished() ) {
					players[i]->setOut(HIGH);
					finishedCount += 1;
				}				
			}
			if( finishedCount == playerCount ) {
				raceState = complete;
			}
			break;
		case complete:
			//Disable motors
			delay(5000);
			resetRace();
			break;
	}
}

void debug() {
	if( lastRaceState != raceState ) {
		switch( raceState ) {
			case initiating:
				Serial.println("Initiating");
				break;
			case playersReady:
				Serial.println("Players set");
				break;
			case falseStart:
				Serial.println("False start");
				break;
			case begun:
				Serial.println("Begun");
				break;
			case complete:
				Serial.println("Complete");
				break;
		}
	}
	lastRaceState = raceState;
}









