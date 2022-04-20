/*
 * mk_i2c.h		RTOS ESP8266
 *
 *  Created on: 12 mar 2022
 *      Author: Miros³aw Kardaœ
 */

#ifndef COMPONENTS_MK_I2C_INCLUDE_MK_I2C_H_
#define COMPONENTS_MK_I2C_INCLUDE_MK_I2C_H_

#define I2C_SCL_PIN         	5               /*!< gpio number for I2C master clock */
#define I2C_SDA_PIN        		4               /*!< gpio number for I2C master data  */


//******** Wywo³ania Mutexów z funkcj¹ return ***************
#define I2C_TAKE_MUTEX do { \
		esp_err_t _err_ = i2c_take_mutex(); \
        if( _err_ != ESP_OK ) return _err_;\
    } while (0)

#define I2C_GIVE_MUTEX do { \
		esp_err_t _err_ = i2c_give_mutex(); \
        if (_err_ != ESP_OK) return _err_;\
    } while (0)


//******** Wywo³ania Mutexów bez funkcji return ***************
#define I2C_TAKE_MUTEX_NORET do { \
        i2c_take_mutex(); \
    } while (0)

#define I2C_GIVE_MUTEX_NORET do { \
        i2c_give_mutex(); \
    } while (0)






extern esp_err_t i2c_take_mutex( void );
extern esp_err_t i2c_give_mutex( void );



extern void force_i2c_stop( uint8_t on );

extern esp_err_t  i2c_init( uint8_t port_nr, uint8_t scl_pin_nr, uint8_t sda_pin_nr );

extern esp_err_t i2c_check_dev( uint8_t port_nr, uint8_t slave_addr );

extern esp_err_t i2c_write_byte_to_dev( uint8_t port_nr, uint8_t slave_addr, uint8_t byte );
extern esp_err_t i2c_write_word_to_dev( uint8_t port_nr, uint8_t slave_addr, uint16_t word );

extern esp_err_t i2c_read_byte_from_dev( uint8_t port_nr, uint8_t slave_addr, uint8_t * byte );
extern esp_err_t i2c_read_word_from_dev( uint8_t port_nr, uint8_t slave_addr, uint16_t * word );


extern esp_err_t i2c_dev_read( uint8_t port_nr, uint8_t slave_addr,
		const void *out_data, size_t out_size,
		void *in_data, size_t in_size );


extern esp_err_t i2c_dev_write( uint8_t port_nr, uint8_t slave_addr,
		const void *out_reg, size_t out_reg_size,
		const void *out_data, size_t out_size);


extern esp_err_t i2c_dev_read_reg( uint8_t port_nr, uint8_t slave_addr, uint8_t reg,
        void *in_data, size_t in_size);

extern esp_err_t i2c_dev_write_reg( uint8_t port_nr, uint8_t slave_addr, uint8_t reg,
        const void *out_data, size_t out_size);


#endif /* COMPONENTS_MK_I2C_INCLUDE_MK_I2C_H_ */
