// DEVELOPED BY ROGER MILLA FORTUNATO SANTOS

#include "mbed.h"


Ticker ticker_led,ticker_Press;
Timer timer;
InterruptIn button(PA_0);

enum States {DOWN, UP, FALL, RISE, MIN, MAX};      //Estados da máquina
enum Events {ONE_PRESS,PRESS_TO_FALL, PRESS_TO_RISE, BT_RELEASE,BT_PRESSED, MAX_LUMINOSITY, MIN_LUMINOSITY}; 
//PwmOut Leds[]={PD_15,PD_13,(PA_9)}; 
//PwmOut Leds[]={LED1,LED2,(PB_1)}; 

PwmOut pwm_port(PA_9);
DigitalOut myYELLOWled(PD_13);
DigitalOut myBLUEled(PD_15);


void StateMachine(int event);   //Função da máquina de estados
Events eventSelect(void);          //Função que verifica qual evento foi gerado
void stateSelect(int state);    //Função que seleciona o próximo estado da máquina de estados


//*************************** Variáveis globais ********************************

States nextState = DOWN;        //Próximo estado da máquina de estado
States currentState = DOWN;
Events event;
float timePressed=0;
bool button_pressed=false;
int Y_Led_blinking=0,B_Led_blinking=0;
bool occurred_event=false;
bool direction=0;


//*************************** Protótipo de funções ********************************

void Leds_OFF();
void timer_begin();
void timer_end();
void blink_YELLOW();
void blink_BLUE();
void check_bt();



int main() {
       
    button.rise(&timer_begin);
    button.fall(&timer_end);

    while(1){
        if (occurred_event){
            event = eventSelect();
            StateMachine(event);
            stateSelect(nextState);
            occurred_event=false;   
        } 
    }   
}




//**************************************************************************
//---------------------------- STATE MACHINE -------------------------------

void StateMachine(int event){       //Função chamada para alterar o próximo estado da máquina

    switch (currentState){
        case UP:
            switch(event){
                case ONE_PRESS:
                    nextState = DOWN;
                break;       
                case PRESS_TO_RISE:
                    nextState = RISE;
                break;
            }
        break;

        case DOWN:
            switch(event){                
                case ONE_PRESS:
                    nextState = UP;
                break;                       
                case PRESS_TO_FALL:
                    nextState = FALL;
                break;
            }
        break;
        
        case FALL:
            switch(event){
                case BT_PRESSED:
                    nextState =FALL;
                break;                       
                case BT_RELEASE:                
                    nextState = DOWN;                    
                break;
                case MIN_LUMINOSITY:
                    nextState=MIN;
                break;
            }
        break;

        case RISE:
            switch(event){
                case BT_PRESSED:
                    nextState = RISE;
                break;                       
                case BT_RELEASE:                
                    nextState = UP;                    
                break;
                case MAX_LUMINOSITY:
                    nextState=MAX;
                break;
            }
        break;

        case MIN:
            nextState = UP;
        break;
        
        
        case MAX:
            nextState = DOWN;
        break;

    }

}


Events eventSelect(void){      //Função que detecta qual evento foi gerado
       
    if ((B_Led_blinking==1) || (Y_Led_blinking==1))
        return BT_PRESSED;

    if ((currentState==RISE) && (pwm_port==1))
        return MAX_LUMINOSITY;

    if ((currentState==FALL)  && (pwm_port==0))
        return MIN_LUMINOSITY; 

    if (((currentState==RISE) || (currentState==FALL))  && (B_Led_blinking==0) && (Y_Led_blinking==0))
        return BT_RELEASE;

    if ((currentState==MAX) || (currentState==MIN))
        return BT_RELEASE;

    if (timePressed < 1)
        return ONE_PRESS;
        
    if ((timePressed>1) && (currentState==UP))
        return PRESS_TO_RISE;

    if ((timePressed>1) && (currentState==DOWN))
          return PRESS_TO_FALL;
      
}
        



void stateSelect(int state){        //Função chamada para configurar as ações de cada estado

        switch (state){
            case UP:
                ticker_Press.detach();
                Leds_OFF();            //Apaga leds
                myBLUEled=1;              //Acende led AZUL
                button.enable_irq();
                occurred_event=false;
                break;
                
            case DOWN:
                Leds_OFF();
                myYELLOWled=1;              //Acende led AMARELO 
                button.enable_irq();
                occurred_event=false;               
                break;

              case FALL:
                direction=0;
                button.disable_irq();
                ticker_led.attach(&blink_YELLOW,0.2);
                ticker_Press.attach(&check_bt,0.1);
                break;

            case RISE:
                direction=1;
                button.disable_irq();
                ticker_led.attach(&blink_BLUE,0.2);
                ticker_Press.attach(&check_bt,0.1);                 
                break;

            case MIN:
                Leds_OFF();                
                break;    

            case MAX:
                Leds_OFF();
                break;
                
        }

    currentState = nextState;
}


void check_bt (){
    occurred_event=true;    
    button_pressed=true;
    timePressed=0;
        switch (button.read()){

              case 1:
                 if (direction==1){
                      pwm_port=pwm_port+0.05;
                      button_pressed=false;
                 }else{
                      pwm_port=pwm_port-0.05;
                      button_pressed=false;
                 }
                  

              break;

              case 0:

                   switch (button_pressed){
                       case true:
                          
                       break;
                   
                       case false:
                          B_Led_blinking=0;
                          Y_Led_blinking=0;
                          ticker_Press.detach();
                          ticker_led.detach(); 

                       break;
                   }

              break;
        }
}

//**************************************************************************
//---------------------------- BLINK YELLOW LED ----------------------------

void blink_YELLOW()
{
    Y_Led_blinking=1;
    myYELLOWled=!myYELLOWled;
    //LED1=!LED1;
    
}

//---------------------------- BLINK BLUE LED -------------------------------
void blink_BLUE()
{
       B_Led_blinking=1;
       myBLUEled=!myBLUEled;   // Each Blink correspond to 1 sec.
       //LED2=!LED2;
       
}



//**************************************************************************
//---------------------------- PRESS BUTTON ------------------------------

void timer_begin() {            //rise, press button

    timer.start();    
    
}

void timer_end()             //fall, release button
{
    timer.stop();
    timePressed=timer.read();
    timer.reset();
    ticker_led.detach();
    occurred_event=true;
}


//**************************************************************************
//---------------------------- TURN LEDS OFF -------------------------------

void Leds_OFF()
{

myBLUEled=0;
myYELLOWled=0;

}

