/////////////////////////////////////////////////////////////////////////
////                                                                 ////
////  Joystick.c                                            ////
////  Joystick 8 ejes 24 botones                                   ////
////  Autor: Alejandro                                                     ////
////                                                                 ////
////  02/05/2010                                                    ////
////                                                                 ////
/////////////////////////////////////////////////////////////////////////

#include <18F2455.h>
#device ADC = 10
#fuses HSPLL,NOWDT,NOPROTECT,NOLVP,NODEBUG,USBDIV,PLL5,CPUDIV1,VREGEN,PUT
#use delay(clock=48000000)
#use fast_io(A)
#use fast_io(b)
#use fast_io(c)


#DEFINE USB_HID_DEVICE  TRUE

#define USB_EP1_TX_ENABLE  USB_ENABLE_INTERRUPT   //turn on EP1 for IN bulk/interrupt transfers
#define USB_EP1_TX_SIZE 20

#define USB_EP1_RX_ENABLE  USB_ENABLE_INTERRUPT   //turn on EP1 for IN bulk/interrupt transfers
#define USB_EP1_RX_SIZE 8

#include <pic18_usb.h>
#include "PPM_USB_HID3.h"              //USB Configuration and Device descriptors for this UBS device
#include <usb.c>                                 //handles usb setup tokens and get descriptor reports

/////////////////////////////////////////////////////////////////////////////
//
// Defines y Zarandajas
//
/////////////////////////////////////////////////////////////////////////////
   int8 TxUSB[20];


#define LED_ON  output_high
#define LED_OFF output_low



/////////////////////////////////////////////////////////////////////////////
//
// RAM
//
/////////////////////////////////////////////////////////////////////////////

int8 connected;
int8 enumerated;
int8 rx_msg[USB_EP1_RX_SIZE];
int8 tx_msg[22]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};


char NextChar='0';
int1 fTX=0;
int1 fInicio=0;
int1 fLeer=0;

int8 i;
int8 rNroCanal=0;
int8 rTiempo=0;
int8 txlen=0;
int8 txpoint=0;

int8 rAntBot1a8=0;   //estado anterior de los botones
int8 FpBtn1a8=0;  //Flacos positivos
#bit fp1  =  FpBtn1a8.0
#bit fp2  =  FpBtn1a8.1
#bit fp3  =  FpBtn1a8.2
#bit fp4  =  FpBtn1a8.3
#bit fp5  =  FpBtn1a8.4
#bit fp6  =  FpBtn1a8.5
#bit fp7  =  FpBtn1a8.6
#bit fp8  =  FpBtn1a8.7

int8 rEncDir=0; //Deteccuion de la direccion de los encoders
#bit Dir1 = rEncDir.0
#bit Dir2 = rEncDir.1
#bit Dir3 = rEncDir.2
#bit Dir4 = rEncDir.3
#bit Dir5 = rEncDir.4
#bit Dir6 = rEncDir.5
#bit Dir7 = rEncDir.6
#bit Dir8 = rEncDir.7



int8  Eje0=0;
int16 Eje1=0;
int16 Eje2=0;
int16 Eje3=0;
int16 Eje4=0;
int16 Eje5=0;
int16 Eje6=0;
int16 Eje7=0;
int16 Eje8=0;
int8 Btn1a8=0;    //Encoders
int8 Btn9a16=0;   //Encoders
int8 Btn17a24=0;  //Ultimos 8 borones
int8 Btn24a32=0;
//Registros enviados por USB 24 Botones


#bit Btn1   =  btn1a8.0
#bit Btn2   =  btn1a8.1
#bit Btn3   =  btn1a8.2
#bit Btn4   =  btn1a8.3
#bit Btn5   =  btn1a8.4
#bit Btn6   =  btn1a8.5
#bit Btn7   =  btn1a8.6
#bit Btn8   =  btn1a8.7
#bit Btn9   =  btn9a16.0
#bit Btn10   =  btn9a16.1
#bit Btn11   =  btn9a16.2
#bit Btn12   =  btn9a16.3
#bit Btn13   =  btn9a16.4
#bit Btn14   =  btn9a16.5
#bit Btn15   =  btn9a16.6
#bit Btn16   =  btn9a16.7
/////////////////////////////////////////////////////////////////////////////
//
// usb_debug_task()
//
// When called periodically, displays debugging information over serial
// to display enumeration and connection states.  Also lights LED2 and LED3
// based upon enumeration and connection status.
//
/////////////////////////////////////////////////////////////////////////////

void usb_debug_task(void) {

   enumerated=usb_enumerated();

}





/////////////////////////////////////////////////////////////////////////////
//
// usb_rx_task()
//
// Listens to EP1 for any incoming packets.  The only report ID that is
// configurd to send us data is 2 (keyboard LED status, see above)
//
/////////////////////////////////////////////////////////////////////////////

void usb_rx_task(void){


}


#int_RTCC
void RTCC_isr() { 
fleer=1;
}



#int_TIMER1   //Timer 1 overflow
void TIMER1_int(){

}


