#include <stdbool.h>
#include "sst26_flash.h"
#include "log.h"
#include "stdio.h"


#define CONFIG SST26_config

#define SST26_PAGE_SIZE 256
#define SST26_SECTOR_SIZE 4096
#define SST26_MEMORY_SIZE 0x800000

#define SST26_SREG_BUSY_MASK 0x01

// ==CONFIG==
// noop
#define NOP 0x00
// reset enable
#define RSTEN 0x66
// reset
#define RST 0x99
// enable QuadIO (QSPI)
#define EQIO 0x38
// disable QuadIO
#define RSTQIO 0xFF
// read status register
#define RDSR 0x05
// write status register
#define WRSR 0x01
// read configuration register
#define RDCR 0x35

// ==READ==
// read memory
#define READ 0x03
// high speed read
#define HSPI_RD 0x0B

// ==ID==
// JEDEC-ID read
#define JEDEC_ID 0x9F
// QUAD IO J-ID read
#define QUAD_JEDEC_ID 0xAF
// Serial flash discoverable parameters
#define SFDP 0x5A

#define WREN 0x06
#define WRDI 0x04

#define SE 0x20 // sector erase  -> 4kBytes of memory
#define BE 0xD8 // block erase -> 64, 32 or 8 kBytes of memory. Depends on block
#define CE 0xC7 // chip erase
#define PP 0x02 // page program

#define ULBPR 0x98 // global unlock block protection register

#define WRITE_ENABLE_STATUS_MASK 0x02


//#define ENABLE_LOG


static struct tModeMask {
	uint32_t AddressModeMask;
	uint32_t DataModeMask;
	uint32_t InstructionModeMask;
	enum {
		MODE_SPI = 0, MODE_QSPI
	} mode;

} mode_mask = { .AddressModeMask = QSPI_ADDRESS_1_LINE, .DataModeMask =
		QSPI_DATA_1_LINE, .InstructionModeMask = QSPI_INSTRUCTION_1_LINE,
		.mode = MODE_SPI };



static void set_1_line_mode_mask() {
#ifdef ENABLE_LOG
	log_info("set_1_line_mode_mask");
#endif
	mode_mask.AddressModeMask = QSPI_ADDRESS_1_LINE;
	mode_mask.DataModeMask = QSPI_DATA_1_LINE;
	mode_mask.InstructionModeMask = QSPI_INSTRUCTION_1_LINE;
	mode_mask.mode = MODE_SPI;
}

static void set_4_line_mode_mask() {
#ifdef ENABLE_LOG
	log_info("set_4_line_mode_mask");
#endif
	mode_mask.AddressModeMask = QSPI_ADDRESS_4_LINES;
	mode_mask.DataModeMask = QSPI_DATA_4_LINES;
	mode_mask.InstructionModeMask = QSPI_INSTRUCTION_4_LINES;
	mode_mask.mode = MODE_QSPI;
}

static QSPI_CommandTypeDef command = {
		.DdrMode = QSPI_DDR_MODE_DISABLE, // final GLOBAL STATE
		.SIOOMode = QSPI_SIOO_INST_EVERY_CMD, // final GLOBAL STATE
		.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE, // final GLOBAL STATE
		.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS, // final GLOBAL STATE
		.AlternateBytes = 0, // final GLOBAL STATE
		.AddressSize = QSPI_ADDRESS_24_BITS, // final GLOBAL STATE

		.AddressMode = QSPI_ADDRESS_NONE, .DataMode = QSPI_DATA_1_LINE,
		.InstructionMode = QSPI_INSTRUCTION_1_LINE,

		.Instruction = 0, .Address = 0, .DummyCycles = 0, .NbData = 0, };

static void qspi_error(){
	log_error("QSPI error: 0x%02X, state: 0x%02X", HAL_QSPI_GetError(CONFIG.hqspi), HAL_QSPI_GetState(CONFIG.hqspi));
}

