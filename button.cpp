#include "button.h"
#include <Arduino.h>

Button::Button(int PIN) : BUTTON_PIN(PIN) {

    lastState = HIGH;
    currentState = HIGH;
    pressStart = 0;
    pressDuration = 0;

}

void Button::setPinMode(){

    pinMode(BUTTON_PIN, INPUT_PULLUP);

}

void Button::update(){

    lastState = currentState;
    currentState = digitalRead(BUTTON_PIN);

}

bool Button::pressed(){

    return (lastState == HIGH && currentState == LOW);
    
}

bool Button::isDown(){

    return(currentState == LOW);

}

bool Button::released(){

    return (lastState == LOW && currentState == HIGH);

}