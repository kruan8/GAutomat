/*
 * DHT.h
 *
 *  Created on: 23. 3. 2017
 *      Author: priesolv
 */

#ifndef DHT_H_
#define DHT_H_

#include "stm32f4xx.h"

/* Exported constants --------------------------------------------------------*/
#define MAX_TICS 10000
#define DHT11_OK 0
#define DHT11_NO_CONN 1
#define DHT11_CS_ERROR 2

/* Exported macro ------------------------------------------------------------*/
typedef enum
{
  dht_in = 0,
  dht_out,
}dht_dir_e;

typedef struct
{
  int16_t Temp; /*!< Temperature in tenths of degrees.
                     If real temperature is 27.3°C, this variable's value is 273 */
  uint16_t Hum; /*!< Humidity in tenths of percent.
                     If real humidity is 55.5%, this variable's value is 555 */
}dht_data_t;

typedef enum
{
  DHT_OK,                     /*!< Data OK */
  DHT_ERROR,                  /*!< An error occurred */
  DHT_CONNECTION_ERROR,       /*!< Device is not connected */
  DHT_WAITHIGH_ERROR,         /*!< Wait high pulse timeout */
  DHT_WAITLOW_ERROR,          /*!< Wait low pulse timeout */
  DHT_WAITHIGH_LOOP_ERROR,    /*!< Loop error for high pulse */
  DHT_WAITLOW_LOOP_ERROR,     /*!< Loop error for low pulse */
  DHT_PARITY_ERROR            /*!< Data read fail */
} dht_error_e;

/* Exported functions ------------------------------------------------------- */
void DHT_Init(uint32_t nPCLK_Frequency);
dht_error_e DHT_GetData(dht_data_t* data);

#endif /* DHT_H_ */
