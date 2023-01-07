#include "main.h"


//-----------------------Prototipos das funcoes-----------------------//
/*
void I2CInit(void);
void I2CWrite(uint8_t reg_addr, uint16_t data);
uint16_t I2CRead(uint8_t reg_addr);
void I2CWriteConf(uint8_t data);
*/
//-----------------------Definicao das funcoes-----------------------//

//initialize I2C module 1
void I2CInit(void){
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C1);    //enable I2C module 1

    SysCtlPeripheralReset(SYSCTL_PERIPH_I2C1);     //reset module

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);   //enable GPIO peripheral that contains I2C 1

    GPIOPinConfigure(GPIO_PA6_I2C1SCL);            // Configure the pin muxing for I2C1 functions on port B2 and B3
    GPIOPinConfigure(GPIO_PA7_I2C1SDA);

    GPIOPinTypeI2CSCL(GPIO_PORTA_BASE, GPIO_PIN_6);
    GPIOPinTypeI2C(GPIO_PORTA_BASE, GPIO_PIN_7);   // Select the I2C function for the pins

    // Enable and initialize the I2C0 master module.  Use the system clock for
    // the I2C1 module.  The last parameter sets the I2C data transfer rate.
    // If false the data rate is set to 100kbps and if true the data rate will
    // be set to 400kbps.
    I2CMasterInitExpClk(I2C1_BASE, SysCtlClockGet(), false);

    //clear I2C FIFOs
    //HWREG(I2C1_BASE + I2C_1_FIFOCTL) = 80008000;
}

void I2CWrite(uint8_t reg_addr, uint16_t data){
    uint8_t dataMSB, dataLSB;

    dataLSB = data & 0x00FF;
    dataMSB = data >> 8;

    I2CMasterSlaveAddrSet(I2C1_BASE, SLAVE_ADDR, false);

    I2CMasterDataPut(I2C1_BASE, reg_addr);
    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_START);
    while(I2CMasterBusy(I2C1_BASE));

    I2CMasterDataPut(I2C1_BASE, dataMSB);
    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
    while(I2CMasterBusy(I2C1_BASE));

    I2CMasterDataPut(I2C1_BASE, dataLSB);
    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
    while(I2CMasterBusy(I2C1_BASE));
}

void I2CWriteConf(uint8_t data){
    I2CMasterSlaveAddrSet(I2C1_BASE, SLAVE_ADDR, false);

    I2CMasterDataPut(I2C1_BASE, REG_CONFIG_ADDR);
    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_START);
    while(I2CMasterBusy(I2C1_BASE));

    I2CMasterDataPut(I2C1_BASE, data);
    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
    while(I2CMasterBusy(I2C1_BASE));
}

uint16_t I2CRead(uint8_t reg_addr){
    uint16_t data = 0;
    //uint8_t dataMSB, dataLSB;
    uint32_t dataMSB, dataLSB;

    I2CMasterSlaveAddrSet(I2C1_BASE, SLAVE_ADDR, false);

    I2CMasterDataPut(I2C1_BASE, reg_addr);
    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_SINGLE_SEND);
    while(I2CMasterBusy(I2C1_BASE));

    I2CMasterSlaveAddrSet(I2C1_BASE, SLAVE_ADDR, true);

    dataLSB = I2CMasterDataGet(I2C1_BASE);
    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);
    while(I2CMasterBusy(I2C1_BASE));

    dataMSB = I2CMasterDataGet(I2C1_BASE);
    I2CMasterControl(I2C1_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
    while(I2CMasterBusy(I2C1_BASE));

    dataLSB = dataLSB >> 4;                 //primeiro envia o byte mais significativo que corresponde ao dataMSB

    data = dataMSB << 4;                  //em vez de fzr de 8 bits faco de 4 para fcr com o valor guardado nos 12 bits menos significativos
    data += dataLSB;

    return data;
}
