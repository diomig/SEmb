/*
 * i2c.h
 *
 *  Created on: 05/01/2023
 *      Author: diomi
 */

#ifndef I2C_H_
#define I2C_H_



#define SLAVE_ADDR              0x48
#define REG_TEMP_ADDR           0x00
#define REG_CONFIG_ADDR         0x01
#define REG_TLOW_ADDR           0x02
#define REG_THIGH_ADDR          0x03

void I2CInit(void);
void I2CWrite(uint8_t reg_addr, uint16_t data);
uint16_t I2CRead(uint8_t reg_addr);
void I2CWriteConf(uint8_t data);

#endif /* I2C_H_ */
