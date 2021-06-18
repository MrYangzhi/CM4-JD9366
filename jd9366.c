
#include "JD9366_BOE7P.h"
 
#define panel_reset(val) sunxi_lcd_gpio_set_value(sel, 0, val)
#define panel_power_en(val) sunxi_lcd_gpio_set_value(sel, 1, val)
#define Delayms(x) sunxi_lcd_delay_ms(x);
static void LCD_power_on(u32 sel);
static void LCD_power_off(u32 sel);
static void LCD_bl_open(u32 sel);
static void LCD_bl_close(u32 sel);
 
static void LCD_panel_init(u32 sel);
static void LCD_panel_exit(u32 sel);
 
 
static void LCD_cfg_panel_info(panel_extend_para *info)
{
    u32 i = 0, j = 0;
    u32 items;
    u8 lcd_gamma_tbl[][2] = {
        /* {input value, corrected value} */
        {0, 0},
        {15, 15},
        {30, 30},
        {45, 45},
        {60, 60},
        {75, 75},
        {90, 90},
        {105, 105},
        {120, 120},
        {135, 135},
        {150, 150},
        {165, 165},
        {180, 180},
        {195, 195},
        {210, 210},
        {225, 225},
        {240, 240},
        {255, 255},
    };
 
 
    u32 lcd_cmap_tbl[2][3][4] = {
        {
            {LCD_CMAP_G0, LCD_CMAP_B1, LCD_CMAP_G2, LCD_CMAP_B3},
            {LCD_CMAP_B0, LCD_CMAP_R1, LCD_CMAP_B2, LCD_CMAP_R3},
            {LCD_CMAP_R0, LCD_CMAP_G1, LCD_CMAP_R2, LCD_CMAP_G3},
        },
        {
            {LCD_CMAP_B3, LCD_CMAP_G2, LCD_CMAP_B1, LCD_CMAP_G0},
            {LCD_CMAP_R3, LCD_CMAP_B2, LCD_CMAP_R1, LCD_CMAP_B0},
            {LCD_CMAP_G3, LCD_CMAP_R2, LCD_CMAP_G1, LCD_CMAP_R0},
        },
    };
 
 
    items = sizeof(lcd_gamma_tbl)/2;
    for (i = 0; i < items - 1; i++) {
        u32 num = lcd_gamma_tbl[i+1][0] - lcd_gamma_tbl[i][0];
 
        for (j = 0; j < num; j++) {
            u32 value = 0;
 
            value = lcd_gamma_tbl[i][1] + ((lcd_gamma_tbl[i+1][1] - lcd_gamma_tbl[i][1]) * j)/num;
            info->lcd_gamma_tbl[lcd_gamma_tbl[i][0] + j] = (value<<16) + (value<<8) + value;
        }
    }
    info->lcd_gamma_tbl[255] = (lcd_gamma_tbl[items-1][1]<<16)
        + (lcd_gamma_tbl[items-1][1]<<8) + lcd_gamma_tbl[items-1][1];
 
 
    memcpy(info->lcd_cmap_tbl, lcd_cmap_tbl, sizeof(lcd_cmap_tbl));
 
}
 
static __s32 LCD_open_flow(u32 sel)
{
    /* open lcd power, and delay 50ms */
    LCD_OPEN_FUNC(sel, LCD_power_on, 5);
    /* open lcd power, than delay 200ms */
    LCD_OPEN_FUNC(sel, LCD_panel_init, 40);
    /* open lcd controller, and delay 100ms */
    LCD_OPEN_FUNC(sel, sunxi_lcd_tcon_enable, 40);
    /* open lcd backlight, and delay 0ms */
    LCD_OPEN_FUNC(sel, LCD_bl_open, 0);
 
    return 0;
}
 
static __s32 LCD_close_flow(u32 sel)
{
    /* close lcd backlight, and delay 0ms */
    LCD_CLOSE_FUNC(sel, LCD_bl_close, 0);
    /* close lcd controller, and delay 0ms */
    LCD_CLOSE_FUNC(sel, sunxi_lcd_tcon_disable, 0);
    /* open lcd power, than delay 200ms */
    LCD_CLOSE_FUNC(sel, LCD_panel_exit,	20);
    /* cose lcd power, and delay 500ms */
    LCD_CLOSE_FUNC(sel, LCD_power_off, 50);
 
    return 0;
}
 
