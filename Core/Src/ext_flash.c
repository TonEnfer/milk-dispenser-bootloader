#include "ext_flash.h"

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

//static uint32_t flash_sector_addr = 0;
//static uint8_t flash_sector_mem[4096];

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
	mode_mask.AddressModeMask = QSPI_ADDRESS_1_LINE;
	mode_mask.DataModeMask = QSPI_DATA_1_LINE;
	mode_mask.InstructionModeMask = QSPI_INSTRUCTION_1_LINE;
	mode_mask.mode = MODE_SPI;
}

static void set_4_line_mode_mask() {
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
	HAL_StatusTypeDef status;

	if ((status = qspi_reset()) != HAL_OK) {
		return status;
	}
	return spi_reset();
}

static HAL_StatusTypeDef read_status_register(uint8_t *STATUS_REG) {
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
		command.DummyCycles = 2;
		break;
	}

	status = HAL_QSPI_Command(CONFIG.hqspi, &command, CONFIG.timeout);
	if (status != HAL_OK) {
		return status;
	}

	return HAL_QSPI_Receive(CONFIG.hqspi, STATUS_REG, CONFIG.timeout);
}

static HAL_StatusTypeDef read_configuration_register(uint8_t *STATUS_REG) {
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
		command.DummyCycles = 2;
		break;
	}

	status = HAL_QSPI_Command(CONFIG.hqspi, &command, CONFIG.timeout);
	if (status != HAL_OK) {
		return status;
	}

	return HAL_QSPI_Receive(CONFIG.hqspi, STATUS_REG, CONFIG.timeout);
}

static HAL_StatusTypeDef spi_read_device_id(uint8_t *ID_REG) {
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

HAL_StatusTypeDef SST26_ReadDeviceID(uint8_t *ID_REG){
	return spi_read_device_id(ID_REG);
}

HAL_StatusTypeDef SST26_Read(uint32_t address, uint32_t size, uint8_t *buffer) {
	HAL_StatusTypeDef status;

	command.AddressMode = mode_mask.AddressModeMask;
	command.DataMode = mode_mask.DataModeMask;
	command.InstructionMode = mode_mask.InstructionModeMask;

	command.DummyCycles = 0;
	command.NbData = size;

	switch (mode_mask.mode) {
	case MODE_SPI:
		command.Instruction = READ;
		command.DummyCycles = 0;
		break;
	case MODE_QSPI:
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

HAL_StatusTypeDef SST26_init() {
	HAL_StatusTypeDef status = full_reset();
	if (status != HAL_OK) {
		return status;
	}

	uint8_t STATUS_REG = 0;
	uint8_t CONFIG_REG = 0;

	status = read_status_register(&STATUS_REG);
	if (status != HAL_OK) {
		return status;
	}
	if (STATUS_REG != 0) {
		return HAL_ERROR;
	}

	status = read_configuration_register(&CONFIG_REG);
	if (status != HAL_OK) {
		return status;
	}
	if (CONFIG_REG != 0x08) {
		return HAL_ERROR;
	}

	status = enable_qspi();
	if (status != HAL_OK) {
		return status;
	}

	status = read_status_register(&STATUS_REG);
	if (status != HAL_OK) {
		return status;
	}
	if (STATUS_REG != 0) {
		return HAL_ERROR;
	}

	status = read_configuration_register(&CONFIG_REG);
	if (status != HAL_OK) {
		return status;
	}
	if (CONFIG_REG != 0x08) {
		return HAL_ERROR;
	}

	return HAL_OK;
}

