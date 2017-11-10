#INCLUDE "16F887.h"
#FUSES      NOWDT,PUT,HS,PROTECT,NOLVP
#USE DELAY (CLOCK = 20M)
#INCLUDE "lcd_16x2.c"
#include "ds13b07.c"

#define backLight       pin_c7
#define buzzer          pin_e2
#define RF_input        pin_c0
#define rst_pass        pin_a0
#define pt2272          pin_a1
#define addr_hrs_on     0x00     
#define addr_min_on     0x01
#define addr_hrs_off    0x02     
#define addr_min_off    0x03
#define addr_flag_alarm 0x04
#define addr_first_pass 0x10
#define addr_flag_pass  0x15
#define flag_pass       0x11   
#define addr_flag_ds1307 0x3f
#define flag_ds1307      0x11
//DINH NGHIA BUTTON PORT====================
#define r1        pin_b0      //hang 1
#define r2        pin_b1      //hang 2
#define r3        pin_b2      //hang 3
#define r4        pin_b3      //hang 4
#define c1        pin_b4      //cot 1
#define c2        pin_b5      //cot 2
#define c3        pin_b6      //cot 3
#define c4        pin_b7      //cot 4
//KHAI BAO BIEN========================
char keyCode[]={"741*8520963#ABCD"};
INT1 flag_alert;                    // trang thai bao dong
INT1 flag_on_off_pt2272;            // trang thai bat/tat bao dong
int1 ena_on_off_auto;               // cho phep bat tat bao dong
unsigned int8 timerToResetPass;     // dem thoi gian reset
unsigned INT8 key_value;            // chua gia tri keypad
unsigned INT8 timerToExit;          // dem thoi gian de thoat
unsigned INT8 timerToAlert;         // dem thoi gian de bao dong
unsigned INT8 mode_value;  
signed INT8 hrs_on, min_on, sec_on, hrs_off, min_off, sec_off;
signed int8 i;
signed int8 hrs, min, sec, sec_temp;  
char pass[4], passTemp[4],passConfirm[4];

//KHAI BAO CHUONG TRINH CON=============
int8 readkeypad();
int1 checkRealTime();               
void homeScreen(byte hrs,min,sec);
void menuScreen();
void settingScreen();
void timeScreen();
void alarmScreen();
void sysTime();
void on_off_pt2272();
void on_off_pt2272_auto();                   
int1 isPassOk();                     
void alarmSet();                     
void showAlarm();            
void blinkMode_alarm();           
void blinkMode_sysTime();            
void passInit();                     
void getPass();  
void passChange();
int1 comfirmPassword();

//CHUONG TRINH CHINH===================
void main(){
   set_tris_a (0x01);
   set_tris_b (0xf0);
   set_tris_c (0x7f);
   set_tris_d (0x00);
   set_tris_e (0x07);
   
   lcd_setup ();  
   output_low (buzzer);
   output_low (backlight);
   
   passInit();
   ds1307_init ();
   IF (ds1307_read_byte (addr_flag_ds1307)!=flag_ds1307){   
      ds1307_set_date_time (0,0,0,0, 8, 11, 00) ;
      ds1307_write_byte (addr_flag_ds1307, flag_ds1307) ;
   }
   ds1307_get_time(hrs,min,sec);
   sec_temp = sec;
   
   flag_alert=0;
   flag_on_off_pt2272 = 0;
   timerToExit = 0;
   timerToAlert = 0;
   timerToResetPass = 0;
   ena_on_off_auto = 0;
   
   WHILE (true){
      IF (checkRealTime () ){
         homescreen (hrs,min,sec);
         lcd_GOTO_xy (1, 0); lcd_data ("Alert:  ") ;
         IF (flag_on_off_pt2272) lcd_data ("ON ") ;
         ELSE lcd_data ("OFF");
         on_off_pt2272_auto ();
      }

         IF (flag_on_off_pt2272==1){         //kiem tra moddule pt2272 co duoc bat
            IF ( (flag_alert==0)&& !input (RF_input)){
               delay_ms (20);
               IF (!input(RF_input)){flag_alert=1; timerToAlert=0;}
            }
            else IF ( (flag_alert==0)&& input (RF_input)) timerToAlert=0;
            else IF ( (flag_alert==1)&& (timerToAlert>5) ){
               lcd_command (0x01); delay_ms (2) ;
               lcd_GOTO_xy (0, 4); lcd_data ("**ALERT**") ;
               output_high (buzzer);
               do{
                  if(isPassOK()) {
                     on_off_pt2272();
                     output_low(buzzer);      
                  }
               }WHILE (flag_alert==1) ;
            }
         }
       
      key_value=readkeypad  ()  ;
      if (key_value!=0xff) {
         output_high (backLight);
         timerToExit=0;
         IF (key_value==12) { 
            if(isPassOK()) menuScreen (); 
         }
      }
      if(!input(rst_pass)){
            if(timerToResetPass>10) {
               write_eeprom(addr_flag_pass,0xff);
               write_eeprom(addr_flag_alarm,0xff);
               reset_cpu();
            }
      }else timerToResetPass=0;
   }
}
//CHUONG TRINH CON=====================
//CHUONG TRINH QUET KEY================
//   [7][8][9][A]
//   [4][5][6][B]
//   [1][2][3][C]
//   [*][0][#][D]

