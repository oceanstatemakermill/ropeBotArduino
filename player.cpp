#include "player.h"
#include "Arduino.h"

#define DEFAULT_SPEED 255

Player::Player() {
	//Set pins variables
	topStopPin = -1;
	bottomStopPin = -1;
	outPin = -1;
	upButtonPin = -1;
	downButtonPin = -1;
	
	initialize(false);
}

Player::Player(int top, int bottom, int out, int up, int down, Motor* theMotor ) {
	//Set pins variables
	topStopPin = top;
	bottomStopPin = bottom;
	outPin = out;
	upButtonPin = up;
	downButtonPin = down;
		
	motor = theMotor;

	initialize();
}

void Player::update() {
	up.update();
	down.update();

	if( controlsEnabled ) {

		if( up.rose() ) {
			upHits++;
		}
		if( down.rose() ) {
			downHits++;
		}

		//Update the speed with the current number of hits
		if( millis() - hitTimer.start > hitTimer.duration ) {
			currentSpeed = (currentSpeed * FRICTION) / 100;
			currentSpeed += upHits * ACCELERATION_FACTOR;
			currentSpeed -= downHits * ACCELERATION_FACTOR;

			if( currentSpeed > 255 ) {
				currentSpeed = 255;
			}
			if( currentSpeed < -255 ) {
				currentSpeed = -255;
			}
			
			upHits = 0;
			downHits = 0;
			hitTimer.start = millis();
		}
	}

    move();
    
	switch( state ) {
		case offmark:
			//Turn off motors
			if( digitalRead(bottomStopPin) == HIGH ) {	
				state = onmark;
			}
			break;
		case onmark:
			if( digitalRead(bottomStopPin) == LOW ) {
				state = started;
			}			
			break;			
		case started:
			//Turn on motors
			if( digitalRead(bottomStopPin) == HIGH ) {
				state = onmark;
			} else if( digitalRead(topStopPin) == HIGH ) {
				state = halfway;
			}
			break;
		case halfway:
			if( digitalRead(bottomStopPin) == HIGH ) {
				state = finished;
			}
			break;
		case finished:
			//Turn off motors
			break;
	}
	
	if( state != previousState ) {
//		Serial.println(getState());
		previousState = state;
	}
}

const char* Player::getState() {
	return stateNames[state];
}

bool Player::isOnMark() {
	return state == onmark;
}

bool Player::isFinished() {
	return state == finished;
}

void Player::setOut(bool value) {
	digitalWrite(outPin, value);
}

void Player::setControls(bool enabled) {
	controlsEnabled = enabled;
}

void Player::stopMotors() {
	motor->brake();
}

void Player::reset() {
	setControls(false);
	setOut(false);
	state = offmark;	
	
	if( digitalRead(topStopPin) == LOW ) {
		motor->drive( DEFAULT_SPEED / 4, 750 );
	}	
	move(-DEFAULT_SPEED / 4);
}

// PRIVATE FUNCTIONS

void Player::initialize(bool pinsSet) {
	//ID
	ID = "";
	
	//Pins
	if( pinsSet ) {
		pinMode( topStopPin, INPUT);
		pinMode( bottomStopPin, INPUT);
		pinMode( outPin, OUTPUT);
		pinMode( upButtonPin, INPUT );
		pinMode( downButtonPin, INPUT );
	}

	//Debounce objects for player controllers
	up = Bounce();
	up.attach(upButtonPin);
	up.interval(DEBOUNCE_TIME);
	
	down = Bounce();
	down.attach(downButtonPin);
	down.interval(DEBOUNCE_TIME);	

	//Motor
	currentSpeed = 0;
	controlsEnabled = false;
	upHits = 0;
	downHits = 0;
	hitTimer.start = millis();
	hitTimer.duration = HIT_RESET_TIME;

	//State
	state = offmark;
	previousState = finished;
}

void Player::move(int newSpeed) {
	currentSpeed = newSpeed;
	move();
}

void Player::move() {
	if( digitalRead(topStopPin) == HIGH && currentSpeed > 0 ) {
		motor->brake();
		currentSpeed = 0;
	} else if( digitalRead(bottomStopPin) == HIGH && currentSpeed < 0) {
		motor->brake();
		currentSpeed = 0;
	} else if( currentSpeed != 0 ) {
		motor->drive(currentSpeed);
	} else {
		motor->brake();
	}	
}

