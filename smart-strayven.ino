#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Button.h>

//set the height and width of the screen
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

//initialize stuff for display
#define OLED_RESET 4

//text sizes
#define AMMO_TEXT_SIZE 6

#define IR_MAP_TRIP_VAL 95
#define DART_LEGNTH_FEET 0.2362208333

//for voltmeter
#define R1 100000.0
#define R2 10000.0

//for PWM
#define POT_PIN 2
#define PWM_OUT_PIN 3
#define MOTOR_ACCEL_TIME 200
#define PWM_MAPPED_MAX_OUTPUT_THRESHOLD 16

//io pins
#define IR_RECEIVER_PIN 0
#define VOLTMETER_PIN 1

//pins for buttons
#define TRIGGER_BTN_PIN 4
#define RELOAD_BTN_PIN 7
#define MAG_SZ_TOG_BTN_PIN 8

//parameters for buttons
#define INVERT true
#define PULLUP true
#define DEBOUNCE_MS 20

Adafruit_SSD1306 display(OLED_RESET);     //display

Button reloadBtn (RELOAD_BTN_PIN, PULLUP, INVERT, DEBOUNCE_MS);         //reloading button
Button magSzTogBtn (MAG_SZ_TOG_BTN_PIN, PULLUP, INVERT, DEBOUNCE_MS);   //magazine size toggle button
Button triggerBtn (TRIGGER_BTN_PIN, PULLUP, INVERT, DEBOUNCE_MS);

byte magSizeArr[] = {5, 6, 10, 12, 15, 18, 20, 22, 25, 36, 0};  //keep track of the magazine sizes
byte currentMagSize = 0;  //keep track of the current magazine size
byte currentAmmo = magSizeArr[currentMagSize];    //keep track of how much ammo there currently is
byte maxAmmo = magSizeArr[currentMagSize];    //keep track of what the max ammo is, for use when reloading 

double tripTime, exitTime;	//values to keep track of chrono timing

boolean hasAccelerated = false;
int lastPotReading = 0;

String chronoToPrint, ammoToPrint, voltageToPrint;		//keep track of what  vals to print

//this code will run when the Arduino turns on
void setup() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);    //begin stuff for the display
  initDisplayAmmo();     //show the ammo
}

//this code loops many times a second
void loop() {
  chrono();    //count ammo, constantly check for the trigger switch to be pressed to count
  reload();       //reload, constantly check for the magazine switch to be pressed/not pressed
  toggleMags();   //toggle between magazine sizes, constanly check for the magazine toggle switch to be pressed
  voltMeter();
  pwm();
}

//actually dispaly ammo onto screen
void displayValues () {
  display.clearDisplay();           //clear the display, so the stuff that was here before is no longer here
  display.setTextColor(WHITE);      //set the color of text text

  display.setTextSize(AMMO_TEXT_SIZE);   //set the size of the text
  display.setCursor( (SCREEN_WIDTH/2) - ((ammoToPrint.length()*2) * (AMMO_TEXT_SIZE * 1.5)), (SCREEN_HEIGHT/2) - (AMMO_TEXT_SIZE * 3) );  //center text
  display.print(ammoToPrint);     //print the text

	display.setTextSize(1);
	display.setCursor(0, 45);
  display.print(chronoToPrint);

  display.setCursor(60, 45);
	display.print(voltageToPrint);

	int lineLength = lastPotReading * 8;
	display.drawLine(0, 61, lineLength, 61, WHITE);
  display.drawLine(0, 62, lineLength, 62, WHITE);
  display.drawLine(0, 63, lineLength, 63, WHITE);

  display.display();                //display the text
}

//set up ammo to be displayed
void initDisplayAmmo () {
  //if the ammo to print, current ammo, is less that 10, make it like '01' or '04'  
  String ammoToPrint = currentAmmo < 10 ? ("0" + (String)currentAmmo) : (String)currentAmmo;
  displayValues();  //display the text, the ammo
}

void displayChronoValues(String toPrint) {
    chronoToPrint = toPrint;
    displayValues();
}

void initDisplayChronoValues(double fps) {
    if (fps == -1) {
        chronoToPrint = "ERR";
    } else if (fps == -2) {
        chronoToPrint = "NO FPS";
    } else {
        chronoToPrint = ( (String)(fps)  + " fps" );
    }
}