static HAL_StatusTypeDef enable_qspi() {
#ifdef ENABLE_LOG
	log_info("enable_qspi");
#endif
	if (mode_mask.mode != MODE_SPI) {
		return HAL_ERROR;
	}

	HAL_StatusTypeDef status;

	command.AddressMode = QSPI_ADDRESS_NONE;
	command.DataMode = QSPI_DATA_NONE;
	command.InstructionMode = QSPI_INSTRUCTION_1_LINE;

	command.Instruction = EQIO;
	command.DummyCycles = 0;
	command.NbData = 0;

	status = HAL_QSPI_Command(CONFIG.hqspi, &command, CONFIG.timeout);
	if (status != HAL_OK) {
		return status;
	}

	set_4_line_mode_mask();
	return HAL_OK;
}

static HAL_StatusTypeDef spi_reset() {
#ifdef ENABLE_LOG
	log_info("spi_reset");
#endif
	HAL_StatusTypeDef status;

	command.AddressMode = QSPI_ADDRESS_NONE;
	command.DataMode = QSPI_DATA_NONE;
	command.InstructionMode = QSPI_INSTRUCTION_1_LINE;

	command.Instruction = RSTEN;
	command.DummyCycles = 0;
	command.NbData = 0;

	status = HAL_QSPI_Command(CONFIG.hqspi, &command, CONFIG.timeout);
	if (status != HAL_OK) {
		return status;
	}

	command.AddressMode = QSPI_ADDRESS_NONE;
	command.DataMode = QSPI_DATA_NONE;
	command.InstructionMode = QSPI_INSTRUCTION_1_LINE;

	command.Instruction = RST;
	command.DummyCycles = 0;
	command.NbData = 0;

	status = HAL_QSPI_Command(CONFIG.hqspi, &command, CONFIG.timeout);
	if (status != HAL_OK) {
		return status;
	}
	set_1_line_mode_mask();
	return HAL_OK;
}

static HAL_StatusTypeDef qspi_reset() {
#ifdef ENABLE_LOG
	log_info("qspi_reset");
#endif
	HAL_StatusTypeDef status;

	command.AddressMode = QSPI_ADDRESS_NONE;
	command.DataMode = QSPI_DATA_NONE;
	command.InstructionMode = QSPI_INSTRUCTION_4_LINES;

	command.Instruction = RSTEN;
	command.DummyCycles = 0;
	command.NbData = 0;

	status = HAL_QSPI_Command(CONFIG.hqspi, &command, CONFIG.timeout);
	if (status != HAL_OK) {
		return status;
	}

	command.AddressMode = QSPI_ADDRESS_NONE;
	command.DataMode = QSPI_DATA_NONE;
	command.InstructionMode = QSPI_INSTRUCTION_4_LINES;

	command.Instruction = RST;
	command.DummyCycles = 0;
	command.NbData = 0;

	status = HAL_QSPI_Command(CONFIG.hqspi, &command, CONFIG.timeout);
	if (status != HAL_OK) {
		return status;
	}
	set_1_line_mode_mask();
	return HAL_OK;
}

static HAL_StatusTypeDef full_reset() {
#ifdef ENABLE_LOG
	log_info("full_reset");
#endif
	HAL_StatusTypeDef status;

	if ((status = qspi_reset()) != HAL_OK) {
		return status;
	}
	if ((status = spi_reset()) != HAL_OK) {
			return status;
		}
	HAL_Delay(100);
	return HAL_OK;
}

static HAL_StatusTypeDef read_configuration_register(uint8_t *CONFIG_REG) {
#ifdef ENABLE_LOG
	log_info("read_configuration_register");
#endif
	HAL_StatusTypeDef status;

	command.AddressMode = QSPI_ADDRESS_NONE;
	command.DataMode = mode_mask.DataModeMask;
	command.InstructionMode = mode_mask.InstructionModeMask;

	command.Instruction = RDCR;
	command.NbData = 1;

	switch (mode_mask.mode) {
	case MODE_SPI:
		command.DummyCycles = 0;
		break;
	case MODE_QSPI:
	default:
		command.DummyCycles = 2;
		break;
	}

	status = HAL_QSPI_Command(CONFIG.hqspi, &command, CONFIG.timeout);
	if (status != HAL_OK) {
		return status;
	}

	return HAL_QSPI_Receive(CONFIG.hqspi, CONFIG_REG, CONFIG.timeout);
}