static void LCD_power_on(u32 sel)
{
    sunxi_lcd_pin_cfg(sel, 1);
    panel_power_en(0);
    /* config lcd_power pin to open lcd power0 */
    sunxi_lcd_power_enable(sel, 1);
    sunxi_lcd_delay_ms(5);
    /* config lcd_power pin to open lcd power1 */
    sunxi_lcd_power_enable(sel, 0);
    sunxi_lcd_delay_ms(8);
    panel_power_en(1);
    sunxi_lcd_delay_ms(10);
    panel_reset(1);
    sunxi_lcd_delay_ms(10);
    panel_reset(0);
    sunxi_lcd_delay_ms(20);
    panel_reset(1);
}
 
static void LCD_power_off(u32 sel)
{
    sunxi_lcd_pin_cfg(sel, 0);
    /* config lcd_power pin to close lcd power1 */
    sunxi_lcd_power_disable(sel, 0);
    /* config lcd_power pin to close lcd power0 */
    sunxi_lcd_power_disable(sel, 1);
    /* sunxi_lcd_pwm_disable(sel);//close pwm module */
}
 
static void LCD_bl_open(u32 sel)
{
    /* open pwm module */
    sunxi_lcd_pwm_enable(sel);
    /* config lcd_bl_en pin to open lcd backlight */
    sunxi_lcd_backlight_enable(sel);
}
 
static void LCD_bl_close(u32 sel)
{
    /* config lcd_bl_en pin to close lcd backlight */
    sunxi_lcd_backlight_disable(sel);
    /* close pwm module */
    sunxi_lcd_pwm_disable(sel);
}
#if 0
 
#define REGFLAG_DELAY         0XFF
#define REGFLAG_END_OF_TABLE  0xFE   /* END OF REGISTERS MARKER */
 