void main() {
   int8 Trim[2]; 
   int8 Buffer; 

   set_tris_a(0b11101111);
   set_tris_b(0b11111111);
   set_tris_c(0b00110111);
   port_b_pullups(false);
   setup_adc_ports( AN0_TO_AN10 );
   setup_adc(ADC_CLOCK_INTERNAL );


  
   setup_timer_0(RTCC_INTERNAL|RTCC_DIV_128|RTCC_8_BIT);


   delay_ms(500);

   usb_init_cs();


   disable_interrupts(INT_TIMER1);
   disable_interrupts(int_tbe);
   enable_interrupts(INT_TIMER0);
   enable_interrupts(global);

   set_adc_channel(1);


while (TRUE) 
   { 
      int8 rTemp;
      int8 temp=0;
      int8 temp1=0;
      
      usb_task();
      usb_debug_task();
      
      if(fLeer){
               fLeer=0;
               set_adc_channel(0);  //PREPARO PARA PRIMER EJE
               //LEO DIGITALES
               output_low(pin_c7);
               output_low(pin_c6);
               output_high(pin_a4); //prepara para leer primeros  8 botones
               delay_us(5);
               temp= input_b();
               temp1=input_c();
               
               output_low(pin_a4);
               output_low(pin_c7);
               output_high(pin_c6); //prepara para leer segundos  8 botones
               
               //leo los 8 botones
               temp=temp & 0b11110001;
               temp1=(temp1 & 0b00000111)*2;
               Btn1a8 = temp | temp1;
               
/*               #asm comf rAntBot1a8   //Estado anterior de los botones
               #endasm
               FpBtn1a8 = rTemp & rAntBot1a8;   //Flancos positivos de los botones
               
               rAntBot1a8 = rTemp;   //igualo estado actual con anterior
               btn7=0;
               btn8=0;
               if(fp3 & bit_test(rtemp,2))btn7=1;
               if(fp3 & !bit_test(rtemp,2))btn8=1;
*/
               temp= input_b();
               temp1=input_c();
               output_low(pin_a4);
               output_low(pin_c6);
               output_high(pin_c7); //prepara para leer ultimos 8 botones
               
               //leo los otros 8 botones
               temp=temp & 0b11110001;
               temp1=(temp1 & 0b00000111)*2;
               Btn9a16 = temp | temp1;
/*
//Una vez obtenido el flanco y la direccion seteo los botones
               //Reseteo todos los botones del 1 al 16
               btn1a8=0;
               btn9a16=0;
               //Encoder 1
               if(fp1 & Dir1) Btn1=1;
               if(fp1 & !Dir1)Btn2=1;
               //Encoder 2
               if(fp2 & Dir2) Btn3=1;
               if(fp2 & !Dir2)Btn4=1;
               //Encoder 3
               if(fp3 & Dir3) Btn5=1;
               if(fp3 & !Dir3)Btn6=1;               
               //Encoder 4
               if(fp4 & Dir4) Btn7=1;
               if(fp4 & !Dir4)Btn8=1;              
               //Encoder 5
               if(fp5 & Dir5) Btn9=1;
               if(fp5 & !Dir5)Btn10=1;               
               //Encoder 6
               if(fp6 & Dir6) Btn11=1;
               if(fp6 & !Dir6)Btn12=1;               
               //Encoder 7
               if(fp7 & Dir7) Btn13=1;
               if(fp7 & !Dir7)Btn14=1;               
               //Encoder 8
               if(fp8 & Dir8) Btn15=1;
               if(fp8 & !Dir8)Btn16=1;               
*/               
               //leo ultimos ultimos 8 botones
               temp= input_b();
               temp1=input_c();
               temp=temp & 0b11110001;
               temp1=(temp1 & 0b00000111)*2;
               Btn17a24 = temp | temp1;
               
               //LEO ANALOGICAS
               Eje1=read_adc();
               set_adc_channel(1);  //PREPARO PARA EJE 2
               delay_us(10);
               Eje2=read_adc();
               set_adc_channel(2);  //PREPARO PARA EJE 3
               delay_us(10);
               Eje3=read_adc();
               set_adc_channel(3);  //PREPARO PARA EJE 4
               delay_us(10);
               Eje4=read_adc();
               set_adc_channel(4);  //PREPARO PARA EJE 5
               delay_us(10);
               Eje5=read_adc();
               set_adc_channel(9);  //PREPARO PARA EJE 6
               delay_us(10);
               Eje6=read_adc();
               set_adc_channel(8);  //PREPARO PARA EJE 7
               delay_us(10);
               Eje7=read_adc();
               set_adc_channel(10);  //PREPARO PARA EJE 8
               delay_us(10);
               Eje8=read_adc();
               fTx=1;
               
               }

      if(usb_enumerated())             //si esta configurado el Joysick
      { 
               if (fTX){
                     txusb[0]=*(&eje0 + 1);
                     txusb[1]=*(&eje0 + 2);
                     txusb[2]=*(&eje0 + 3);
                     txusb[3]=*(&eje0 + 4);
                     txusb[4]=*(&eje0 + 5);
                     txusb[5]=*(&eje0 + 6);
                     txusb[6]=*(&eje0 + 7);
                     txusb[7]=*(&eje0 + 8);
                     txusb[8]=*(&eje0 + 9);
                     txusb[9]=*(&eje0 + 10);
                     txusb[10]=*(&eje0 + 11);
                     txusb[11]=*(&eje0 + 12);
                     txusb[12]=*(&eje0 + 13);
                     txusb[13]=*(&eje0 + 14);
                     txusb[14]=*(&eje0 + 15);
                     txusb[15]=*(&eje0 + 16);
                     txusb[16]=Btn1a8;
                     txusb[17]=Btn9a16;
                     txusb[18]=Btn17a24;
                     usb_put_packet(  1,  &eje0+1, 20, USB_DTS_TOGGLE);
                     fTX=0; 
                     delay_ms(10);
               }
            } 

     } 
}

