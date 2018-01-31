#define MY_RADIO_NRF24
#define MY_RF24_CHANNEL	81
#define MY_NODE_ID 10
//#define MY_DEBUG // Enables debug messages in the serial log
//#define NDEBUG
#define MY_BAUD_RATE 115200
#define MY_OTA_FIRMWARE_FEATURE

#define MY_TRANSPORT_WAIT_READY_MS 30000

#include <MySensors.h>  
#include <SPI.h>
#include "U8glib.h"
#include <SoftwareSerial.h>
#include <avr/wdt.h>
#include "EEPROMAnything.h"


#define SKETCH_NAME "CO2 meter"
#define SKETCH_MAJOR_VER "1"
#define SKETCH_MINOR_VER "13"



/*****************************************************************************************************/
/*                               				Common settings									      */
/******************************************************************************************************/
#define RADIO_RESET_DELAY_TIME 40 //Задержка между сообщениями
#define MESSAGE_ACK_RETRY_COUNT 2  //количество попыток отсылки сообщения с запросом подтверждения
#define DATASEND_DELAY  10

boolean gotAck=false; //подтверждение от гейта о получении сообщения 
int iCount = MESSAGE_ACK_RETRY_COUNT;

unsigned long ulLastReceivedMark=0;
unsigned long ulLastReceivedMills1 = 0;
unsigned long ulLastReceivedMills2 = 1;


#define WATCHDOGTIME 30000 // in ms
#define WARNBEEPFREQ 30000 // in ms
#define ALARMBEEPFREQ 1500 // in ms

#define SENSORHEATTIME 120 //in seconds

#define CRCCONTINOUSERRORS	5


#define CHILD_ID_CO2 0
#define REBOOT_CHILD_ID 100
#define CHILD_ID_NIGHTMODE 101
#define CHILD_ID_SPEAKER 102
#define CHILD_ID_ABC 103

SoftwareSerial mySerial(A0, A1); // A0 - к TX сенсора, A1 - к RX

byte cmd[9] = {0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79}; 
// Serial3.write("\xFF\x01\x87\x00\x00\x00\x00\x00\x78"); ZERO POINT CALIBRATION
// Serial3.write("\xFF\x01\x79\x00\x00\x00\x00\x00\x86"); ABC logic off
// Serial3.write("\xFF\x01\x79\xA0\x00\x00\x00\x00\xE6"); ABC logic on

byte ABCoff[9] = {0xFF,0x01,0x79,0x00,0x00,0x00,0x00,0x00,0x86}; 
byte ABCon[9] = {0xFF,0x01,0x79,0xA0,0x00,0x00,0x00,0x00,0xE6}; 


byte setrangeA_cmd[9] = {0xFF, 0x01, 0x99, 0x00, 0x00, 0x00, 0x13, 0x88, 0xCB}; // задаёт диапазон 0 - 5000ppm


unsigned char response[9];


U8GLIB_SH1106_128X64 u8g(U8G_I2C_OPT_DEV_0|U8G_I2C_OPT_FAST); // Dev 0, Fast I2C / TWI

boolean metric = true;                                                                                                                                                      
MyMessage msgCO2(CHILD_ID_CO2, V_LEVEL);  
MyMessage msgBuzzerState(CHILD_ID_SPEAKER, V_STATUS);


 unsigned int ppm = 0;


boolean bNightMode = false;
boolean bSpeakerOff = false;
boolean bABCstate = false;

boolean bCO2Warn = false;
boolean bCO2Alarm = false;

boolean bredrawMainScreen = false;

typedef struct {
	byte    ABCstate; //0  -off, 1 - on
	byte    MeasurementRange;	 //0 - 2000ppm, 1 - 5000ppm
	byte 	buzzerState; //0 - off, 1 - on
	byte    nightMode; //0 - off, 1 - on
	byte    shortRestart = 0;
} sensor_config;

bool bCRCcontErrors = 0;


sensor_config configSettings;