static HAL_StatusTypeDef write_configuration_register(uint8_t CONFIG_REG){
#ifdef ENABLE_LOG
	log_info("write_configuration_register: 0x%02x", CONFIG_REG);
#endif
	HAL_StatusTypeDef status;

	command.AddressMode = QSPI_ADDRESS_NONE;
	command.DataMode = mode_mask.DataModeMask;
	command.InstructionMode = mode_mask.InstructionModeMask;

	command.Instruction = WRSR;
	command.NbData = 2;
	command.DummyCycles = 0;

	status = HAL_QSPI_Command(CONFIG.hqspi, &command, CONFIG.timeout);
	if (status != HAL_OK) {
		return status;
	}

	uint8_t statusAndConfig[2] = {CONFIG_REG, CONFIG_REG};

	status = HAL_QSPI_Transmit(CONFIG.hqspi, statusAndConfig, CONFIG.timeout);
	return status;
}

static HAL_StatusTypeDef write_enable(){
#ifdef ENABLE_LOG
	log_info("write_enable");
#endif
	HAL_StatusTypeDef status;

	command.AddressMode = QSPI_ADDRESS_NONE;
	command.DataMode = QSPI_DATA_NONE;
	command.InstructionMode = mode_mask.InstructionModeMask;

	command.DummyCycles = 0;
	command.NbData = 0;
	command.Address = 0;

	command.Instruction = WREN;

	status = HAL_QSPI_Command(CONFIG.hqspi, &command, CONFIG.timeout);
	return status;
}

static HAL_StatusTypeDef global_unlock_block_protection(){
#ifdef ENABLE_LOG
	log_info("global_unlock_block_protection");
#endif
	HAL_StatusTypeDef status;

	command.AddressMode = QSPI_ADDRESS_NONE;
	command.DataMode = QSPI_DATA_NONE;
	command.InstructionMode = mode_mask.InstructionModeMask;

	command.Instruction = ULBPR;
	command.NbData = 0;
	command.DummyCycles = 0;

	status = HAL_QSPI_Command(CONFIG.hqspi, &command, CONFIG.timeout);
	return status;
}

static HAL_StatusTypeDef page_program(uint32_t address, uint32_t size, uint8_t *buffer){
#ifdef ENABLE_LOG
	log_info("page_program(%d, %d, 0x%08X)", address, size, (uint32_t)buffer);
#endif
	if(size < 1 || size > 256){
		log_error("Invalid page_program argument: size out of range");
		return HAL_ERROR;
	}
	HAL_StatusTypeDef status;

	command.AddressMode = mode_mask.AddressModeMask;
	command.DataMode = mode_mask.DataModeMask;
	command.InstructionMode = mode_mask.InstructionModeMask;

	command.DummyCycles = 0;
	command.NbData = size;
	command.Address = address;

	command.Instruction = PP;

	status = HAL_QSPI_Command(CONFIG.hqspi, &command, CONFIG.timeout);
	if(status != HAL_OK){
		log_error("page_program HAL_QSPI_Command: 0x%02X", status);
		return status;
	}

	status = HAL_QSPI_Transmit(CONFIG.hqspi, buffer, CONFIG.timeout);
	if(status!= HAL_OK){
		log_error("page_program HAL_QSPI_Transmit: 0x%02X", status);
		return status;
	}
	return HAL_OK;
}

static HAL_StatusTypeDef sector_erase(uint32_t address){
#ifdef ENABLE_LOG
	log_info("sector_erase %d", address);
#endif
	HAL_StatusTypeDef status;

	command.AddressMode = mode_mask.AddressModeMask;
	command.DataMode = QSPI_DATA_NONE;
	command.InstructionMode = mode_mask.InstructionModeMask;

	command.DummyCycles = 0;
	command.NbData = 0;
	command.Address = address;

	command.Instruction = SE;

	status = HAL_QSPI_Command(CONFIG.hqspi, &command, CONFIG.timeout);
	if(status!= HAL_OK){
		log_error("page_program HAL_QSPI_Command: 0x%02X", status);
		return status;
	}
	return HAL_OK;
}


