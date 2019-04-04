#include <Ethernet.h> 
#include <Servo.h>
#include <Keypad.h>
#include <LiquidCrystal.h>

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// initialize the library instance:
EthernetClient client;

//initialize lcd
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

//initialize servo
Servo doorLock;

// stages of door(servo)lock
int doorClose = 180;
int doorOpen = 90;

//string object in which we will store the entered code
String typedCode="";

//string object to show in default settings of lcd
String message2 = "*=terug #=stuur";

//bookSensor
int buttonState = 1;  

//available paths after logincode was right
const int TAKEBOOK = 1;
const int PLACEBOOK = 2;

//initialize keypad
const byte ROWS = 4; 
const byte COLS = 4; 

char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {34, 36, 38, 40}; 
byte colPins[COLS] = {26, 28, 30, 32}; 

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 


// Variable for web api
char server[]="ncjpoppe.acue.webpages.avans.nl";

void setup() {
  Serial.begin(9600);
   // initialize lcd
  lcd.begin(16, 2);

  //configure pin2 as an input and enable the internal pull-up resistor
  // to check if there's a book in the safe
  pinMode(8, INPUT_PULLUP);
  
  // initialize startposition servo
  doorLock.attach(9);
  doorLock.write(doorClose);
   
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  
  // connect to network
  Serial.println("Initialize Ethernet with DHCP:");

  //feedback
  lcd.print("please wait");
  
  if (Ethernet.begin(mac) == 0) {

    //feedback
    printMessage("internet gefaald");
    Serial.println("Failed to configure Ethernet using DHCP");

  } else {
    setupScreen();
    Serial.print("  DHCP assigned IP ");
    Serial.println(Ethernet.localIP());
  }
}

//function to set the default screen settings
void setupScreen() {
  typedCode =""; 
    lcd.clear();
    //print default message on bottom of lcd
    lcd.setCursor(0,1);
    lcd.print(message2);
    //print typedcode on top of lcd
    lcd.setCursor(0,0);
}

//function if person has pressed * to delete one number of their typed code
void resetCode(String remainingCode) {
    setupScreen();
    lcd.print(remainingCode);
}

//function to print any type of message if the default screen settings don't apply
void printMessage(String message){
     lcd.clear();
     //start message at the top
     lcd.setCursor(0,0);

     //logic so that the message will print on both rows of the lcd screen
     String newmessage =message.substring(0,16);
     lcd.print(message.substring(0,16));

    if(message.length()>16){
       lcd.setCursor(0, 1);
       lcd.print(message.substring(16,32));
    }
  }