void before() 
{
 // This will execute before MySensors starts up 
//clearOLED();


//wait(2000);

//tone(5, 1000, 500);

u8g.begin();

/*
configSettings.ABCstate = 0;
configSettings.MeasurementRange = 1;
configSettings.buzzerState = 1;
configSettings.nightMode = 0;
EEPROM_writeAnything(0, configSettings);	
*/

EEPROM_readAnything(0, configSettings);



switch (configSettings.buzzerState) {
  case 0:
    	bSpeakerOff = true;
    break;
  case 1:
    	bSpeakerOff = false;
    break;
  default:
    	bSpeakerOff = false;
}


switch (configSettings.nightMode) {
  case 0:
    	bNightMode = false;
    break;
  case 1:
    	bNightMode = true;
    break;
  default:
    	bNightMode = false;
}

switch (configSettings.ABCstate) {
  case 0:
    	bABCstate = false;
    break;
  case 1:
    	bABCstate = true;
    break;
  default:
    	bABCstate = false;
}



if (configSettings.shortRestart != 1)
{
	drawNetStartMenu(0);
}




}

void presentation() 
{






					    iCount = MESSAGE_ACK_RETRY_COUNT;

	                    while( !sendSketchInfo(SKETCH_NAME, SKETCH_MAJOR_VER"."SKETCH_MINOR_VER) && iCount > 0 )
	                      {
	                         wait(RADIO_RESET_DELAY_TIME);
	                        iCount--;
	                       }

						wait(RADIO_RESET_DELAY_TIME);



					    iCount = MESSAGE_ACK_RETRY_COUNT;

	                    while( !present(CHILD_ID_CO2, S_AIR_QUALITY, "CO2 level") && iCount > 0 )
	                      {
	                         wait(RADIO_RESET_DELAY_TIME);
	                        iCount--;
	                       }

						wait(RADIO_RESET_DELAY_TIME);



					    iCount = MESSAGE_ACK_RETRY_COUNT;

	                    while( !present(REBOOT_CHILD_ID, S_BINARY, "Reboot sensor") && iCount > 0 )
	                      {
	                         wait(RADIO_RESET_DELAY_TIME);
	                        iCount--;
	                       }

						wait(RADIO_RESET_DELAY_TIME);


					    iCount = MESSAGE_ACK_RETRY_COUNT;

	                    while( !present(CHILD_ID_NIGHTMODE, S_BINARY, "Night mode") && iCount > 0 )
	                      {
	                         wait(RADIO_RESET_DELAY_TIME);
	                        iCount--;
	                       }

						wait(RADIO_RESET_DELAY_TIME);


					    iCount = MESSAGE_ACK_RETRY_COUNT;

	                    while( !present(CHILD_ID_SPEAKER, S_BINARY, "Buzzer") && iCount > 0 )
	                      {
	                         wait(RADIO_RESET_DELAY_TIME);
	                        iCount--;
	                       }

						wait(RADIO_RESET_DELAY_TIME);


					    iCount = MESSAGE_ACK_RETRY_COUNT;

	                    while( !present(CHILD_ID_ABC, S_BINARY, "ABC") && iCount > 0 )
	                      {
	                         wait(RADIO_RESET_DELAY_TIME);
	                        iCount--;
	                       }

						wait(RADIO_RESET_DELAY_TIME);






					    iCount = MESSAGE_ACK_RETRY_COUNT;

	                    while( !request(CHILD_ID_SPEAKER, V_STATUS) && iCount > 0 )
	                      {
	                         wait(RADIO_RESET_DELAY_TIME);
	                        iCount--;
	                       }

    
					    iCount = MESSAGE_ACK_RETRY_COUNT;

	                    while( !request(CHILD_ID_NIGHTMODE, V_STATUS) && iCount > 0 )
	                      {
	                         wait(RADIO_RESET_DELAY_TIME);
	                        iCount--;
	                       }


 
    



    //request(CHILD_ID_ABC, V_STATUS);

if (configSettings.shortRestart != 1)
{
  	wait(7000); 
}
else
{
	wait(2000);
}

  	//Enable watchdog timer
  	wdt_enable(WDTO_8S); 

}


void setup() {
  Serial.begin(115200);
  mySerial.begin(9600);

    //myOLED.begin();



/* Set measurement range to 0 -5000 ppm */

//wait(2000);
  setRange(5000);

  wait(1000);

/* Switch ABC*/

switch (bABCstate) {
  case 0:
    	ABC_off();
    break;
  case 1:
    	ABC_on();
    break;
  default:
    	ABC_off();
}


 //ABC_off();


  //u8g.setContrast(1);

if (configSettings.shortRestart != 1)
{

	  	char bufi[9];

			for (int i=0; i<=SENSORHEATTIME; i++)
			{
				wait(1000);
				sprintf (bufi, "%d", (SENSORHEATTIME - i));	
				drawStartupMessage(bufi);					

			}

	    	configSettings.shortRestart = 0;
	    	EEPROM_writeAnything(0, configSettings);  

}



}

