#ifndef BUTTON_H
#define BUTTON_H

class Button{

    public:

    bool lastState;
    bool currentState;
    const int BUTTON_PIN;
    unsigned long pressStart;
    unsigned long pressDuration;


    Button(int BUTTON_PIN);

    void setPinMode();

    void update();

    bool pressed();

    bool isDown();

    bool released();

};

#endif