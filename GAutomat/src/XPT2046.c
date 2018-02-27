/* Includes ------------------------------------------------------------------*/
#include "XPT2046.h"
#include "SPI1.h"
#include "timer.h"

#define XPT2046_CS_PIN                     GPIO_Pin_11
#define XPT2046_CS_GPIO_PORT               GPIOC
#define XPT2046_CS_SCK_GPIO_CLK            RCC_AHB1Periph_GPIOC

#define XPT2046_IRQ_PIN                    GPIO_Pin_10
#define XPT2046_IRQ_GPIO_PORT              GPIOC
#define XPT2046_IRQ_SCK_GPIO_CLK           RCC_AHB1Periph_GPIOC

#define XPT2046_CS_ENABLE                  (XPT2046_CS_GPIO_PORT->BSRRH = XPT2046_CS_PIN)
#define XPT2046_CS_DISABLE                 (XPT2046_CS_GPIO_PORT->BSRRL = XPT2046_CS_PIN)

#define XPT2046_SPI_PRESCALER              spi_br_64  // SPI1 na APB2 100MHz

#define CHX   0x92
#define CHY   0xD2

#define XPT2046_AD_SAMPLES    10

/* Private variables ---------------------------------------------------------*/
static Matrix      matrix;
static Coordinate  screen;

/* Private define ------------------------------------------------------------*/
#define THRESHOLD 2

// controller specific functions - should not be used outside
static bool read_IRQ(void);
static void WR_CMD (uint8_t cmd);
static uint16_t RD_AD(void);

PtrXPT2046ClickCallback pXPT2046ClickCallback = 0;

void XPT2046_Init(void) 
{ 
  GPIO_InitTypeDef GPIO_InitStruct;

  // SPI1 nakonfigurovano od driveru displeje ILI9163, budeme jenom menit rychlost

  /* CS pin */
  RCC_AHB1PeriphClockCmd(XPT2046_CS_SCK_GPIO_CLK, ENABLE);
  GPIO_InitStruct.GPIO_Pin = XPT2046_CS_PIN;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_25MHz;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(XPT2046_CS_GPIO_PORT, &GPIO_InitStruct);

  XPT2046_CS_DISABLE;

  /* IRQ pin */
  RCC_AHB1PeriphClockCmd(XPT2046_IRQ_SCK_GPIO_CLK, ENABLE);
  GPIO_InitStruct.GPIO_Pin =  XPT2046_IRQ_PIN;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(XPT2046_IRQ_GPIO_PORT, &GPIO_InitStruct);
}

uint16_t Read_X(void)  
{  
  XPT2046_CS_ENABLE;
  XPT2046_CS_ENABLE;
  WR_CMD(CHX); 
  uint16_t curr_X = RD_AD();
  XPT2046_CS_DISABLE;
  return curr_X;    
}

uint16_t Read_Y(void)  
{
  XPT2046_CS_ENABLE;
  XPT2046_CS_ENABLE;
  WR_CMD(CHY); 
  uint16_t curr_Y = RD_AD();
  XPT2046_CS_DISABLE;
  return curr_Y;     
}

void XPT2046_GetAdXY(uint16_t *x, uint16_t *y)
{
  *x = Read_X();
  *y = Read_Y();
}

static void WR_CMD (uint8_t cmd)
{
  while (!(SPI1->SR & SPI_I2S_FLAG_TXE));
  SPI_SendData(SPI1, cmd);
  while (!(SPI1->SR & SPI_I2S_FLAG_TXE));
  while (SPI1->SR & SPI_I2S_FLAG_BSY);
}

Coordinate *Read_XPT2046(void)
{
  uint16_t buffer[2][XPT2046_AD_SAMPLES];
  uint16_t x, y;
  uint8_t count = 0;

  SPI1_SetPrescaler(XPT2046_SPI_PRESCALER);

  while(!read_IRQ() && count < XPT2046_AD_SAMPLES)
  {		   
    XPT2046_GetAdXY(&x, &y);
	  buffer[0][count] = x;
	  buffer[1][count] = y;
	  count++;  
  }

  // average X Y
  if(count == XPT2046_AD_SAMPLES)
  {
    uint32_t tempX = 0;
    uint32_t tempY = 0;
    for (uint8_t i = 2; i < XPT2046_AD_SAMPLES; ++i)  // vynechame prvni 2 vzorky
    {
      tempX += buffer[0][i];
      tempY += buffer[1][i];

      screen.x = tempX >> 3;  // :8
      screen.y = tempY >> 3;
    }

    return &screen;
  }

  return 0; 
}
	 
