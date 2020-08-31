
#include "driver/i2c.h"
#include "esp_types.h"

#ifndef _SHT3X_H
#define _SHT3X_H

#define SHT3X_ADDRESS           0x45

#define SHT3X_OK                0x0
#define SHT3X_ERR_I2CCONFIG     0x01
#define SHT3X_ERR_I2CDRVINS     0x02
#define SHT3X_ERR_CRC           0x03



typedef enum {
  SHT3X_CS_HIGHREP =  0x2C06,
  SHT3X_CS_MEDREP =   0x2C0D,
  SHT3X_CS_LOWREP =   0x2C10,
  SHT3X_HIGHREP =     0x2400,
  SHT3X_MEDREP =      0x240B,
  SHT3X_LOWREP =      0x2416
} sht3x_oneshot_t;

typedef enum {
  SHT3X_PER_005_HIGHREP = 0x2032,
  SHT3X_PER_005_MEDREP =  0x2024,
  SHT3X_PER_005_LOWREP =  0x202F,
  SHT3X_PER_01_HIGHREP =  0x2130,
  SHT3X_PER_01_MEDREP =   0x2126,
  SHT3X_PER_01_LOWREP =   0x212D,
  SHT3X_PER_02_HIGHREP =  0x2236,
  SHT3X_PER_02_MEDREP =   0x2220,
  SHT3X_PER_02_LOWREP =   0x222B,
  SHT3X_PER_04_HIGHREP =  0x2334,
  SHT3X_PER_04_MEDREP =   0x2322,
  SHT3X_PER_04_LOWREP =   0x2329,
  SHT3X_PER_10_HIGHREP =  0x2737,
  SHT3X_PER_10_MEDREP =   0x2721,
  SHT3X_PER_10_LOWREP =   0x272A,
  SHT3X_PER_ART =         0x2B32
} sht3x_periodic_t;

typedef enum {
  ALERT_NONE = 0,
  ALERT_PENDING
} sht3x_alert_t;

typedef enum {
  HEAT_OFF = 0,
  HEAT_ON
} sht3x_heater_t;

typedef enum {
  RH_ALERT_NONE = 0,
  RH_ALERT
} sht3x_rhalert_t;

typedef enum {
  T_ALERT_NONE = 0,
  T_ALERT
} sht3x_talert_t;

typedef enum {
  RESET_DETECT_CLEAR = 0,
  RESET_DETECT
} sht3x_resetd_t;

typedef enum {
  CMD_SUCCESS = 0,
  CMD_FAIL
} sht3x_lstcmd_t;

typedef enum {
  CRC_SUCCESS = 0,
  CRC_FAIL
} sht3x_lstcrc_t;

typedef struct {
  sht3x_alert_t alert;        /* Pending alerts flag */
  sht3x_heater_t heater;      /* Heater status */
  sht3x_rhalert_t rhalert;    /* Rel hum tracking alert flag */
  sht3x_talert_t talert;      /* Temp tracking alert flag */
  sht3x_resetd_t reset;       /* Reset detect flag */
  sht3x_lstcmd_t cmd_status;  /* Last command status result */
  sht3x_lstcrc_t crc_status;  /* Last write crc status */
  bool valid;                 /* Set when crc passes */
} sht3x_status_t;

typedef struct {
  float temp_c;               /* Temperature reading in celsius */
  float temp_f;               /* Temperature reading in fahrenheit */
  float hum;                  /* Relative humidity reading in % */
  bool valid;                 /* Set when crc passes */
} sht3x_reading_t;

typedef struct {
  int address;
  i2c_port_t i2cport;
} sht3x_t;


int sht3x_initialize(sht3x_t* sht3x, i2c_config_t i2c_config);
void sht3x_oneshot(sht3x_t* sht3x, sht3x_reading_t* sht3x_reading, sht3x_oneshot_t mode);
void sht3x_set_periodic(sht3x_t* sht3x, sht3x_periodic_t mode);
void sht3x_fetch_periodic(sht3x_t* sht3x, sht3x_reading_t* sht3x_reading);
void sht3x_status(sht3x_t* sht3x, sht3x_status_t* sht3x_status);

#endif // _SHT3X_H