int readkeypad (){
   output_bit (c1, 1) ;
   output_bit (c2, 1) ;
   output_bit (c3, 1) ;
   output_bit (c4, 1) ;
   output_bit (c1, 0); 
   delay_ms (5);
   IF (!input  (r1)) {delay_ms(20); if(!input(r1)) return 0;}//7
   IF (!input  (r2)) {delay_ms(20); if(!input(r2)) return 1;}//4
   IF (!input  (r3)) {delay_ms(20); if(!input(r3)) return 2;}//1
   IF (!input  (r4)) {delay_ms(20); if(!input(r4)) return 3;}//*
   output_bit (c1, 1); 
   output_bit (c2, 0); 
   delay_ms (5);
   IF (!input  (r1)) {delay_ms(20); if(!input(r1)) return 4;}//8
   IF (!input  (r2)) {delay_ms(20); if(!input(r2)) return 5;}//5
   IF (!input  (r3)) {delay_ms(20); if(!input(r3)) return 6;}//2
   IF (!input  (r4)) {delay_ms(20); if(!input(r4)) return 7;}//0
   output_bit (c2, 1);
   output_bit (c3, 0); 
   delay_ms (5);
   IF (!input  (r1)) {delay_ms(20); if(!input(r1)) return 8;}//9
   IF (!input  (r2)) {delay_ms(20); if(!input(r2)) return 9;}//6
   IF (!input  (r3)) {delay_ms(20); if(!input(r3)) return 10;}//3
   IF (!input  (r4)) {delay_ms(20); if(!input(r4)) return 11;}//#
   output_bit (c3, 1); 
   output_bit (c4, 0); 
   delay_ms (5);
   IF (!input  (r1)) {delay_ms(20); if(!input(r1)) return 12;}//A
   IF (!input  (r2)) {delay_ms(20); if(!input(r2)) return 13;}//B
   IF (!input  (r3)) {delay_ms(20); if(!input(r3)) return 14;}//C
   IF (!input  (r4)) {delay_ms(20); if(!input(r4)) return 15;}//D
   output_bit (c4, 1); 
   return 0xff;
}

//KIEM TRA REALTIME====================
int1 checkRealTime(){
   ds1307_get_time (hrs, min, sec) ;
   IF (sec_temp!=sec){
      sec_temp=sec;
      timerToAlert++;
      timerToExit++;
      timerToResetPass++;
      IF (timerToExit >30){
         output_low (backLight);
         key_value=12;
      }
      return 1;
   }
   return 0;
}

