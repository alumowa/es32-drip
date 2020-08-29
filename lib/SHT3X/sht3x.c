#include "sht3x.h"


static void sht3x_read_into(sht3x_t* sht3x, uint8_t data[], int length) {

	i2c_cmd_handle_t cmd_handle = i2c_cmd_link_create();
	i2c_master_start(cmd_handle);
	i2c_master_write_byte(cmd_handle, (sht3x->address << 1) | I2C_MASTER_READ, true);
	i2c_master_read(cmd_handle, data, length, I2C_MASTER_ACK);
	i2c_master_stop(cmd_handle);
	i2c_master_cmd_begin(sht3x->i2cport, cmd_handle, 1000 / portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd_handle);
}

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

int sht3x_initialize(sht3x_t* sht3x, i2c_config_t i2c_config) {

	esp_err_t err;

	//Configure & install i2c drivers.
	err = i2c_param_config(sht3x->i2cport, &i2c_config);
	if(err != ESP_OK) return SHT3X_ERR_I2CCONFIG;

	err = i2c_driver_install(sht3x->i2cport, I2C_MODE_MASTER, 0, 0, 0);
	if(err != ESP_OK) return SHT3X_ERR_I2CDRVINS;

	return SHT3X_OK;
}

void sht3x_readOnce(sht3x_t* sht3x, sht3x_reading_t* sht3x_reading, sht3x_oneshot_t mode){

	sht3x_write_command(sht3x, mode);

	uint8_t result_raw[6] = {1, 2, 3, 4, 5, 6};
	sht3x_read_into(sht3x, result_raw, sizeof(result_raw));

	uint16_t temp_raw = (result_raw[0] << 8) | result_raw[1];
	uint16_t hum_raw = (result_raw[3] << 8) | result_raw[4];

	sht3x_reading->temp_c = -45 + (175 * temp_raw / (65535.0 - 1));
	sht3x_reading->temp_f = -49 + (315 * temp_raw / (65535.0 - 1));
	sht3x_reading->hum = 100 * hum_raw / 65535.0;
}

void sht3x_status(sht3x_t* sht3x, sht3x_status_t* sht3x_status) {


	sht3x_write_command(sht3x, 0xF32D);

	uint8_t result_raw[2] = {0};
	sht3x_read_into(sht3x, result_raw, 2);
	uint16_t result = result_raw[0] << 8 | result_raw[1];

	sht3x_status->alert = (result & 0x8000) ? ALERT_PENDING : ALERT_NONE;
	sht3x_status->heater = (result & 0x2000) ? HEAT_ON : HEAT_OFF;
	sht3x_status->rhalert = (result & 0x800) ? RH_ALERT : RH_ALERT_NONE;
	sht3x_status->talert = (result & 0x400) ? T_ALERT : T_ALERT_NONE;
	sht3x_status->reset = (result & 0x10) ? RESET_DETECT : RESET_DETECT_CLEAR;
	sht3x_status->cmd_status = (result & 0x2) ? CMD_FAIL : CMD_SUCCESS;
	sht3x_status->crc_status = (result & 0x1) ? CRC_FAIL : CRC_SUCCESS;
}