FunctionalState XPT2046_SetCalibrationMatrix( Coordinate * displayPtr, Coordinate * screenPtr)
{
  FunctionalState retTHRESHOLD = ENABLE ;
  /* K£½(X0£­X2) (Y1£­Y2)£­(X1£­X2) (Y0£­Y2) */
  matrix.Divider = ((screenPtr[0].x - screenPtr[2].x) * (screenPtr[1].y - screenPtr[2].y)) -
                       ((screenPtr[1].x - screenPtr[2].x) * (screenPtr[0].y - screenPtr[2].y));
  if( matrix.Divider == 0 )
  {
    retTHRESHOLD = DISABLE;
  }
  else
  {
    /* A£½((XD0£­XD2) (Y1£­Y2)£­(XD1£­XD2) (Y0£­Y2))£¯K	*/
    matrix.An = ((displayPtr[0].x - displayPtr[2].x) * (screenPtr[1].y - screenPtr[2].y)) -
                    ((displayPtr[1].x - displayPtr[2].x) * (screenPtr[0].y - screenPtr[2].y)) ;
	/* B£½((X0£­X2) (XD1£­XD2)£­(XD0£­XD2) (X1£­X2))£¯K	*/
    matrix.Bn = ((screenPtr[0].x - screenPtr[2].x) * (displayPtr[1].x - displayPtr[2].x)) -
                    ((displayPtr[0].x - displayPtr[2].x) * (screenPtr[1].x - screenPtr[2].x)) ;
    /* C£½(Y0(X2XD1£­X1XD2)+Y1(X0XD2£­X2XD0)+Y2(X1XD0£­X0XD1))£¯K */
    matrix.Cn = (screenPtr[2].x * displayPtr[1].x - screenPtr[1].x * displayPtr[2].x) * screenPtr[0].y +
                    (screenPtr[0].x * displayPtr[2].x - screenPtr[2].x * displayPtr[0].x) * screenPtr[1].y +
                    (screenPtr[1].x * displayPtr[0].x - screenPtr[0].x * displayPtr[1].x) * screenPtr[2].y ;
    /* D£½((YD0£­YD2) (Y1£­Y2)£­(YD1£­YD2) (Y0£­Y2))£¯K	*/
    matrix.Dn = ((displayPtr[0].y - displayPtr[2].y) * (screenPtr[1].y - screenPtr[2].y)) -
                    ((displayPtr[1].y - displayPtr[2].y) * (screenPtr[0].y - screenPtr[2].y)) ;
    /* E£½((X0£­X2) (YD1£­YD2)£­(YD0£­YD2) (X1£­X2))£¯K	*/
    matrix.En = ((screenPtr[0].x - screenPtr[2].x) * (displayPtr[1].y - displayPtr[2].y)) -
                    ((displayPtr[0].y - displayPtr[2].y) * (screenPtr[1].x - screenPtr[2].x)) ;
    /* F£½(Y0(X2YD1£­X1YD2)+Y1(X0YD2£­X2YD0)+Y2(X1YD0£­X0YD1))£¯K */
    matrix.Fn = (screenPtr[2].x * displayPtr[1].y - screenPtr[1].x * displayPtr[2].y) * screenPtr[0].y +
                    (screenPtr[0].x * displayPtr[2].y - screenPtr[2].x * displayPtr[0].y) * screenPtr[1].y +
                    (screenPtr[1].x * displayPtr[0].y - screenPtr[0].x * displayPtr[1].y) * screenPtr[2].y ;
  }

  return( retTHRESHOLD ) ;
}

FunctionalState getDisplayPoint(Coordinate * displayPtr, Coordinate * screenPtr)
{
  FunctionalState retTHRESHOLD = ENABLE;

  if (matrix.Divider != 0)
  {
    /* XD = AX+BY+C */        
    displayPtr->x = ((matrix.An * screenPtr->x) +
                      (matrix.Bn * screenPtr->y) +
                       matrix.Cn) / matrix.Divider ;
	/* YD = DX+EY+F */        
    displayPtr->y = ((matrix.Dn * screenPtr->x) +
                      (matrix.En * screenPtr->y) +
                       matrix.Fn) / matrix.Divider ;
  }
  else
  {
    retTHRESHOLD = DISABLE;
  }

  return(retTHRESHOLD);
} 

static bool read_IRQ(void)
{
  return XPT2046_IRQ_GPIO_PORT->IDR & XPT2046_IRQ_PIN;
}

bool XPT2046_Press(void)
{
  bool bResult = !read_IRQ();

  // cekani na uvolneni
  if (bResult)
  {
    uint8_t nCheckTime = Timer_GetTicks_ms();
    while (true)
    {
      if (!read_IRQ())
      {
        nCheckTime = Timer_GetTicks_ms();
      }

      if (Timer_GetTicks_ms() > nCheckTime + 300)
      {
        break;
      }
    }

    if (pXPT2046ClickCallback)
    {
      return pXPT2046ClickCallback();
    }
  }

  return bResult;
}

static uint16_t RD_AD(void)  
{ 
  uint16_t buf;
  uint8_t temp;

  // clear RX flag
  temp = SPI1->DR;

  while (!(SPI1->SR & SPI_I2S_FLAG_TXE));
  SPI_SendData(SPI1, 0x0000);

  /* Wait for SPI data reception */
  while (!(SPI1->SR & SPI_I2S_FLAG_RXNE));
  temp = SPI_ReceiveData(SPI1);
//  temp = SPI1->DR;

  buf = temp << 8;
  XPT2046_CS_ENABLE;   // Todo: wait, zatim nevim na co

  while (!(SPI1->SR & SPI_I2S_FLAG_TXE));
  SPI_SendData(SPI1, 0x0000);

  while (!(SPI1->SR & SPI_I2S_FLAG_RXNE));
  temp = SPI_ReceiveData(SPI1);
//  temp = SPI1->DR;

  buf |= temp; 
  buf >>= 3;
  buf &= 0xfff;
  return buf; 
}

void XPT2046_SetClickCallback(PtrXPT2046ClickCallback pFunction)
{
  pXPT2046ClickCallback = pFunction;
}