//HOME SCREEN==========================
void homeScreen(byte hrs,min,sec){
   lcd_goto_xy (0, 0); 
   lcd_data ("Clock:  ") ;
   lcd_data (hrs/10+0x30) ;
   lcd_data (hrs%10+0x30) ;
   lcd_data (":");
   lcd_data (min/10+0x30) ;
   lcd_data (min%10+0x30) ;
   lcd_data (":");
   lcd_data (sec/10+0x30) ;
   lcd_data (sec%10+0x30) ;
}
//MENU SCREEN==========================
void menuScreen()
{
   lcd_command (0x01); delay_ms (2) ;
   lcd_GOTO_xy (0, 0); lcd_data ("0.On/Off Alert") ;
   lcd_GOTO_xy (1, 0); lcd_data ("1.Setting") ;
   do{
      key_value=readkeypad ();
      checkRealTime ();
      if(key_value!=0xff){     
         timerToExit=0;
         switch (key_value){
            case 7://bat/tat bao dong
               on_off_pt2272 ();
               key_value=12;
               timerToExit=0;
               break;
            case 2:
               settingScreen (); 
               break;
            default: break;
         }
      }   
   }while(key_value!=12);
}
//SETTING SCREEN=======================
void settingScreen(){
   lcd_command (0x01); delay_ms (2) ;
   lcd_GOTO_xy (0, 0); lcd_data ("0.Time") ;
   lcd_GOTO_xy (1, 0); lcd_data ("1.Password") ;
   timerToExit=0;
   do{ 
      key_value=readkeypad  ()  ;
      checkRealTime ();      
      if(key_value!= 0xff){
         timerToExit=0;
         SWITCH (key_value){
            CASE 7://cai dat gio
               timeScreen ();
               BREAK;
            CASE 2://doi mat khau
               passChange ();
               BREAK;
            DEFAULT: break;
         }
      }
   }while(key_value!=12);
}
//CAI DAT GIO==========================
void timeScreen(){
   lcd_command (0x01); delay_ms (2) ;
   lcd_GOTO_xy (0, 0); lcd_data ("0.System Time") ;
   lcd_GOTO_xy (1, 0); lcd_data ("1.Alarm") ;
   timerToExit=0;
   do{
      key_value=readkeypad  ()  ;
      checkRealTime();
      if(key_value!= 0xff){
         timerToExit = 0;
         SWITCH (key_value){
            CASE 7://cai dat gio he thong
               timerToExit=0;
               sysTime ();
               break;
            CASE 2://cai dat hen gio
               timerToExit=0;
               alarmScreen ();
               break;
            DEFAULT: break;
         }
      }    
   }while(key_value!=12);
}
//TIMER SCREEN=========================
void alarmScreen(){
   lcd_command (0x01); delay_ms (2) ;
   lcd_GOTO_xy (0, 0); lcd_data ("0.On"); //case 7
   lcd_GOTO_xy (1, 0); lcd_data ("1.Off"); //case 2
   do{          
      key_value=readkeypad  ()  ;
      checkRealTime();
      if(key_value!=0xff){
         timerToExit=0;
         SWITCH (key_value){
            CASE 7://bat hen gio
               timerToExit=0;
               alarmSet ();
               break;
            CASE 2://tat hen gio
               lcd_command (0x01); delay_ms (2) ;
               timerToExit=0;
               ena_on_off_AUTO=0;
               key_value=12;//thoat
               break;
            DEFAULT: break;
         }
      }
   }while(key_value!=12);
}

//=====================================

void sysTime(){  
   signed int8 hrst, mint,sect;
   lcd_command (0x0f); delay_us(40); // bat con tro nhap nhay
   lcd_command (0x01); delay_ms (2) ;
   mode_value = 0;
   checkRealTime();
   sect = sec; mint = min; hrst = hrs;
   homescreen (hrst,mint,sect);
   do{         
      blinkMode_sysTime();                      //nhap nhay con tro
      key_value = readkeypad();
      checkRealTime();                          //dem thoi gian thoat
      if(key_value!=0xff){
         timerToExit = 0; 
         switch(key_value){
            case 11:                            //tang gia tri mode_value
               mode_value++;
               if(mode_value==3) mode_value = 0;
               break;
            case 13:                            //tang gia thoi gian
               switch (mode_value){
                  case 0:
                      sect++; if(sect>59) sect=0;
                      homescreen (hrst,mint,sect);
                     break;
                  case 1:
                      mint++; if(mint>59) mint=0;
                      homescreen (hrst,mint,sect);
                     break;
                  case 2:
                      hrst++; if(hrst>23) hrst=0;
                      homescreen (hrst,mint,sect);
                     break;
                  default: break;
               }
               break;
            case 14:                            //giam thoi gian
               switch (mode_value){
                  case 0:
                     sect--; if(sect<0) sect=59;
                     homescreen (hrst,mint,sect);
                     break;
                  case 1:
                     mint--; if(mint<0) mint=59;
                     homescreen (hrst,mint,sect);
                     break;
                  case 2:
                     hrst--; if(hrst<0) hrst=23;
                      homescreen (hrst,mint,sect);
                     break;
                  default: break;
               }
               break;
            case 15:                             //luu thoi gian da thiet lap
               ds1307_set_date_time(0,0,0,0, hrst, mint, sect);               
               key_value = 12;
            default: break;
         }
      }     
   }while(key_value!=12);
   lcd_command(0x0c); delay_us(40);  //tat con tro
}

//=====================================