void loop() {
        //get typed key from keypad
       char customKey = customKeypad.getKey();

       //read if the pullup is HIGH (no book) or LOW (book)
       int buttonState = digitalRead(8);

      //first, check if typedCode is longer than 6
      if (typedCode.length() < 6) {

       // some numbers on the keypad that can't be part of an unlockcode are ruled out
       if (customKey && customKey != '*' && customKey != '#' && customKey != 'A' && customKey != 'B' && customKey != 'C' && customKey != 'D'){
            // add typed key to full code
           typedCode = typedCode + customKey; 
           lcd.print(customKey);
        }

        //logic to remove a typed key from the typedCode string
        if (customKey == '*'){
          //delete last digit of the typedCode
          typedCode = typedCode.substring(0,typedCode.length()-1);
          resetCode(typedCode);
        }
       }
      
     // code completed (code is 6 digits)
      else{
        // //logic to remove a typed key from the typedCode string even if code is 6
        if (customKey == '*'){
          //delete last digit of the typedCode
          typedCode = typedCode.substring(0,typedCode.length()-1);
          resetCode(typedCode);
        }
        
        // if user sends the code by pressing #
        if (customKey=='#') {

          //reset customKey to prevent failing of code if 
          // someone presses # another time later in the code
          customKey = 'D';
          Serial.println("entered code: "+typedCode);

          //get validation result from httprequest of webapp to see if code is in the database
          int validationResult=httpRequest("/webapp/zoekboek/checkcode.php?code="+typedCode);

          //take book
          if (validationResult==TAKEBOOK) {
               //is there a book in the safe? (does the button detect a book)
               if (buttonState == LOW) {
                 
                  //provide user with feedback
                  printMessage("pak boek,sluit deur, druk op #");
                  
                  //open the door
                  doorLock.write(doorOpen);
                  
                  while (buttonState == LOW){
                    //reset button to see if it's released (if the book gets picked up)
                    buttonState = digitalRead(8);                    
                  }
                    //if user has picked up the book the feedback changes
                    printMessage("sluit deur, druk op #");

                    //wait for correct button to be pressed
                     while (customKey !='#'){
                    customKey = customKeypad.getKey();
                  }
                  
                    //user tries to close door
                   if (customKey=='#') {
                      doorLock.write(doorClose);
                      
                      //door has to be closed to start httprequest
                      if (doorLock.read() == doorClose){
                        
                          //httprequest to delete book from books database and delete code from unlockcodes database
                          int placementResult=httpRequest("/webapp/zoekboek/deletecode.php?code="+typedCode);

                          //placementresult 3 = success
                          if (placementResult == 3) {
                              //provide user with feedback before restarting the loop
                               printMessage("boek opgepakt!");
                               delay(4000);
                               setupScreen();
                            }
                          else {
                            //door stays open if database fails
                                doorLock.write(doorOpen);
                                printMessage("db failed");
                            }
                          }
                     else {
                            doorLock.write(doorOpen);
                            printMessage("error");
                      }
                  }
                    
               }
               else { 
                    //is there no book in the safe? then the door stays shut and the loop starts at the beginning
                  lcd.clear();
                  printMessage("helaas, er ligt nog geen boek.");
                  doorLock.write(doorClose);
                  delay(5000);
                  setupScreen();
             }
          
          } 

          //place book
          else if (validationResult==PLACEBOOK) {

              //is there room to place a book? (is the buttonstate low)
              if (buttonState == HIGH) {
                //provide user with feedback and open door
                printMessage("plaats boek,sluit deur, druk op #");
                doorLock.write(doorOpen);
                
                while (buttonState == HIGH){
                     //reset button to see if it's pressed (if the book gets placed)
                    buttonState = digitalRead(8);
                }

                //message changes if user placed the book
                printMessage("sluit deur, druk op #");

                //wait for correct button to be pressed
                while (customKey !='#'){
                    customKey = customKeypad.getKey();
                  }
                  
                 //user tries to close door
                 if (customKey=='#') {
                    doorLock.write(doorClose);

                    //check if door is closed to start http request
                    if (doorLock.read() == doorClose){
                      
                        //httprequest to add the code to unlockcodes and mark book as 'placed' in books database
                        int placementResult=httpRequest("/webapp/zoekboek/addcode.php?code="+typedCode);

                        //placementResult 3 = success
                        if (placementResult == 3) {
                            //provide user with feedback before resetting the loop
                             printMessage("boek geplaatst!");
                             delay(4000);
                             setupScreen();
                          }
                        else {
                          //door stays open if database fails
                              doorLock.write(doorOpen);
                              printMessage("db failed");
                          }
                        }
                   else {
                          doorLock.write(doorOpen);
                          printMessage("error");
                    }
                
                }
                else {}
                
              }
              //is there already a book in the safe? provide user with this feedback before resetting the loop
              else { 
              printMessage("helaas, er ligt al een boek.");
              delay(3000); 
              setupScreen();
               }
           
             
          } else {
            // code wasn't in either of the databases, reset loop
            printMessage("code ongeldig");
            delay(3000); 
            setupScreen();
          }
        }
     }
}

int httpRequest(String requestedUrl) {
  // String to save the response of the executed request
  String responseData="";

  // Connect to the server
  if (client.connect(server, 80)) {
    // send HTTP GET request:
    client.println("GET "+requestedUrl+" HTTP/1.1");
    client.println("Host: "+String(server));

    //user agent is mozilla instead of arduino to prevent firewall block from webpages server of Avans
    client.println("User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:65.0) Gecko/20100101 Firefox/65.0");
    client.println(); 

    // Receive the data from server per character and put this in a string
  
    Serial.println("Receiving data from: http://"+String(server)+requestedUrl);
    
    // wait till there is at least one character received
    Serial.print("Waiting for data from server....");
    int endOfHeaderChar = 0;
    while (responseData.length()==0) {    
      Serial.print(".");
      while (client.available()) {
        char c = client.read();
        Serial.println(String(c));

        // Skip headers of webpages (difference headers and data is made by pattern CR (\r) LF (\n) CR LF (13 10 13 10)) (detecting enters)
        if ((endOfHeaderChar == 0 && c == 13) || 
            (endOfHeaderChar == 1 && c == 10) ||
            (endOfHeaderChar == 2 && c == 13) || 
            (endOfHeaderChar == 3 && c == 10))   
              endOfHeaderChar++;
        else if (endOfHeaderChar != 4) {
          endOfHeaderChar = 0;
        } else {
          // add received character to received characters 
          responseData=responseData+String(c);
        } 
      }
    } 
    // Done with receiving data. Close connection with server 
    client.stop();
    Serial.println("Data ontvangen...");
    
    // Print received data
    Serial.println("\n\n<Begin ontvangen data>\n"+responseData+"\n<Einde ontvangen data>");

    // inlogresponse codes of API to see if code is in either of the databases
    if (responseData.indexOf("APIresponseCode 1")!=-1){
      return 1;
    } else if (responseData.indexOf("APIresponseCode 2")!=-1){
      return 2;
    } 
    //codes for adding code to database or deleting code from database
     else if (responseData.indexOf("APIresponseCode gelukt")!=-1){
      return 3;
    }
     else if (responseData.indexOf("APIresponseCode mislukt")!=-1){
      return 4;
    }else {
      // default
      return 0;
    }
  } else {
    // if connection fails return 0 (API result failed code)
    Serial.println("Connection failed");
    return 0;
  } 
}