static HAL_StatusTypeDef chip_erase(){
#ifdef ENABLE_LOG
	log_info("chip_erase");
#endif
	HAL_StatusTypeDef status;

	command.AddressMode = QSPI_ADDRESS_NONE;
	command.DataMode = QSPI_DATA_NONE;
	command.InstructionMode = mode_mask.InstructionModeMask;

	command.DummyCycles = 0;
	command.NbData = 0;
	command.Address = 0;

	command.Instruction = CE;

	status = HAL_QSPI_Command(CONFIG.hqspi, &command, CONFIG.timeout);
	if(status!= HAL_OK){
		return status;
	}
	return HAL_OK;
}



static HAL_StatusTypeDef spi_read_device_id(uint8_t *ID_REG) {
#ifdef ENABLE_LOG
	log_info("spi_read_device_id");
#endif
	if (mode_mask.mode != MODE_SPI) {
		return HAL_ERROR;
	}
	HAL_StatusTypeDef status;

	command.AddressMode = QSPI_ADDRESS_NONE;
	command.DataMode = QSPI_DATA_1_LINE;
	command.InstructionMode = QSPI_INSTRUCTION_1_LINE;

	command.Instruction = JEDEC_ID;
	command.DummyCycles = 0;
	command.NbData = 3;

	status = HAL_QSPI_Command(CONFIG.hqspi, &command, CONFIG.timeout);
	if (status != HAL_OK) {
		return status;
	}

	return HAL_QSPI_Receive(CONFIG.hqspi, ID_REG, CONFIG.timeout);
}

HAL_StatusTypeDef SST26_Status(uint8_t *STATUS_REG) {
#ifdef ENABLE_LOG
	log_info("SST26_Status");
#endif
	HAL_StatusTypeDef status;

	command.AddressMode = QSPI_ADDRESS_NONE;
	command.DataMode = mode_mask.DataModeMask;
	command.InstructionMode = mode_mask.InstructionModeMask;

	command.Instruction = RDSR;
	command.NbData = 1;

	switch (mode_mask.mode) {
	case MODE_SPI:
		command.DummyCycles = 0;
		break;
	case MODE_QSPI:
	default:
		command.DummyCycles = 2;
		break;
	}

	status = HAL_QSPI_Command(CONFIG.hqspi, &command, CONFIG.timeout);
	if (status != HAL_OK) {
		log_error("SST26_Status HAL_QSPI_Command error: 0x%02X", status);
		qspi_error();
		return status;
	}

	status = HAL_QSPI_Receive(CONFIG.hqspi, STATUS_REG, CONFIG.timeout);
	if(status != HAL_OK){
		log_error("SST26_Status HAL_QSPI_Receive error: 0x%02X", status);
		qspi_error();
	}
	return status;
}

struct tBusyResult {
	HAL_StatusTypeDef halStatus;
	bool busy;
};

static struct tBusyResult busy(){
	struct tBusyResult result;
	uint8_t sreg = 0;
	result.halStatus = SST26_Status(&sreg);
	result.busy = sreg & SST26_SREG_BUSY_MASK;
	return result;
}

static HAL_StatusTypeDef busyWait(uint32_t timeout){
	uint32_t timeoutAt = HAL_GetTick() + timeout;
	while(HAL_GetTick()<timeoutAt){
		struct tBusyResult b = busy();
		if(b.halStatus!=HAL_OK){
			log_error("SST26_EraseChip busy: 0x%02X", b.halStatus);
			return b.halStatus;
		}
		if(!b.busy){
			return HAL_OK;
		}
	}
	return HAL_TIMEOUT;
}

static HAL_StatusTypeDef initialize_configuration_register(){
#ifdef ENABLE_LOG
	log_info("initialize_configuration_register");
#endif
	while(1){
		struct tBusyResult b = busy();
		if(b.halStatus!=HAL_OK){
			log_error("SST26_EraseChip busy: 0x%02X", b.halStatus);
		}
		if(!b.busy){
			break;
		}
	}

	const uint8_t WR_PROTECT_DISABLE = 0x00;
	const uint8_t IOC_WP_HOLD_DISABLED = 0x02;
	HAL_StatusTypeDef status;
	if ((status = write_configuration_register(WR_PROTECT_DISABLE | IOC_WP_HOLD_DISABLED))!=HAL_OK){
		return status;
	}
	return status;
}

