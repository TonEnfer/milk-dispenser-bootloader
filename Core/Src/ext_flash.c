#include "ext_flash.h"
#include "log.h"

#define CONFIG SST26_config

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


#define ENABLE_LOG


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

static HAL_StatusTypeDef initialize_configuration_register(){
#ifdef ENABLE_LOG
	log_info("initialize_configuration_register");
#endif
	const uint8_t WR_PROTECT_DISABLE = 0x00;
	const uint8_t IOC_WP_HOLD_DISABLED = 0x02;
	HAL_StatusTypeDef status;
	if ((status = write_configuration_register(WR_PROTECT_DISABLE | IOC_WP_HOLD_DISABLED))!=HAL_OK){
		return status;
	}

	uint8_t sreg;
	if((status = SST26_Status(&sreg))!=HAL_OK){
		return status;
	}

	if (sreg & WRITE_ENABLE_STATUS_MASK){
		log_error("Invalid status: WE is set");
		return HAL_ERROR;
	}

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
		return status;
	}

	status = HAL_QSPI_Transmit(CONFIG.hqspi, buffer, CONFIG.timeout);
	if(status!= HAL_OK){
		return status;
	}

	HAL_Delay(100);
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
		return status;
	}

	HAL_Delay(100);
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
		return status;
	}

	return HAL_QSPI_Receive(CONFIG.hqspi, STATUS_REG, CONFIG.timeout);
}

static HAL_StatusTypeDef globalUnlockBlockProtection(){
#ifdef ENABLE_LOG
	log_info("SST26_GlobalUnlockBlockProtection");
#endif
	HAL_StatusTypeDef status;
	if((status = write_enable())!=HAL_OK){
		return status;
	}
	if((status = global_unlock_block_protection())!=HAL_OK){
		return status;
	}

	uint8_t sreg;
	if((status = SST26_Status(&sreg))!=HAL_OK){
		return status;
	}

	if (sreg & WRITE_ENABLE_STATUS_MASK){
		log_error("Invalid status: WE is set");
		return HAL_ERROR;
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
	log_info("SST26_Read %d %d %d", address, size, (uint32_t)buffer);
#endif
	HAL_StatusTypeDef status;

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
		return status;
	}

	return HAL_QSPI_Receive(CONFIG.hqspi, buffer, CONFIG.timeout);
}

HAL_StatusTypeDef SST26_EraseSector(uint32_t address){
#ifdef ENABLE_LOG
	log_info("SST26_EraseSector %d", address);
#endif
	HAL_StatusTypeDef status;
	if((status = write_enable()) != HAL_OK){
		return status;
	}

	if((status = sector_erase(address))!=HAL_OK){
		return status;
	}

	uint8_t sreg;
	if((status = SST26_Status(&sreg))!=HAL_OK){
		return status;
	}

	if (sreg & WRITE_ENABLE_STATUS_MASK){
		log_error("Invalid status: WE is set");
		return HAL_ERROR;
	}

	return HAL_OK;
}

static HAL_StatusTypeDef SST26_Write_Dump(uint32_t address, uint32_t size, uint8_t *buffer){
#ifdef ENABLE_LOG
	log_info("SST26_Write_Dump(%d, %d, 0x%08X)", address, size, (uint32_t)buffer);
#endif
	HAL_StatusTypeDef status;
	status = write_enable();
	if(status != HAL_OK){
		return status;
	}
	status = page_program(address, size, buffer);
	return status;
}

HAL_StatusTypeDef SST26_Write(uint32_t address, uint32_t size, uint8_t *buffer){
#ifdef ENABLE_LOG
	log_info("SST26_Write_Dump(%d, %d, 0x%08X)", address, size, (uint32_t)buffer);
#endif
	HAL_StatusTypeDef status;
	// Store in memory
	// Flush memory if write to another block
	// Split flushes into 256byte pages
	// Read new flash sector into memory
	// Store in memory
	return HAL_OK;
}

HAL_StatusTypeDef SST26_init() {
#ifdef ENABLE_LOG
	log_info("SST26_init");
#endif
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

