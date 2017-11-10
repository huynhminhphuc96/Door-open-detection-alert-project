#define lcd_function_set       0x38
#define lcd_display_control    0x0c
#define lcd_clear_display      0x01
#define lcd_entry_mode         0x06

#define output_lcd   output_d
#define lcd_rs       pin_e0
#define lcd_e        pin_e1

void lcd_command(unsigned char mdk)
{
   output_low(lcd_rs);
   output_lcd(mdk);
   output_high(lcd_e);
   delay_us(20);
   output_low(lcd_e);
   delay_us(20);  
}

void lcd_data(unsigned char mht)
{
   output_high(lcd_rs);
   output_lcd(mht);
   output_high(lcd_e);
   delay_us(20);
   output_low(lcd_e);
   delay_us(20);
}

void lcd_setup()
{
   output_low(lcd_e);
   output_low(lcd_rs);
   lcd_command(lcd_function_set);         
   delay_us(40);
   lcd_command(lcd_display_control);   
   delay_us(40);
   lcd_command(lcd_clear_display);      
   delay_ms(2);
   lcd_command(lcd_entry_mode);       
   delay_us(40); 
}

void lcd_goto_xy(signed int8 x, signed int8 y)
{ 
   const unsigned int8 lcd_vitri[]={0x80,0xc0,0x94,0xd4};
   lcd_command(lcd_vitri[x]+y);
}