static HAL_StatusTypeDef globalUnlockBlockProtection(){
#ifdef ENABLE_LOG
	log_info("SST26_GlobalUnlockBlockProtection");
#endif
	HAL_StatusTypeDef status;

	if((status = busyWait(CONFIG.timeout))!=HAL_OK){
		return status;
	}

	if((status = write_enable())!=HAL_OK){
		return status;
	}
	if((status = global_unlock_block_protection())!=HAL_OK){
		return status;
	}
	return status;
}

HAL_StatusTypeDef SST26_ReadDeviceID(uint8_t *ID_REG){
#ifdef ENABLE_LOG
	log_info("SST26_ReadDeviceID");
#endif
	return spi_read_device_id(ID_REG);
}

HAL_StatusTypeDef SST26_Read(uint32_t address, uint32_t size, uint8_t *buffer) {
#ifdef ENABLE_LOG
	log_info("SST26_Read(0x%08X,%d,0x%08X)", address, size, (uint32_t)buffer);
#endif
	HAL_StatusTypeDef status;
	if((status = busyWait(CONFIG.timeout))!=HAL_OK){
		return status;
	}
	command.AddressMode = mode_mask.AddressModeMask;
	command.DataMode = mode_mask.DataModeMask;
	command.InstructionMode = mode_mask.InstructionModeMask;

	command.DummyCycles = 0;
	command.NbData = size;
	command.Address = address;

	switch (mode_mask.mode) {
	case MODE_SPI:
		command.Instruction = READ;
		command.DummyCycles = 0;
		break;
	case MODE_QSPI:
	default:
		command.Instruction = HSPI_RD;
		command.DummyCycles = 3 * 2;
		break;
	}

	status = HAL_QSPI_Command(CONFIG.hqspi, &command, CONFIG.timeout);
	if (status != HAL_OK) {
		log_error("SST26_Read HAL_QSPI_Command error: 0x%02X", status);
		qspi_error();
		return status;
	}

	status = HAL_QSPI_Receive(CONFIG.hqspi, buffer, CONFIG.timeout);
	if(status!=HAL_OK){
		log_error("SST26_Read HAL_QSPI_Receive error: 0x%02X", status);
		qspi_error();
	}
	return status;
}

HAL_StatusTypeDef SST26_EraseSector(uint32_t address){
#ifdef ENABLE_LOG
	log_info("SST26_EraseSector(%d)", address);
#endif
	HAL_StatusTypeDef status;
	if((status = busyWait(CONFIG.timeout))!=HAL_OK){
		return status;
	}
	if((status = write_enable()) != HAL_OK){
		return status;
	}

	if((status = sector_erase(address))!=HAL_OK){
		return status;
	}
	return HAL_OK;
}


HAL_StatusTypeDef SST26_EraseChip(){
#ifdef ENABLE_LOG
	log_info("SST26_EraseChip");
#endif
	HAL_StatusTypeDef status;
	if((status = busyWait(CONFIG.timeout))!=HAL_OK){
		return status;
	}
	if((status = write_enable()) != HAL_OK){
		return status;
	}

	if((status = chip_erase())!=HAL_OK){
		return status;
	}

	return HAL_OK;
}


HAL_StatusTypeDef SST26_Write(uint32_t address, uint32_t size, uint8_t *buffer){
#ifdef ENABLE_LOG
	log_info("SST26_Write(%d, %d, 0x%08X)", address, size, (uint32_t)buffer);
#endif
	HAL_StatusTypeDef status;
	if((status = busyWait(CONFIG.timeout))!=HAL_OK){
		return status;
	}
	status = write_enable();
	if(status != HAL_OK){
		return status;
	}
	status = page_program(address, size, buffer);
	return status;
}

HAL_StatusTypeDef SST26_SectorWrite(uint32_t sectorNumber, uint32_t sectorCount, uint8_t *buffer){
	HAL_StatusTypeDef status;
	for(uint32_t sector = sectorNumber; sector<sectorNumber+sectorCount; sector++){
		if((status = SST26_EraseSector(sector * SST26_SECTOR_SIZE))!=HAL_OK){
			return status;
		}
		for(uint32_t page = 0; page < SST26_SECTOR_SIZE/SST26_PAGE_SIZE; page++){
			SST26_Write(sector * SST26_SECTOR_SIZE + page * SST26_PAGE_SIZE, SST26_PAGE_SIZE, buffer);
			buffer+=SST26_PAGE_SIZE;
		}
	}
	return HAL_OK;
}

