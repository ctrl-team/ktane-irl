#define GC9A01_DRIVER // Round TFT SPI Display 

#define TFT_WIDTH  240 // ST7789 240 x 240 and 240 x 320
#define TFT_HEIGHT 240 // GC9A01 240 x 240

#define TFT_MISO 0 // RX PIN - Not Connected in display
#define TFT_SCLK 2 // In display, connect to SCL
#define TFT_MOSI 3 // In display, connect to SDA
#define TFT_CS   20  // In display, connect to CS
#define TFT_DC   18  // In display, connect to DC
#define TFT_RST  19  // In display, connect to RES

#define TFT_BL   17            // In display, connect to BLK
#define TFT_BACKLIGHT_ON HIGH  // Level to turn ON back-light (HIGH or LOW)

#define LOAD_GLCD   
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6 
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF
#define SMOOTH_FONT

#define SPI_FREQUENCY  27000000 // This display can′t work in higher frequencies

// Optional reduced SPI frequency for reading TFT
#define SPI_READ_FREQUENCY  20000000

// Optional reduced SPI frequency for reading TFT
#define SPI_READ_FREQUENCY  5000000