void resetChronoVals() {
    tripTime = -10;
    exitTime = -10;
}

double calculateChronoReadings(double firstTime, double secondTime) {
    if ( (tripTime > -10) && (exitTime > -10) ) {
        resetChronoVals();
        return (DART_LEGNTH_FEET) / ((secondTime-firstTime)/1000000.0);
    }
}

void chrono() {
    //when tripped and expecting first value
    if ((map(analogRead(IR_RECEIVER_PIN), 0, 1023, 0, 100) > IR_MAP_TRIP_VAL) && (tripTime == -10) ) { 
        tripTime = micros();
    //when tripped and expecting second value
    } else if ( (tripTime != -10) && (exitTime == -10) && (map(analogRead(IR_RECEIVER_PIN), 0, 1023, 0, 100) < IR_MAP_TRIP_VAL) )  {
        exitTime = micros();
        initDisplayChronoValues(calculateChronoReadings(tripTime, exitTime));

        //count ammo stuff
        //make sure that the ammo is less than 99 so it doesnt overflow the display
        //make sure it's in increment mode
        if ( (magSizeArr[currentMagSize] == 0) && (currentAmmo < 99) ) {
            currentAmmo++;    //increment ammo
        
        //make sure that the ammo is more than 0 so no negative numbers are displayed
        //make sure it's in increment mode
        } else if ( (currentAmmo > 0) && (magSizeArr[currentMagSize] != 0) ){
            currentAmmo--;    //decrement ammo
        }
    
        initDisplayAmmo();    //display the ammo    
        
    //when no second value within 1 second
    } else if ( (tripTime != -10) && (tripTime + 2000000) < micros() ) {
        resetChronoVals();
        initDisplayChronoValues(-1);
    } 
}

void reload () {
  reloadBtn.read();               //read reload button
  if (reloadBtn.wasPressed()) {   //reload button pressed
    currentAmmo = maxAmmo;        //reset ammo
    initDisplayAmmo();            //display ammo
  }
}

void toggleMags () {
  magSzTogBtn.read();               //read magazine size toggle button
  if (magSzTogBtn.wasPressed()) {    //magazine size toggle button pressed
    //cycle through mag sizes based on array, and make sure array doens't overflow
    currentMagSize = (currentMagSize < (sizeof(magSizeArr)/sizeof(magSizeArr[0]) - 1)) ? currentMagSize + 1 : 0;    

    //there's a new max ammo, because there's a new magazine size
    maxAmmo = magSizeArr[currentMagSize];
    currentAmmo = maxAmmo;

    initDisplayAmmo();      //display ammo
  }
}

//values for time checking on voltage
double lastVoltageCheckTime = 0;
int delayTime = 500;

//check and calculate and display voltage (RHETORICAL STRATEGY: POLYSYNDETON)
void voltMeter () {
  //make sure only prints every .5 sec
  if (millis() >= lastVoltageCheckTime + delayTime) {
    //calculate voltage
    float voltageIn = ((analogRead(VOLTMETER_PIN) * 1) / 1) ;

    //make sure voltage is above 0.03, since it could be an error
    if (voltageIn < 0.5) {
      voltageIn = 0; 
    }

    voltageToPrint = ((String)voltageIn + " v");

    lastVoltageCheckTime = millis();
  }
}

void pwm () {
	triggerBtn.read();                                          //read trigger so later can check if pressed/released
	if(triggerBtn.isPressed() && !hasAccelerated) {				//when trigger first pressed
		digitalWrite(PWM_OUT_PIN, HIGH);						//motor at full power
		delay(MOTOR_ACCEL_TIME);								//allow motor to reach full speed
		hasAccelerated = true;									//allow pwm
	} else if (triggerBtn.isPressed() && hasAccelerated) {		//if trigger pressed
		analogWrite(PWM_OUT_PIN, analogRead(POT_PIN)/4);		//write PWM depending on pot value
	} else if (triggerBtn.wasReleased()) {						//when trigger released
		digitalWrite(PWM_OUT_PIN, LOW);							//turn motor off
		hasAccelerated = false;									//reset flag to check for acceleration
	}

	if (map(analogRead(POT_PIN), 0, 1010, 0, 16) != lastPotReading) {
		lastPotReading = map(analogRead(POT_PIN), 0, 1010, 0, PWM_MAPPED_MAX_OUTPUT_THRESHOLD);
		displayValues();
	}
}