void loop() 
{

	getCO2Level();

	beepWarn();


	beepAlarm();


	if ( bredrawMainScreen )
	{
		bredrawMainScreen = false;
		drawMainScreen();		
	}

  //reset watchdog timer
  wdt_reset();   
}




void getCO2Level()
{

	  unsigned long currentMillis = millis();

  if( (currentMillis - ulLastReceivedMark) > WATCHDOGTIME )
  {

	ulLastReceivedMark = currentMillis;

	  mySerial.write(cmd, 9);
	  memset(response, 0, 9);
	  mySerial.readBytes(response, 9);
	  int i;
	  byte crc = 0;
	  for (i = 1; i < 8; i++) crc+=response[i];
	  crc = 255 - crc;
	  crc++;

	  if ( !(response[0] == 0xFF && response[1] == 0x86 && response[8] == crc) ) {
	    Serial.println("CRC error: " + String(crc) + " / "+ String(response[8]));
	    bCRCcontErrors++;

	    if ( bCRCcontErrors >= CRCCONTINOUSERRORS )
	    {
	    	configSettings.shortRestart = 1;
	    	EEPROM_writeAnything(0, configSettings);  
	    	bCRCcontErrors = 0;
	    	 while(1) {                 
              			};
	    }

	  } else {
	    unsigned int responseHigh = (unsigned int) response[2];
	    unsigned int responseLow = (unsigned int) response[3];
	    ppm = (256*responseHigh) + responseLow;
	    //ppm = (((256*responseHigh) + responseLow) / 5) * 2;

	            		#ifdef NDEBUG      
	    					Serial.println(ppm);
	    				#endif	

					    iCount = MESSAGE_ACK_RETRY_COUNT;

	                    while( !send(msgCO2.set(ppm,0)) && iCount > 0 )
	                      {

	                         wait(RADIO_RESET_DELAY_TIME);
	                        iCount--;
	                       }

	                      //gotAck = false; 



	drawMainScreen();

	bCRCcontErrors = 0;

	  }


}



}



void drawMainScreen()
{
//Serial.println("Speaker state: " + String(bSpeakerOff) );
	u8g.firstPage();  
	  do {
	  	char buf[9];
		sprintf (buf, "%d", ppm);
		
		u8g.setFont(u8g_font_fub35n); // u8g_font_fub42n // 35
		u8g.setFontPosCenter();
	    
	    if (ppm > 999)
	    {
	    	u8g.drawStr(3, 40, buf); // 10,30
	    }
	    else
		{
	    	u8g.drawStr(20, 40, buf); // 10,30

		}

	  	u8g.setFont(u8g_font_fixed_v0r); // u8g_font_tpssbr
	  	if (ppm < 801)
	  	{
	  		u8g.drawStr(1, 9, "CO  GOOD"); // 10,30	  	
	  		bCO2Warn = false;	
	  		bCO2Alarm = false;
	  	}
	  	else if ( ppm >800 && ppm < 1001 )
	  	{
	  		u8g.drawStr(1, 9, "CO  NEAR GOOD"); // 10,30	
	  		bCO2Warn = false;  	
	  		bCO2Alarm = false;	
	  	}
	  	else if ( ppm >1000 && ppm < 1401 )
	  	{
	  		u8g.drawStr(1, 9, "CO  ACCEPT"); // 10,30	
	  		bCO2Warn = true;  		
	  		bCO2Alarm = false;
	  	}
	  	else if ( ppm >1400 )
	  	{
	  		u8g.drawStr(1, 9, "CO  BAD"); // 10,30	
	  		bCO2Warn = false;  	
	  		bCO2Alarm = true;	
	  	}


	  	//u8g.setFont(u8g_font_tpssbr); // u8g_font_fixed_v0r
	  	u8g.drawStr(13, 12, "2"); // 10,30


	  	u8g.setFont(u8g_font_unifont_76); // u8g_font_tpssbr
		u8g.drawStr(119, 10, " "); // 10,30

	  	if (!bSpeakerOff)
	  	{
	  		u8g.drawStr(105, 9, ")"); // 10,30
	  	}

	  	if (bNightMode)
		{
	  		u8g.drawStr(77, 10, "]"); // 10,30
		}

		if (!bABCstate)
		{
	  		u8g.drawStr(91, 9, "0"); // 10,30			
		}
		else if (bABCstate)
		{
	  		u8g.drawStr(91, 9, "2"); // 10,30				
		}

	  	//u8g.setFont(u8g_font_6x10r); // u8g_font_fub42n // 35
		//u8g.setFontPosCenter();
	    //u8g.drawStr(10, 60, "Acceptable level"); // 10,30
	  } while( u8g.nextPage() );

}