void SST26_MemoryTest(){
	HAL_StatusTypeDef status;

	uint64_t TEST_MAX = SST26_MEMORY_SIZE;
	uint8_t buffer[SST26_PAGE_SIZE];

	uint64_t t1 = HAL_GetTick();

	uint32_t *buffer32 = (uint32_t*)buffer;
	uint32_t value = 0;
	uint32_t sector = 0xFFFFFFFF;

	uint64_t i=0;

	log_info("Speed test");

	for(i=0; i<TEST_MAX; i+=SST26_PAGE_SIZE){
		for(uint64_t j=0; j<SST26_PAGE_SIZE/4; j++){
			buffer32[j] = value;
			value++;
		}
		if(i/4096 != sector){
			SST26_EraseSector(i);
			sector = i/4096;
		}
		status = SST26_Write(i, SST26_PAGE_SIZE, buffer);
		if (status!=HAL_OK){
			log_error("Write error: %d", status);
			return;
		}
	}

	uint64_t t2 = HAL_GetTick();
	log_info("Current speed: %llu kB/s", i/(t2 - t1) );
	log_info("Wrote memory in: %llu, %llu", (t2-t1), i);

	value = 0;

	for(uint64_t i=0; i<TEST_MAX; i+=SST26_PAGE_SIZE){
		status = SST26_Read(i, SST26_PAGE_SIZE, buffer);
		if (status!=HAL_OK){
			log_error("Read error: %d", status);
			return;
		}
		for(uint64_t j=0; j<SST26_PAGE_SIZE/4; j++){
			if(buffer32[j] != value){
				log_error("Incorrect value in memory at %llu: expected %llu, actual: %llu", i+j, value, buffer32[j]);
				return;
			}
			value++;
		}
	}
	log_info("Read correct");
}

static bool initialized = 0;

HAL_StatusTypeDef SST26_init() {
#ifdef ENABLE_LOG
	log_info("SST26_init");
#endif
	if (initialized){
		return HAL_OK;
	}
	initialized = true;
	HAL_StatusTypeDef status = full_reset();
	if (status != HAL_OK) {
		return status;
	}

	status = write_enable();
	if(status!=HAL_OK){
		log_error("Cannot enable write: 0x%02x", status);
		return status;
	}

	status = initialize_configuration_register();
	if(status!=HAL_OK){
		log_error("Cannot initialize configuration register: 0x%x", status);
		return status;
	}

	if((status = globalUnlockBlockProtection())!=HAL_OK){
		log_error("Cannot unlock global protection");
		return status;
	}

	uint8_t STATUS_REG = 0;
	uint8_t CONFIG_REG = 0;

	status = SST26_Status(&STATUS_REG);
	if (status != HAL_OK) {
		log_error("1. status register read error: 0x%02X", status);
		return status;
	}
	if (STATUS_REG != 0) {
		log_error("1. Status register is not zero: 0x%02X", STATUS_REG);
		return HAL_ERROR;
	}

	status = read_configuration_register(&CONFIG_REG);
	if (status != HAL_OK) {
		log_error("1. Config register read error: 0x%02X", status);
		return status;
	}
	if (CONFIG_REG != 0x0a) {
		log_error("1. Config register is not 0x0a: 0x%02X", CONFIG_REG);
		return HAL_ERROR;
	}


	status = enable_qspi();
	if (status != HAL_OK) {
		log_error("enable qspi error: %x", STATUS_REG);
		return status;
	}

	status = SST26_Status(&STATUS_REG);
	if (status != HAL_OK) {
		log_error("2. status register read error: %x", status);
		return status;
	}
	if (STATUS_REG != 0) {
		log_error("2. Status register is not zero: %x", STATUS_REG);
		return HAL_ERROR;
	}

	status = read_configuration_register(&CONFIG_REG);
	if (status != HAL_OK) {
		log_error("2. Config register read error: %x", status);
		return status;
	}
	if (CONFIG_REG != 0x0a) {
		log_error("2. Config register is not 0x08: %x", CONFIG_REG);
		return HAL_ERROR;
	}

	return HAL_OK;
}

