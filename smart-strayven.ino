#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Button.h>

//set the height and width of the screen
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

//initialize stuff for display
#define OLED_RESET 4
#define TEXT_SIZE 8

//pins for button
#define TRIGGER_BTN_PIN 1
#define RELOAD_BTN_PIN 4
#define MAG_SZ_TOG_BTN_PIN 5

//parameters for buttons
#define INVERT true
#define PULLUP true
#define DEBOUNCE_MS 20

Adafruit_SSD1306 display(OLED_RESET);     //display

Button reloadBtn (RELOAD_BTN_PIN, PULLUP, INVERT, DEBOUNCE_MS);         //reloading button
Button magSzTogBtn (MAG_SZ_TOG_BTN_PIN, PULLUP, INVERT, DEBOUNCE_MS);   //magazine size toggle button

byte magSizeArr[] = {5, 6, 10, 12, 15, 18, 20, 22, 25, 36, 0};  //keep track of the magazine sizes
byte currentMagSize = 0;  //keep track of the current magazine size
byte currentAmmo = magSizeArr[currentMagSize];    //keep track of how much ammo there currently is
byte maxAmmo = magSizeArr[currentMagSize];    //keep track of what the max ammo is, for use when reloading 

//this code will run when the Arduino turns on
void setup() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);    //begin stuff for the display
  initDisplayAmmo();     //show the ammo
}

//this code loops many times a second
void loop() {
  countAmmo();    //count ammo, constantly check for the trigger switch to be pressed to count
  reload();       //reload, constantly check for the magazine switch to be pressed/not pressed
  toggleMags();   //toggle between magazine sizes, constanly check for the magazine toggle switch to be pressed
}

//actually dispaly ammo onto screen
void displayAmmo (String ammoToDisplay) {
  display.clearDisplay();           //clear the display, so the stuff that was here before is no longer here
  display.setTextSize(TEXT_SIZE);   //set the size of the text
  display.setTextColor(WHITE);      //set the color of text text
  display.setCursor( (SCREEN_WIDTH/2) - ((ammoToDisplay.length()*2) * (TEXT_SIZE * 1.5)), (SCREEN_HEIGHT/2) - (TEXT_SIZE * 3) );  //center text
  display.print(ammoToDisplay);     //print the text
  display.display();                //display the text
}

//set up ammo to be displayed
void initDisplayAmmo () {
  //if the ammo to print, current ammo, is less that 10, make it like '01' or '04'  
  String ammoToDisplay = currentAmmo < 10 ? ("0" + (String)currentAmmo) : (String)currentAmmo;
  displayAmmo(ammoToDisplay);  //display the text, the ammo
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