void drawStartupMessage( char *str)
{
	u8g.firstPage();  
    do {

	  	u8g.setFont(u8g_font_unifont_76); // u8g_font_tpssbr
		u8g.drawStr(119, 10, " "); // 10,30

	  	if (!bSpeakerOff)
	  	{
	  		u8g.drawStr(105, 9, ")"); // 10,30
	  	}

	  	if (bNightMode)
		{
	  		u8g.drawStr(77, 10, "]"); // 10,30
		}

		if (!bABCstate)
		{
	  		u8g.drawStr(91, 9, "0"); // 10,30			
		}
		else if (bABCstate)
		{
	  		u8g.drawStr(91, 9, "2"); // 10,30				
		}

	  	u8g.setFont(u8g_font_fixed_v0r); // u8g_font_tpssbr

	  	u8g.drawStr(1, 9, "CO"); // 10,30	  		
	  	u8g.drawStr(13, 12, "2"); // 10,30	  	

	  	u8g.drawStr(20, 35, "HEATING SENSOR"); // 10,30	  
	  	u8g.drawStr(57, 49, str); // 10,30




    } while( u8g.nextPage() );
}

void clearOLED(){
    u8g.firstPage();  
    do {
    } while( u8g.nextPage() );
}


void drawNetStartMenu( int iStage)
{

	u8g.firstPage();  
    do {

	  	u8g.setFont(u8g_font_unifont_76); // u8g_font_tpssbr
		u8g.drawStr(119, 10, " "); // 10,30

	  	if (!bSpeakerOff)
	  	{
	  		u8g.drawStr(105, 9, ")"); // 10,30
	  	}

	  	if (bNightMode)
		{
	  		u8g.drawStr(77, 10, "]"); // 10,30
		}

		if (!bABCstate)
		{
	  		u8g.drawStr(91, 9, "0"); // 10,30			
		}
		else if (bABCstate)
		{
	  		u8g.drawStr(91, 9, "2"); // 10,30				
		}

	  	u8g.setFont(u8g_font_fixed_v0r); // u8g_font_tpssbr

	  	u8g.drawStr(1, 9, "CO"); // 10,30	  		
	  	u8g.drawStr(13, 12, "2"); // 10,30	
	  	u8g.drawStr(10, 35, "STARTING MYSENSORS"); // 10,30	  
	  	u8g.drawStr(43, 49, "NETWORK"); // 10,30	 


	  	if (iStage == 1)
	  	{
	  	   u8g.drawStr(10, 59, "RANGE SET"); // 10,30	
	  	}

	  	if (iStage == 2)
	  	{
	  	   u8g.drawStr(10, 59, "RANGE SET, ABC SET"); // 10,30	

	  	}


    } while( u8g.nextPage() );

}


void beepWarn()
{


	  unsigned long currentMillis = millis();


  if( (currentMillis - ulLastReceivedMills1) > WARNBEEPFREQ )
  {

  		ulLastReceivedMills1 = currentMillis;


  		if ( !bSpeakerOff && bCO2Warn)
  		{
  			tone(5, 1000, 400);
  		}


  }



}

void beepAlarm()
{


	  unsigned long currentMillis = millis();


  if( (currentMillis - ulLastReceivedMills2) > ALARMBEEPFREQ )
  {

  		ulLastReceivedMills2 = currentMillis;


  		if ( !bSpeakerOff && bCO2Alarm)
  		{
  			tone(5, 1000, 500);
  		}


  }


}