void on_off_pt2272_auto(){
   IF (ena_on_off_auto){
      hrs_on=read_eeprom  (addr_hrs_on)  ;
      min_on=read_eeprom  (addr_min_on)  ;
      hrs_off=read_eeprom  (addr_hrs_off)  ;
      min_off=read_eeprom  (addr_min_off)  ;
      IF ((hrs_on==hrs)&&(min_on==min) && !flag_on_off_pt2272){
         on_off_pt2272 ();//bat
      }
      IF ((hrs_off==hrs)&&(min_off==min)&& flag_on_off_pt2272){
         on_off_pt2272 ();;//tat
      }
   }
}

//=====================================

void on_off_pt2272(){
   flag_on_off_pt2272=~ flag_on_off_pt2272;
   flag_alert=0;
   IF (flag_on_off_pt2272) output_high (pt2272); //bat module thu RF
   ELSE output_low (pt2272);                     //tat module thu RF
}

//=====================================

void showAlarm(){
   lcd_GOTO_xy (0, 5) ;
   lcd_data (hrs_on/10+0x30);  lcd_data  (hrs_on%10+0x30);  lcd_data  (":") ;
   lcd_data (min_on/10+0x30);  lcd_data  (min_on%10+0x30) ;
   lcd_GOTO_xy (1, 5) ;
   lcd_data (hrs_off/10+0x30);  lcd_data  (hrs_off%10+0x30);  lcd_data  (":") ;
   lcd_data (min_off/10+0x30);  lcd_data  (min_off%10+0x30) ;
   }
   
//=====================================  
   
void alarmSet(){ 
   lcd_command (0x0f); delay_us(40) ; //bat con tro nhap nhay
   lcd_command (0x01); delay_ms (2) ;
   lcd_GOTO_xy (0, 0); lcd_data ("On: ") ;
   lcd_GOTO_xy (1, 0); lcd_data ("Off: ") ;
   mode_value = 0;
   //khoi tao thoi gian bat/tat
   IF (read_eeprom (addr_flag_alarm)!=1){
      write_eeprom (addr_hrs_on, 0) ;
      write_eeprom (addr_min_on, 0) ;
      write_eeprom (addr_hrs_off, 0) ;
      write_eeprom (addr_min_off, 0) ;
   }
   //doc du lieu tu eeprom
   hrs_on=read_eeprom  (addr_hrs_on)  ;
   min_on=read_eeprom  (addr_min_on)  ;
   hrs_off=read_eeprom  (addr_hrs_off)  ;
   min_off=read_eeprom  (addr_min_off)  ;
   
   showAlarm();
   //thay doi thoi gian bat/tat  
   do{    
      blinkMode_alarm();
      key_value=readkeypad  ()  ;
      checkRealTime();
      if(key_value!=0xff){
         timerToExit=0;
         switch(key_value){
            case 11: 
               mode_value++;
               if (mode_value==4) mode_value=0;
               break;
            case 13:
               switch (mode_value){
                  case 0:
                     min_on++; IF (min_on>59) min_on=0; 
                     showAlarm();
                     BREAK;
                  CASE 1:
                     hrs_on++; IF (hrs_on>23) hrs_on=0; 
                     showAlarm();
                     BREAK;
                  CASE 2:
                     min_off++; IF (min_off>59) min_off=0; 
                     showAlarm();
                     BREAK;
                  CASE 3:
                     hrs_off++; IF (hrs_off>23) hrs_off=0; 
                     showAlarm();
                     BREAK;
                  DEFAULT: break;
               }
               break;
            case 14:
               switch (mode_value){
                  CASE 0:
                     min_on--; IF (min_on<0) min_on=59; 
                     showAlarm();
                     BREAK;
                  CASE 1:
                     hrs_on--; IF (hrs_on<0) hrs_on=23; 
                     showAlarm();
                     BREAK;
                  CASE 2:
                     min_off--; IF (min_off<0) min_off=59; 
                     showAlarm();
                     BREAK;
                  CASE 3:
                     hrs_off--; IF (hrs_off<0) hrs_off=23; 
                     showAlarm();
                     BREAK;
                  DEFAULT: break;
               } 
               break;
            case 15: //gan du lieu cho eeprom
               write_eeprom (addr_flag_alarm, 1) ;
               write_eeprom (addr_hrs_on, hrs_on) ;
               write_eeprom (addr_min_on, min_on) ;
               write_eeprom (addr_hrs_off, hrs_off) ;
               write_eeprom (addr_min_off, min_off) ;
               ena_on_off_AUTO=1;
               key_value=12;           
               break;   
            default: break;
         }
      }
   }while(key_value!=12);
   lcd_command(0x0c); delay_us(40); //tat con tro
}
//=====================================