struct LCM_setting_table {
    u8 cmd;
    u32 count;
    u8 para_list[64];
};
 
 
static struct LCM_setting_table lcm_initialization_setting[] = {
    {0xE0, 1, {0x00} },
    {0xE1, 1, {0x93} },
    {0xE2, 1, {0x65} },
    {0xE3, 1, {0xF8} },
    {0x80, 1, {0x03} },	/* 4Lanes */
    {0xE0, 1, {0x04} },
    {0x2D, 1, {0x03} }, /* 4Lanes */
    {0xE0, 1, {0x01} },
    {0x00, 1, {0x00} },
    {0x01, 1, {0x6F} },
    {0x03, 1, {0x00} },
    {0x04, 1, {0x6F} },
    {0x17, 1, {0x00} },
    {0x18, 1, {0xD7} }, /* VGMP=4.8V */
    {0x19, 1, {0x05} },
    {0x1A, 1, {0x00} },
    {0x1B, 1, {0xD7} }, /* VGMN=-4.8V */
    {0x1C, 1, {0x05} },
    {0x1F, 1, {0x79} }, /* VGH_REG=18V */
    {0x20, 1, {0x2D} }, /* VGL_REG=-12V */
    {0x21, 1, {0x2D} }, /* VGL_REG2=-12V */
    {0x22, 1, {0x4F} },
    {0x26, 1, {0xF1} }, /* VDDD from IOVCC */
    {0x37, 1, {0x09} }, /* SS=1, 1, {BGR=1 */
    {0x38, 1, {0x04} }, /* JDT=100 column inversion */
    {0x39, 1, {0x08} }, /* RGB_N_EQ1, 1, { modify 20140806 */
    {0x3A, 1, {0x12} }, /* RGB_N_EQ2, 1, { modify 20140806 */
    {0x3C, 1, {0x78} }, /* SET EQ3 for TE_H */
    {0x3E, 1, {0x80} }, /* SET CHGEN_OFF, 1, { modify 20140806 */
    {0x3F, 1, {0x80} }, /* SET CHGEN_OFF2, 1, { modify 20140806 */
    {0x40, 1, {0x06} }, /* RSO=800 RGB */
    {0x41, 1, {0xA0} }, /* LN=640->1280 line */
    {0x55, 1, {0x0F} }, /* DCDCM=1111, 1, { no output */
    {0x56, 1, {0x01} },
    {0x57, 1, {0xA8} }, /*[7:5]VGH_RT, 1, {[4:2]=VGL_RT, 1, {[1:0]=VCL_RT*/
    {0x58, 1, {0x0A} }, /* AVDD_S */
    {0x59, 1, {0x2A} }, /* VCL = -2.7V */
    {0x5A, 1, {0x37} }, /* VGH = 19V */
    {0x5B, 1, {0x19} },/* VGL = -12V */
    {0x5D, 1, {0x70} },
    {0x5E, 1, {0x50} },
    {0x5F, 1, {0x3F} },
    {0x60, 1, {0x31} },
    {0x61, 1, {0x2D} },
    {0x62, 1, {0x1D} },
    {0x63, 1, {0x22} },
    {0x64, 1, {0x0C} },
    {0x65, 1, {0x25} },
    {0x66, 1, {0x24} },
    {0x67, 1, {0x24} },
    {0x68, 1, {0x41} },
    {0x69, 1, {0x2F} },
    {0x6A, 1, {0x36} },
    {0x6B, 1, {0x28} },
    {0x6C, 1, {0x26} },
    {0x6D, 1, {0x1C} },
    {0x6E, 1, {0x08} },
    {0x6F, 1, {0x02} },
    {0x70, 1, {0x70} },
    {0x71, 1, {0x50} },
    {0x72, 1, {0x3F} },
    {0x73, 1, {0x31} },
    {0x74, 1, {0x2D} },
    {0x75, 1, {0x1D} },
    {0x76, 1, {0x22} },
    {0x77, 1, {0x0C} },
    {0x78, 1, {0x25} },
    {0x79, 1, {0x24} },
    {0x7A, 1, {0x24} },
    {0x7B, 1, {0x41} },
    {0x7C, 1, {0x2F} },
    {0x7D, 1, {0x36} },
    {0x7E, 1, {0x28} },
    {0x7F, 1, {0x26} },
    {0x80, 1, {0x1C} },
    {0x81, 1, {0x08} },
    {0x82, 1, {0x02} },
    {0xE0, 1, {0x02} },
    {0x00, 1, {0x00} },
    {0x01, 1, {0x04} },
    {0x02, 1, {0x06} },
    {0x03, 1, {0x08} },
    {0x04, 1, {0x0A} },
    {0x05, 1, {0x0C} },
    {0x06, 1, {0x0E} },
    {0x07, 1, {0x17} },
    {0x08, 1, {0x37} },
    {0x09, 1, {0x1F} },
    {0x0A, 1, {0x10} },
    {0x0B, 1, {0x1F} },
    {0x0C, 1, {0x1F} },
    {0x0D, 1, {0x1F} },
    {0x0E, 1, {0x1F} },
    {0x0F, 1, {0x1F} },
    {0x10, 1, {0x1F} },
    {0x11, 1, {0x1F} },
    {0x12, 1, {0x1F} },
    {0x13, 1, {0x12} },
    {0x14, 1, {0x1F} },
    {0x15, 1, {0x1F} },
    {0x16, 1, {0x01} },
    {0x17, 1, {0x05} },
    {0x18, 1, {0x07} },
    {0x19, 1, {0x09} },
    {0x1A, 1, {0x0B} },
    {0x1B, 1, {0x0D} },
    {0x1C, 1, {0x0F} },
    {0x1D, 1, {0x17} },
    {0x1E, 1, {0x37} },
    {0x1F, 1, {0x1F} },
    {0x20, 1, {0x11} },
    {0x21, 1, {0x1F} },
    {0x22, 1, {0x1F} },
    {0x23, 1, {0x1F} },
    {0x24, 1, {0x1F} },
    {0x25, 1, {0x1F} },
    {0x26, 1, {0x1F} },
    {0x27, 1, {0x1F} },
    {0x28, 1, {0x1F} },
    {0x29, 1, {0x13} },
    {0x2A, 1, {0x1F} },
    {0x2B, 1, {0x1F} },
    {0x2C, 1, {0x11} },
    {0x2D, 1, {0x0F} },
    {0x2E, 1, {0x0D} },
    {0x2F, 1, {0x0B} },
    {0x30, 1, {0x09} },
    {0x31, 1, {0x07} },
    {0x32, 1, {0x05} },
    {0x33, 1, {0x37} },
    {0x34, 1, {0x17} },
    {0x35, 1, {0x1F} },
    {0x36, 1, {0x01} },
    {0x37, 1, {0x1F} },
    {0x38, 1, {0x1F} },
    {0x39, 1, {0x1F} },
    {0x3A, 1, {0x1F} },
    {0x3B, 1, {0x1F} },
    {0x3C, 1, {0x1F} },
    {0x3D, 1, {0x1F} },
    {0x3E, 1, {0x1F} },
    {0x3F, 1, {0x13} },
    {0x40, 1, {0x1F} },
    {0x41, 1, {0x1F} },
    {0x42, 1, {0x10} },
    {0x43, 1, {0x0E} },
    {0x44, 1, {0x0C} },
    {0x45, 1, {0x0A} },
    {0x46, 1, {0x08} },
    {0x47, 1, {0x06} },
    {0x48, 1, {0x04} },
    {0x49, 1, {0x37} },
    {0x4A, 1, {0x17} },
    {0x4B, 1, {0x1F} },
    {0x4C, 1, {0x00} },
    {0x4D, 1, {0x1F} },
    {0x4E, 1, {0x1F} },
    {0x4F, 1, {0x1F} },
    {0x50, 1, {0x1F} },
    {0x51, 1, {0x1F} },
    {0x52, 1, {0x1F} },
    {0x53, 1, {0x1F} },
    {0x54, 1, {0x1F} },
    {0x55, 1, {0x12} },
    {0x56, 1, {0x1F} },
    {0x57, 1, {0x1F} },
    {0x58, 1, {0x10} },
    {0x59, 1, {0x00} },
    {0x5A, 1, {0x00} },
    {0x5B, 1, {0x10} },
    {0x5C, 1, {0x07} },
    {0x5D, 1, {0x30} },
    {0x5E, 1, {0x00} },
    {0x5F, 1, {0x00} },
    {0x60, 1, {0x30} },
    {0x61, 1, {0x03} },
    {0x62, 1, {0x04} },
    {0x63, 1, {0x03} },
    {0x64, 1, {0x6A} }, /* SETV_OFF, 1, { modify 20140806 */
    {0x65, 1, {0x75} },
    {0x66, 1, {0x0D} },
    {0x67, 1, {0xB3} },
    {0x68, 1, {0x09} },
    {0x69, 1, {0x06} },
    {0x6A, 1, {0x6A} }, /* CKV_OFF, 1, { modify 20140806 */
    {0x6B, 1, {0x04} },
    {0x6C, 1, {0x00} },
    {0x6D, 1, {0x04} },
    {0x6E, 1, {0x04} },
    {0x6F, 1, {0x88} },
    {0x70, 1, {0x00} },
    {0x71, 1, {0x00} },
    {0x72, 1, {0x06} },
    {0x73, 1, {0x7B} },
    {0x74, 1, {0x00} },
    {0x75, 1, {0xBC} },
    {0x76, 1, {0x00} },
    {0x77, 1, {0x0D} },
    {0x78, 1, {0x2C} },
    {0x79, 1, {0x00} },
    {0x7A, 1, {0x00} },
    {0x7B, 1, {0x00} },
    {0x7C, 1, {0x00} },
    {0x7D, 1, {0x03} },
    {0x7E, 1, {0x7B} },
    {0xE0, 1, {0x04} },
    {0x2B, 1, {0x2B} },
    {0x2E, 1, {0x44} },
    {0xE0, 1, {0x00} },
    {0xE6, 1, {0x02} },	/* Watch dog */
    {0xE7, 1, {0x02} },	/* Watch dog */
    {0x11, 1, {0x00} }, /* SLPOUT */
    {REGFLAG_DELAY, 120, {} },
    {0x29, 1, {0x00} }, /* DSPON */
    {REGFLAG_DELAY, 5, {} },
    {REGFLAG_END_OF_TABLE, 0x00, {} }
};
#endif
 