void ABC_off()
{


	  int i;
	  byte crc = 0;

	boolean bDone = false;
	int iCycleCount = 0;

	do{
		  crc = 0;
	  	  mySerial.write(ABCoff, 9);
		  memset(response, 0, 9);
		  mySerial.readBytes(response, 9);
		  for (i = 1; i < 8; i++) crc+=response[i];
		  crc = 255 - crc;
		  crc++;

		  if ( !(response[0] == 0xFF && response[1] == 0x79 && response[8] == crc) ) {
		    Serial.println("ABCoff CRC error: " + String(crc) + " / "+ String(response[8]));
		    iCycleCount++;
		    wait(1000);
		  } else {
		  		bDone = true;
		  		iCycleCount = 4;
		  		drawNetStartMenu(2);
		  		tone(5, 1000, 200);
		  		delay(200);
		  		tone(5, 1000, 200);	  		
		  }
	} while ( !bDone || iCycleCount < 4 ); //|| iCycleCount < 4


	if (!bDone)
	{
		while (1)
		{
			u8g.firstPage();  
		    do {

			  	u8g.setFont(u8g_font_unifont_76); // u8g_font_tpssbr
				u8g.drawStr(119, 10, " "); // 10,30

			  	if (!bSpeakerOff)
			  	{
			  		u8g.drawStr(105, 9, ")"); // 10,30
			  	}

			  	if (bNightMode)
				{
			  		u8g.drawStr(77, 10, "]"); // 10,30
				}

				if (!bABCstate)
				{
			  		u8g.drawStr(91, 9, "0"); // 10,30			
				}
				else if (bABCstate)
				{
			  		u8g.drawStr(91, 9, "2"); // 10,30				
				}

			  	u8g.setFont(u8g_font_fixed_v0r); // u8g_font_tpssbr

			  	u8g.drawStr(1, 9, "CO"); // 10,30	  		
			  	u8g.drawStr(13, 12, "2"); // 10,30	
			  	u8g.drawStr(10, 35, "ERROR SET ABC"); // 10,30	   


		    } while( u8g.nextPage() );
		}

	}


}


void ABC_on()
{
	  int i;
	  byte crc = 0;

	boolean bDone = false;
	int iCycleCount = 0;

	do{
		  crc = 0;
	  	  mySerial.write(ABCon, 9);
		  memset(response, 0, 9);
		  mySerial.readBytes(response, 9);
		  for (i = 1; i < 8; i++) crc+=response[i];
		  crc = 255 - crc;
		  crc++;

		  if ( !(response[0] == 0xFF && response[1] == 0x79 && response[8] == crc) ) {
		    Serial.println("ABCon CRC error: " + String(crc) + " / "+ String(response[8]));
		    iCycleCount++;
		    wait(1000);		    
		  } else {
		  		bDone = true;
		  		iCycleCount = 4;		  		
		  		drawNetStartMenu(2);		  		
		  		tone(5, 1000, 300);
		  }
	} while ( !bDone || iCycleCount < 4);

	if (!bDone)
	{
		while (1)
		{
			u8g.firstPage();  
		    do {

			  	u8g.setFont(u8g_font_unifont_76); // u8g_font_tpssbr
				u8g.drawStr(119, 10, " "); // 10,30

			  	if (!bSpeakerOff)
			  	{
			  		u8g.drawStr(105, 9, ")"); // 10,30
			  	}

			  	if (bNightMode)
				{
			  		u8g.drawStr(77, 10, "]"); // 10,30
				}

				if (!bABCstate)
				{
			  		u8g.drawStr(91, 9, "0"); // 10,30			
				}
				else if (bABCstate)
				{
			  		u8g.drawStr(91, 9, "2"); // 10,30				
				}

			  	u8g.setFont(u8g_font_fixed_v0r); // u8g_font_tpssbr

			  	u8g.drawStr(1, 9, "CO"); // 10,30	  		
			  	u8g.drawStr(13, 12, "2"); // 10,30	
			  	u8g.drawStr(10, 35, "ERROR SET ABC"); // 10,30	   


		    } while( u8g.nextPage() );
		}

	}


}