void blinkMode_sysTime(){
   switch(mode_value){
      case 0: lcd_goto_xy(0,15); break;
      case 1: lcd_goto_xy(0,12); break;
      case 2: lcd_goto_xy(0,9); break;      
      default: break;
   }
}

//=====================================

void blinkMode_alarm(){
   switch(mode_value){
      case 0: lcd_goto_xy(0,9); break;
      case 1: lcd_goto_xy(0,6); break;
      case 2: lcd_goto_xy(1,9); break;      
      case 3: lcd_goto_xy(1,6); break; 
      default: break;
   }
}

//=====================================

void passInit(){
   IF (read_eeprom (addr_flag_pass) != flag_pass)
      FOR (i=0; i<4; i++)
         write_eeprom (addr_first_pass+i, '0');
}

//=====================================

void getPass(){
   for (INT i=0; i<4; i++)
      pass[i]=read_eeprom  (addr_first_pass+i)  ;
}

//=====================================

void passChange(){
   int1 count = 0;
   if(isPassOk()){ 
      lcd_command (0x01); delay_ms (2) ;
      lcd_GOTO_xy (0, 0); lcd_data (">New password") ;
      i=0;
      lcd_GOTO_xy (1, i);
      do{       
         key_value=readkeypad  ()  ;
         if(key_value != 0xff)
         {
            timerToExit=0;
            if (key_value<12&&i<4){
               passTemp[i]=keyCode[key_value];
               lcd_GOTO_xy (1, i); lcd_data ('*');
               i++;
            }
            else if (key_value==13&&i>=0){
               lcd_GOTO_xy (1, i); lcd_data (' ') ;
               i--;
            }
            else if (key_value==15)
            {  /* @count = 0: thuc hien nhap password lan 2 de xac nhan
                  @count = 1: xac nhan password
               */
               if(count == 1)
               {
                  if(comfirmPassword() == 1)
                  {
                     for (i=0; i<4; i++)
                        write_eeprom (addr_first_pass+i, passTemp[i]);
                     write_eeprom(addr_flag_pass, flag_pass);
                     key_value=12;
                  }
                  else 
                  {  
                     lcd_command (0x01); delay_ms (2) ;
                     lcd_GOTO_xy (0, 0); lcd_data (">Wrong! Again") ;
                     delay_ms(500);
                     lcd_GOTO_xy (0, 0); lcd_data (">New Password") ;
                     i=0;
                     lcd_GOTO_xy (1, i);
                     count = 0;
                  }
               }
               else // count = 0
               {
                  for (i=0; i<4; i++)
                     passConfirm[i] = passTemp[i];
                  count = 1;
                  lcd_command (0x01); delay_ms (2) ;
                  lcd_GOTO_xy (0, 0); lcd_data ("Confirm password") ;
                  i=0;
                  lcd_GOTO_xy (1, i);
               }
            }
         }
         checkRealTime();
      }while (key_value!=12) ;
   } 
   key_value = 12;
}

//================================

int1 isPassOk(){
   i=0;
   int j=0;
   lcd_command (0x01); delay_ms (2) ;
   lcd_goto_xy(0,0); lcd_data (">Enter Password") ;
   do{        
      key_value=readkeypad ();
      checkRealTime();
      if(key_value!= 0xff){
         timerToExit=0;
         IF (key_value<12 && i<4){
            passTemp[i]=keyCode[key_value];
            lcd_GOTO_xy (1, i); lcd_data ('*');
            i++;
         }
         else IF (key_value==13 && i>0){
            i--;
            passTemp[i]=' ';
            lcd_GOTO_xy (1, i); lcd_data (' ') ;
         }
         else if (key_value==15){
            getPass ();
            for (i=0; i<4; i++){
               if (pass[i]!=passTemp[i]){
                  j++;
                  if(j>2) {
                     lcd_command(0x01); delay_ms(2);
                     lcd_goto_xy(0,0); lcd_data("*Over 3 Time*"); 
                     delay_ms(1000);
                     return 0;
                  }
                  lcd_command (0x01); delay_ms (2) ;
                  lcd_goto_xy(0,0); lcd_data (">Wrong! Again") ;
                  break;
               }               
            }  
            if(i==4) return 1;
         }
      }      
   }while(key_value != 12);
   return 0;
}
//------------------------------------------------
int1 comfirmPassword()
{
   for (i=0; i<4; i++)
       if (passTemp[i]!=passConfirm[i]) return 0;
   return 1;
}
