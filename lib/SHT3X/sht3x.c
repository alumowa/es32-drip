#include "sht3x.h"

static void sht3x_write_command(sht3x_t* sht3x, uint16_t command){

	//SHT3X takes 2 byte commands
	uint8_t CMD[2] = {command >> 8, command & 0xFF};

	i2c_cmd_handle_t cmd_handle = i2c_cmd_link_create();
	i2c_master_start(cmd_handle);
	i2c_master_write_byte(cmd_handle, (sht3x->address << 1) | I2C_MASTER_WRITE, true);
	i2c_master_write(cmd_handle, CMD, sizeof(CMD), true);
	i2c_master_stop(cmd_handle);
	i2c_master_cmd_begin(sht3x->i2cport, cmd_handle, 1000 / portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd_handle);
}

static bool sht3x_crc(uint8_t* result){

	const int len = 6;		//[MSB][LSB][CRC]
	const int width = 8;
	uint8_t crc = 0xFF;		//Initial value
	uint8_t poly = 0x31;	//(x^8 + x^5 + x^4 + 1)

	int c;

	for(int i = 0; i < len; i++){

		//Validate every 3rd byte. Bail if fail otherwise
		//reset to crc initial.
		if(i % 3 == 2){
			if(crc != result[i]) return false;
			crc = 0xFF;
		}else{
			crc ^= result[i];

			for ( c = 0; c < width; c++ ) {
				crc = ( crc & 0x80 )? (crc << 1) ^ poly : (crc << 1);
			}
		}
	}

	return true;
}

static int sht3x_read_response_into(sht3x_t* sht3x, uint8_t data[], int length) {

	i2c_cmd_handle_t cmd_handle = i2c_cmd_link_create();
	i2c_master_start(cmd_handle);
	i2c_master_write_byte(cmd_handle, (sht3x->address << 1) | I2C_MASTER_READ, true);
	i2c_master_read(cmd_handle, data, length, I2C_MASTER_ACK);
	i2c_master_stop(cmd_handle);
	i2c_master_cmd_begin(sht3x->i2cport, cmd_handle, 1000 / portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd_handle);

	//CRC check
	if(!sht3x_crc(data)){
		return SHT3X_ERR_CRC;
	}

	return SHT3X_OK;
}
static void sht3x_get_measurements(sht3x_t* sht3x, sht3x_reading_t* sht3x_reading) {

	uint8_t result_raw[6] = {0};
	int res = sht3x_read_response_into(sht3x, result_raw, sizeof(result_raw));

	//If crc mismatch, set valid flag to false and bail
	if(res == SHT3X_ERR_CRC){
		sht3x_reading->valid = false;
		return;
	}

	//Parse out to units
	uint16_t temp_raw = (result_raw[0] << 8) | result_raw[1];
	uint16_t hum_raw = (result_raw[3] << 8) | result_raw[4];

	sht3x_reading->temp_c = -45 + (175 * temp_raw / (65535.0 - 1));
	sht3x_reading->temp_f = 32 + (sht3x_reading->temp_c * 1.8);
	sht3x_reading->hum = 100 * hum_raw / 65535.0;
	sht3x_reading->valid = true;
}


int sht3x_initialize(sht3x_t* sht3x, i2c_config_t i2c_config) {

	esp_err_t err;

	//Configure & install i2c drivers.
	err = i2c_param_config(sht3x->i2cport, &i2c_config);
	if(err != ESP_OK) return SHT3X_ERR_I2CCONFIG;

	err = i2c_driver_install(sht3x->i2cport, I2C_MODE_MASTER, 0, 0, 0);
	if(err != ESP_OK) return SHT3X_ERR_I2CDRVINS;

	return SHT3X_OK;
}

void sht3x_oneshot(sht3x_t* sht3x, sht3x_reading_t* sht3x_reading, sht3x_oneshot_t mode){

	sht3x_write_command(sht3x, mode);
	sht3x_get_measurements(sht3x, sht3x_reading);
}

void sht3x_set_periodic(sht3x_t* sht3x, sht3x_periodic_t mode) {

	sht3x_write_command(sht3x, mode);
}

void sht3x_fetch_periodic(sht3x_t* sht3x, sht3x_reading_t* sht3x_reading){

	//0xE000 - Fetch periodic measurements
	sht3x_write_command(sht3x, 0xE000);
	sht3x_get_measurements(sht3x, sht3x_reading);
}

void sht3x_status(sht3x_t* sht3x, sht3x_status_t* sht3x_status) {

	//0xF32D - Fetch status register
	sht3x_write_command(sht3x, 0xF32D);

	uint8_t result_raw[2] = {0};
	int res = sht3x_read_response_into(sht3x, result_raw, 2);

	if(res == SHT3X_ERR_CRC){
		sht3x_status->valid = false;
		return;
	}

	uint16_t result = result_raw[0] << 8 | result_raw[1];

	sht3x_status->alert = (result & 0x8000) ? ALERT_PENDING : ALERT_NONE;
	sht3x_status->heater = (result & 0x2000) ? HEAT_ON : HEAT_OFF;
	sht3x_status->rhalert = (result & 0x800) ? RH_ALERT : RH_ALERT_NONE;
	sht3x_status->talert = (result & 0x400) ? T_ALERT : T_ALERT_NONE;
	sht3x_status->reset = (result & 0x10) ? RESET_DETECT : RESET_DETECT_CLEAR;
	sht3x_status->cmd_status = (result & 0x2) ? CMD_FAIL : CMD_SUCCESS;
	sht3x_status->crc_status = (result & 0x1) ? CRC_FAIL : CRC_SUCCESS;
	sht3x_status->valid = true;
}