void setRange(int iRange)
{

		  int i;
		  byte crc = 0;

		  boolean bDone = false;
		  int iCycleCount = 0;


	if (iRange == 5000)
	{


		  do{
		  	  crc = 0;
			  mySerial.write(setrangeA_cmd, 9);
			  memset(response, 0, 9);
			  mySerial.readBytes(response, 9);
			  for (i = 1; i < 8; i++) crc+=response[i];
			  crc = 255 - crc;
			  crc++;

			  if ( !(response[0] == 0xFF && response[1] == 0x99 && response[8] == crc) ) {
			    Serial.println("setrangeA_cmd CRC error: " + String(crc) + " / "+ String(response[8]));
			    iCycleCount++;
		    	wait(1000);			    
			  } else {
			  		bDone = true;
			  		iCycleCount = 4;
		  		    drawNetStartMenu(1);			  		
			  		tone(5, 1000, 300);	  		
			  }	
		  } while ( !bDone  || iCycleCount < 4 ); //|| iCycleCount < 4
		  
	
	}
	else if (iRange == 2000)
	{

	}


	if (!bDone)
	{
		while (1)
		{
			u8g.firstPage();  
		    do {

			  	u8g.setFont(u8g_font_unifont_76); // u8g_font_tpssbr
				u8g.drawStr(119, 10, " "); // 10,30

			  	if (!bSpeakerOff)
			  	{
			  		u8g.drawStr(105, 9, ")"); // 10,30
			  	}

			  	if (bNightMode)
				{
			  		u8g.drawStr(77, 10, "]"); // 10,30
				}

				if (!bABCstate)
				{
			  		u8g.drawStr(91, 9, "0"); // 10,30			
				}
				else if (bABCstate)
				{
			  		u8g.drawStr(91, 9, "2"); // 10,30				
				}

			  	u8g.setFont(u8g_font_fixed_v0r); // u8g_font_tpssbr

			  	u8g.drawStr(1, 9, "CO"); // 10,30	  		
			  	u8g.drawStr(13, 12, "2"); // 10,30	
			  	u8g.drawStr(10, 30, "ERROR SET RANGE"); // 10,30	   


		    } while( u8g.nextPage() );
		}

	}

}



/*
void ShowInfo()
{
  oled.begin(&Adafruit128x64, I2C_ADDRESS);
  oled.setFont(Arial14);  
 
  oled.setFont(Arial14);
  oled.set1X();
  oled.setCursor(20,1);
  oled.write("* MySensors 2.0 *");
  oled.setCursor(30,3);
  oled.write(SketchName);
  oled.write(" ");
  oled.write(SketchVersion);
  oled.setCursor(30,5);
  oled.write("NodeID");
  oled.write(" ");
  oled.print(getNodeId());
  sleep(3000); 
  oled.clear();
}
*/

	

void receive(const MyMessage &message) 
{
 // Handle incoming message 

  //if (message.isAck())
  //{
  //  gotAck = true;
  //  return;
  //}

    if ( message.sensor == REBOOT_CHILD_ID && message.getBool() == true && strlen(message.getString())>0 ) {
    	   	  #ifdef NDEBUG      
      			Serial.println("Received reboot message");
   	  			#endif    
             //wdt_enable(WDTO_30MS);
              while(1) { 
                
              };

     return;         

     }


    if ( message.sensor == CHILD_ID_SPEAKER && strlen(message.getString())>0 ) {

    	if (message.getBool() == true)
    	{
    		bSpeakerOff = true;
    		configSettings.buzzerState = 0;
    	}
    	else if (message.getBool() == false)
    	{
    		bSpeakerOff = false;
    		configSettings.buzzerState = 1;
    	}

    	EEPROM_writeAnything(0, configSettings);  

    	bredrawMainScreen = true; 

     return;    		
     }


    if ( message.sensor == CHILD_ID_NIGHTMODE && strlen(message.getString())>0 ) {

    	if (message.getBool() == true)
    	{
    		bNightMode = true;
    		configSettings.nightMode = 1;
    		u8g.setContrast(1);
    		bSpeakerOff = true;
    		configSettings.buzzerState = 0;    		
    	}
    	else if (message.getBool() == false)
    	{
    		bNightMode = false;
    		configSettings.nightMode = 0;
    		u8g.setContrast(255);
     		bSpeakerOff = false;
    		configSettings.buzzerState = 1;   		  
    	}

    	EEPROM_writeAnything(0, configSettings);  
    	bredrawMainScreen = true;

					    iCount = MESSAGE_ACK_RETRY_COUNT;

	                    while( !send(msgBuzzerState.set(bSpeakerOff,0)) && iCount > 0 )
	                      {
	                        
	                        wait(RADIO_RESET_DELAY_TIME);
	                        iCount--;
	                       }

	                      //gotAck = false; 

     return;
     }

    if ( message.sensor == CHILD_ID_ABC && strlen(message.getString())>0 ) {

    	if (message.getBool() == true)
    	{
    		bABCstate = true;
    		configSettings.ABCstate = 1;
    		ABC_on();
    	}
    	else if (message.getBool() == false)
    	{
    		bABCstate = false;
    		configSettings.ABCstate = 0;
    		ABC_off();
    	}

    	EEPROM_writeAnything(0, configSettings);
     }
     return;


 }