#define SSD_Single(cmd, val) dsi_dcs_wr_1para(0, cmd, val)
#define SSD_CMD(cmd) dsi_dcs_wr_0para(0, cmd)
 
static void jd9366_init(void) {
 
    //JD9365 initial code
 
    printf("%s\n", __func__);
    //====== JD9366+ BP070WX3-300 =========//
 
 
 
 
    //Page0
    SSD_Single(0xE0, 0x00);
 
    //--- PASSWORD  ----//
    SSD_Single(0xE1, 0x93);
    SSD_Single(0xE2, 0x65);
    SSD_Single(0xE3, 0xF8);
    SSD_Single(0x80, 0x03);
 
    //--- Page4  ----//
    SSD_Single(0xE0, 0x04);
    SSD_Single(0x2D, 0x03);
 
    //--- Page1  ----//
    SSD_Single(0xE0, 0x01);
 
    //Set VCOM
    SSD_Single(0x00, 0x00);
    SSD_Single(0x01, 0xA9);	//VCOM A8 AE AD
    //Set VCOM_Reverse
    SSD_Single(0x03, 0x00);
    SSD_Single(0x04, 0xA8);	
 
    //Set Gamma Power,  VGMP, VGMN, VGSP, VGSN
    SSD_Single(0x17, 0x00);
    SSD_Single(0x18, 0xB1);
    SSD_Single(0x19, 0x01);
    SSD_Single(0x1A, 0x00);
    SSD_Single(0x1B, 0xB1);  //VGMN=0
    SSD_Single(0x1C, 0x01);
 
    //Set Gate Power
    SSD_Single(0x1F, 0x3E);     //VGH_R  = 15V                       
    SSD_Single(0x20, 0x2D);     //VGL_R  = -12V                      
    SSD_Single(0x21, 0x2D);     //VGL_R2 = -12V                      
    SSD_Single(0x22, 0x7E);     //PA[6]=0,  PA[5]=0,  PA[4]=0,  PA[0]=0 
 
    //SETPANEL
    SSD_Single(0x37, 0x19);	//SS=1, BGR=1
 
    //SET RGBCYC
    SSD_Single(0x38, 0x05);	//JDT=101 zigzag inversion
    SSD_Single(0x39, 0x00);	//RGB_N_EQ1,  modify 20140806
    SSD_Single(0x3A, 0x01);	//RGB_N_EQ2,  modify 20140806
    SSD_Single(0x3C, 0x78);	//SET EQ3 for TE_H
    SSD_Single(0x3D, 0xFF);	//SET CHGEN_OFF,  modify 20140827 
    SSD_Single(0x3E, 0xFF);	//SET CHGEN_OFF,  modify 20140827 
    SSD_Single(0x3F, 0xFF);	//SET CHGEN_OFF2,  modify 20140827
    //SSD_Single(0x3E, 0x80);	//SET CHGEN_OFF,  modify 20140806 
    //SSD_Single(0x3F, 0x80);	//SET CHGEN_OFF2,  modify 20140806
 
 
    //Set TCON
    SSD_Single(0x40, 0x06);	//RSO=800 RGB
    SSD_Single(0x41, 0xA0);	//LN=640->1280 line
 
    SSD_Single(0x43, 0x08);
    SSD_Single(0x44, 0x09);
    SSD_Single(0x45, 0x28);
 
    //--- power voltage  ----//
    SSD_Single(0x55, 0x0F);	//DCDCM=1111
    //SSD_Single(0x56, 0x01);
    SSD_Single(0x57, 0x89);	//VGH_RT=100,  Charge pump Ratio:3*AVDD-VCL
    SSD_Single(0x58, 0x0A);
    SSD_Single(0x59, 0x0A);	//VCL = -2.5V
    SSD_Single(0x5A, 0x28);	//VGH = 15V
    SSD_Single(0x5B, 0x19);	//VGL = -12V
 
 
 
    //--- Gamma  ----//
    SSD_Single(0x5D, 0x7C); // 
    SSD_Single(0x5E, 0x66); //
    SSD_Single(0x5F, 0x56); //
    SSD_Single(0x60, 0x49); //
    SSD_Single(0x61, 0x44); //
    SSD_Single(0x62, 0x34); //
    SSD_Single(0x63, 0x37); //
    SSD_Single(0x64, 0x1F); //
    SSD_Single(0x65, 0x36); //
    SSD_Single(0x66, 0x33); //
    SSD_Single(0x67, 0x32); //
    SSD_Single(0x68, 0x4F); //
    SSD_Single(0x69, 0x3D); //
    SSD_Single(0x6A, 0x43); //
    SSD_Single(0x6B, 0x35); //
    SSD_Single(0x6C, 0x30); //
    SSD_Single(0x6D, 0x24); //
    SSD_Single(0x6E, 0x13); //
    SSD_Single(0x6F, 0x00); //
    SSD_Single(0x70, 0x7C); //
    SSD_Single(0x71, 0x66); //
    SSD_Single(0x72, 0x56); //
    SSD_Single(0x73, 0x49); //
    SSD_Single(0x74, 0x44); //
    SSD_Single(0x75, 0x34); //
    SSD_Single(0x76, 0x37); //
    SSD_Single(0x77, 0x1F); //
    SSD_Single(0x78, 0x36); //
    SSD_Single(0x79, 0x33); //
    SSD_Single(0x7A, 0x32); //
    SSD_Single(0x7B, 0x4F); //
    SSD_Single(0x7C, 0x3D); //
    SSD_Single(0x7D, 0x43); //
    SSD_Single(0x7E, 0x35); //
    SSD_Single(0x7F, 0x30); //
    SSD_Single(0x80, 0x24); //
    SSD_Single(0x81, 0x13); //
    SSD_Single(0x82, 0x00); //
 
 
    //Page2,  for GIP
    SSD_Single(0xE0, 0x02);
 
    //GIP_L Pin mapping
    SSD_Single(0x00, 0x45);//0x05
    SSD_Single(0x01, 0x45);//0x05  
    SSD_Single(0x02, 0x44);//0x04
    SSD_Single(0x03, 0x44);//0x04
    SSD_Single(0x04, 0x47);//0x07
    SSD_Single(0x05, 0x47);//0x07
    SSD_Single(0x06, 0x46);//0x06
    SSD_Single(0x07, 0x46);//0x06
    SSD_Single(0x08, 0x40);//0x00
    SSD_Single(0x09, 0x1F);//0x1F
    SSD_Single(0x0A, 0x1F);//0x1F
    SSD_Single(0x0B, 0x1F);//0x1F
    SSD_Single(0x0C, 0x1F);//0x1F
    SSD_Single(0x0D, 0x1F);//0x1F
    SSD_Single(0x0E, 0x1F);//0x1F
    SSD_Single(0x0F, 0x41);//0x01
    SSD_Single(0x10, 0x1F);//0x1F
    SSD_Single(0x11, 0x1F);//0x1F
    SSD_Single(0x12, 0x1F);//0x1F
    SSD_Single(0x13, 0x1F);//0x1F
    SSD_Single(0x14, 0x1F);//0x1F
    SSD_Single(0x15, 0x1F);//0x1F
 
    //GIP_R Pin mapping
    SSD_Single(0x16, 0x45);
    SSD_Single(0x17, 0x45);
    SSD_Single(0x18, 0x44);
    SSD_Single(0x19, 0x44);
    SSD_Single(0x1A, 0x47);
    SSD_Single(0x1B, 0x47);
    SSD_Single(0x1C, 0x46);
    SSD_Single(0x1D, 0x46);
    SSD_Single(0x1E, 0x40);
    SSD_Single(0x1F, 0x1F);
    SSD_Single(0x20, 0x1F);
    SSD_Single(0x21, 0x1F);
    SSD_Single(0x22, 0x1F);
    SSD_Single(0x23, 0x1F);
    SSD_Single(0x24, 0x1F);
    SSD_Single(0x25, 0x41);
    SSD_Single(0x26, 0x1F);
    SSD_Single(0x27, 0x1F);
    SSD_Single(0x28, 0x1F);
    SSD_Single(0x29, 0x1F);
    SSD_Single(0x2A, 0x1F);
    SSD_Single(0x2B, 0x1F);
 
 
 
    //GIP Timing  
    SSD_Single(0x58, 0x40);        // TGEQL,  Initial porch
    //SSD_Single(0x59, 0x00);
    //SSD_Single(0x5A, 0x00);
    SSD_Single(0x5B, 0x10);       //STV number =4,  STV_S0[10:8]
    SSD_Single(0x5C, 0x06);       //STV_S0[7:0] =
    SSD_Single(0x5D, 0x20);//60   //STV_W=4 ,  STV_S1=0, 1h timing
    SSD_Single(0x5E, 0x01);       //STV_S2 =1, 1h timing
    SSD_Single(0x5F, 0x02);       //STV_S3 =2, 1h timing
    //SSD_Single(0x60, 0x00);     //ETV default
    //SSD_Single(0x61, 0x01);     //ETV default
    //SSD_Single(0x62, 0x02);     //ETV default
    SSD_Single(0x63, 0x62);       //SETV_ON
    SSD_Single(0x64, 0x62);       //SETV_OFF
    //SSD_Single(0x65, 0x00);    //ETV
    //SSD_Single(0x66, 0x00);    //ETV
    SSD_Single(0x67, 0x32);          //CKV0_NUM[3:0]=8,  CKV0_W[3:0]=4
    SSD_Single(0x68, 0x08);   //0x06       
    SSD_Single(0x69, 0x62); 
    SSD_Single(0x6A, 0x66); //66 CLK Falling to Source Falling time 3us
    SSD_Single(0x6B, 0x08);  
    SSD_Single(0x6C, 0x00);
    SSD_Single(0x6D, 0x04); //GEQ_GGND1
    SSD_Single(0x6E, 0x04); //GEQ_GGND2
    SSD_Single(0x6F, 0x88);
 
    //SSD_Single(0x70, 0x00);
    //SSD_Single(0x71, 0x00);
    //SSD_Single(0x72, 0x06);
    //SSD_Single(0x73, 0x7B);
    //SSD_Single(0x74, 0x00);
    //SSD_Single(0x75, 0x07);
    //SSD_Single(0x76, 0x00);
    //SSD_Single(0x77, 0x5D);
    //SSD_Single(0x78, 0x17);
    //SSD_Single(0x79, 0x1F);
    //SSD_Single(0x7A, 0x00);
    //SSD_Single(0x7B, 0x00);
    //SSD_Single(0x7C, 0x00);
    //SSD_Single(0x7D, 0x03);
    //SSD_Single(0x7E, 0x7B);
 
 
 
 
    //Page3
    SSD_Single(0xE0, 0x03);
    SSD_Single(0x98, 0x3E);	//From 2E to 3E,  LED_ON
 
 
    //Page4
    SSD_Single(0xE0, 0x04);  //ESD protect
    SSD_Single(0x09, 0x10);
    SSD_Single(0x2B, 0x2B);	
    SSD_Single(0x2D, 0x03);	
    SSD_Single(0x2E, 0x44);	
 
 
    //Page0
    SSD_Single(0xE0, 0x00); //Watch dog
    SSD_Single(0xE6, 0x02);
    SSD_Single(0xE7, 0x06);
 
    //SLP OUT
 
    SSD_CMD(0x11);  	// SLPOUT
    sunxi_lcd_delay_ms(120);
 
 
    //DISP ON
 
    SSD_CMD(0x29);  	// DSPON
    sunxi_lcd_delay_ms(5);
}
 
static void LCD_panel_init(u32 sel)
{
#if 0
    u32 i;
 
    for (i = 0; ; i++) {
        if (lcm_initialization_setting[i].cmd == REGFLAG_END_OF_TABLE)
            break;
        else if (lcm_initialization_setting[i].cmd == REGFLAG_DELAY)
            sunxi_lcd_delay_ms(lcm_initialization_setting[i].count);
        else {
#ifdef SUPPORT_DSI
            dsi_dcs_wr(0, lcm_initialization_setting[i].cmd,
                    lcm_initialization_setting[i].para_list,
                    lcm_initialization_setting[i].count);
#endif
        }
    }
#endif
    jd9366_init();
    /* Begin to translate data */
    sunxi_lcd_dsi_clk_enable(sel);
 
    return;
}
 
static void LCD_panel_exit(u32 sel)
{
    sunxi_lcd_dsi_clk_disable(sel);
    /* panel_reset(0); */
    return;
}
 
/* sel: 0:lcd0; 1:lcd1 */
static __s32 LCD_user_defined_func(u32 sel, u32 para1, u32 para2, u32 para3)
{
    return 0;
}
 
__lcd_panel_t JD9366_BOE7P_panel = {
    /* panel driver name, must mach the name of lcd_drv_name in sys_config.fex */
    .name = "JD9366_BOE7P",
    .func = {
        .cfg_panel_info = LCD_cfg_panel_info,
        .cfg_open_flow = LCD_open_flow,
        .cfg_close_flow = LCD_close_flow,
        .lcd_user_defined_func = LCD_user_defined_func,
    },
};
