#pragma once

#include <stdint.h>

#include "pci.h"

#define	SATA_SIG_ATA	0x00000101	// SATA drive
#define	SATA_SIG_ATAPI	0xEB140101	// SATAPI drive
#define	SATA_SIG_SEMB	0xC33C0101	// Enclosure management bridge
#define	SATA_SIG_PM	    0x96690101	// Port multiplier

#define HBA_PORT_IPM_ACTIVE 1
#define HBA_PORT_DET_PRESENT 3

typedef enum {
    AHCI_DEVICE_TYPE_NULL = 0,
    AHCI_DEVICE_TYPE_SATA = 1,
    AHCI_DEVICE_TYPE_SEMB = 2,
    AHCI_DEVICE_TYPE_PM = 3,
    AHCI_DEVICE_TYPE_SATAPI = 4
} AHCI_DEVICE_TYPE;

// Following code defines different kinds of FIS specified in Serial ATA Revision 3.0.
typedef enum
{
	FIS_TYPE_REG_H2D	= 0x27,	// Register FIS - host to device
	FIS_TYPE_REG_D2H	= 0x34,	// Register FIS - device to host
	FIS_TYPE_DMA_ACT	= 0x39,	// DMA activate FIS - device to host
	FIS_TYPE_DMA_SETUP	= 0x41,	// DMA setup FIS - bidirectional
	FIS_TYPE_DATA		= 0x46,	// Data FIS - bidirectional
	FIS_TYPE_BIST		= 0x58,	// BIST activate FIS - bidirectional
	FIS_TYPE_PIO_SETUP	= 0x5F,	// PIO setup FIS - device to host
	FIS_TYPE_DEV_BITS	= 0xA1,	// Set device bits FIS - device to host
} FIS_TYPE;

// Register FIS - Host to Device
// A host to device register FIS is used by the host to send command or control to a device.
// It contains the IDE registers such as command, LBA, device, feature, count and control. 
// An ATA command is constructed in this structure and issued to the device. 
// All reserved fields in an FIS should be cleared to zero.
typedef struct {
    // DWORD 0
    uint8_t fis_type;       // FIS_TYPE_REG_H2D
    uint8_t pmport:4;       // port multiplier
    uint8_t rsv0:3;         // reserved
    uint8_t c:1;            // 1: command, 0: control
    uint8_t command;        // command register
    uint8_t featurel;       // feature register, 7:0
    
    // DWORD 1
    uint8_t lba0;           // LBA low register, 7:0
    uint8_t lba1;           // LBA mid register, 15:8
    uint8_t lba2;           // LBA high register, 23:16
    uint8_t device;         // device register

    // DWORD 2
    uint8_t lba3;           // LBA register, 31:24
    uint8_t lba4;           // LBA register, 39:32
    uint8_t lba5;           // LBA register 47:40
    uint8_t featureh;       // feature register, 15:8

    // DWORD 3
    uint8_t countl;         // count register, 7:0
    uint8_t counth;         // count register, 15:8
    uint8_t icc;            // isochronous command completion
    uint8_t control;        // control register

    // DWORD 4
    uint8_t resv1[4];       // reserved
} __attribute__((packed)) fis_reg_h2d_t;

// Register FIS – Device to Host
// A device to host register FIS is used by the device to notify the host that some ATA register has changed. 
// It contains the updated task files such as status, error and other registers.
typedef struct {
    // DWORD 0
    uint8_t fis_type;       // FIS_TYPE_REG_D2H
    uint8_t pmport:4;       // port multiplier
    uint8_t rsv0:2;         // reserved
    uint8_t i:1;            // interrupt bit
    uint8_t rsv1:1;         // reserved
    uint8_t status;         // status register
    uint8_t error;          // error register

    // DWORD 1
    uint8_t lba0;           // LBA low register, 7:0
    uint8_t lba1;           // LBA mid register, 15:8
    uint8_t lba2;           // LBA high register, 23:16
    uint8_t device;         // device register

    // DWORD 2
    uint8_t lba3;           // LBA register 31:24
    uint8_t lba4;           // LBA register, 39:32
    uint8_t lba5;           // LBA register, 47:40
    uint8_t rsv2;           // reserved

    // DWORD 3
    uint8_t countl;         // count register, 7:0
    uint8_t counth;         // count register, 15:8
    uint8_t rsv3[2];        // reserved

    // DWORD 4
    uint8_t rsv4[4];        // reserved
} __attribute__((packed)) fis_reg_d2h_t;

// Data FIS – Bidirectional
// This FIS is used by the host or device to send data payload. The data size can be varied.
typedef struct {
    // DWORD 0
    uint8_t fis_type;       // FIS_TYPE_DATA;
    uint8_t pmport:4;       // port multiplier
    uint8_t rsv0:4;         // reserved
    uint8_t rsv1[2];        // reserved
    
    // DWORD 1 ~ N
    uint32_t data[1];      // payload
} __attribute__((packed)) fis_data_t;

// PIO Setup – Device to Host
// This FIS is used by the device to tell the host that it’s about to send or ready to receive a PIO data payload.
typedef struct {
    // DWORD 0
    uint8_t fis_type;       // FIS_TYPE_PIO_SETUP
    uint8_t pmport:4;       // port multiplier
    uint8_t rsv0:1;         // reserved
    uint8_t d:1;            // data transfer direction, 1 - device to host
    uint8_t i:1;            // interrupt bit
    uint8_t rsv1:1;         // reserved
    uint8_t status;         // status register
    uint8_t error;          // error register

    // DWORD 1
    uint8_t lba0;           // LBA low register 7:0
    uint8_t lba1;           // LBA mid register 15:8
    uint8_t lba2;           // LBA high register 23:16
    uint8_t device;         // device register

    // DWORD 2
    uint8_t lba3;           // LBA register, 31:24
    uint8_t lba4;           // LBA register, 39:32
    uint8_t lba5;           // LBA register, 47:40
    uint8_t rsv2;           // reserved

    // DWORD 3
    uint8_t countl;         // count register, 7:0
    uint8_t counth;         // count register, 15:8
    uint8_t rsv3;           // reserved
    uint8_t e_status;       // new value of status register

    // DWORD 4
    uint16_t tc;            // transfer count
    uint8_t rsv4[2];        // reserved
} __attribute__((packed)) fis_pio_setup_t;

// DMA Setup – Device to Host
typedef struct {
    // DWORD 0
    uint8_t fis_type;       // FIS_TYPE_DMA_SETUP
    uint8_t pmport:4;       // port multiplier
    uint8_t rsv0:1;         // reserved
    uint8_t d:1;            // data transfer direction, 1 - device to host
    uint8_t i:1;            // interrupt bit
    uint8_t a:1;            // auto-activate. specifies if DMA Activate FIS is needed
    uint8_t rsv1[2];        // reserved

    // DWORD 1 ~ 2
    uint64_t dma_buff_id;   // DMA Buffer Identifier. 

    // DWORD 3
    uint32_t rsv2;          // reserved

    // DWORD 4
    uint32_t dma_buff_off;  // byte offset into buffer. first 2 bits must be 0

    // DWORD 5
    uint32_t xfer_cnt;      // number of bytes to transfer. Bit 0 must be 0

    // DWORD 6
    uint32_t rsv3;          // reserved
} __attribute__((packed)) fis_dma_setup_t;

typedef struct {
    uint8_t fis_type;       // FIS_TYPE_DEV_BITS
    uint8_t pmport:4;       // port multiplier
    uint8_t rsv0:2;         // reserved
    uint8_t i:1;            // interrupt bit
    uint8_t n:1;
    uint8_t statusl:3;
    uint8_t rsv1:1;         // reserved
    uint8_t statush:3;
    uint8_t rsv2:1;         // reserved
    uint8_t error;
} __attribute__((packed)) fis_dev_bits_t;

// Received FIS
// There are four kinds of FIS which may be sent to the host by the device. 
// When an FIS has been copied into the host specified memory, an according 
// bit will be set in the Port Interrupt Status register (HBA_PORT.is).
// Data FIS – Device to Host is not copied to this structure. 
// Data payload is sent and received through PRDT (Physical Region Descriptor Table) 
// in Command List, as will be introduced later.
typedef struct {
    // 0x00
    fis_dma_setup_t dsfis;  // DMA Setup FIS
    uint8_t pad0[4];

    // 0x20
    fis_pio_setup_t psfis;  // PIO Setup FIS
    uint8_t pad1[12];

    // 0x40
    fis_reg_d2h_t rfis;     // Register - Device to Host FIS
    uint8_t pad2[4];

    // 0x58
    fis_dev_bits_t sdbfis;  // Set Device Bit FIS

    // 0x60
    uint8_t ufis[64];

    // 0xA0
    uint8_t rsv[0x100-0xA0]; // reserved

} __attribute__((packed)) hba_fis_t;

typedef volatile struct {
    uint32_t clb;           // 0x00 - command list base address, 1K-byte aligned
    uint32_t clbu;          // 0x04 - command list base address upper 32 bits
    uint32_t fb;            // 0x08 - FIS base address, 256-byte aligned
    uint32_t fbu;           // 0x0C - FIS base address upper 32 bits
    uint32_t is;            // 0x10 - interrupt status
    uint32_t ie;            // 0x14 - interrupt enable
    uint32_t cmd;           // 0x18 - command and status
    uint32_t rsv0;          // 0x1C - Reserved
    uint32_t tfd;           // 0x20 - task file data
    uint32_t sig;           // 0x24 - signature
    uint32_t ssts;          // 0x28 - SATA status (SCR0:SStatus)
    uint32_t sctl;          // 0x2C - SATA control (SCR2:SControl)
    uint32_t serr;          // 0x30 - SATA error (SCR1:SError)
    uint32_t sact;          // 0x34 - SATA active (SCR3:SActive)
    uint32_t ci;            // 0x38 - command issue
    uint32_t sntf;          // 0x3C - SATA notification (SCR4:SNotification)
    uint32_t fbs;           // 0x40 - FIS-based switch control
    uint32_t rsv1[11];      // 0x44 ~ 0x6F - Reserved
    uint32_t vendor[4];     // 0x70 ~ 0x7F - vendor specific
} __attribute__((packed)) hba_port_t;

typedef volatile struct {
    // 0x00 - 0x2B :: Generic Host Control
    uint32_t cap;           // 0x00 - Host capability
    uint32_t ghc;           // 0x04 - Global host control
    uint32_t is;            // 0x08 - Interrupt Status
    uint32_t pi;            // 0x0C - Port Implemented
    uint32_t vsn;           // 0x10 - Version
    uint32_t ccc_ctl;       // 0x14 - Command Completion Coalescing Control
    uint32_t ccc_pts;       // 0x18 - Command Completion Coalescing Ports
    uint32_t em_loc;        // 0x1C - Enclosure management location
    uint32_t em_ctl;        // 0x20 - Enclosure management control
    uint32_t cap2;          // 0x24 - Host capabilities (extended)
    uint32_t bohc;          // 0x28 - BIOS/OS handoff control and status

    // 0x2C - 0x9F :: Reserved
    uint8_t reserved[0xA0-0x2C];

    // 0xA0 - 0xFF :: Vendor specific registers
    uint8_t vendor[0x100-0xA0];

    // 0x100 - 0x10FF :: Port control registers
    hba_port_t ports[1];        // 1 ~ 32
} __attribute__((packed)) hba_mem_t;

typedef struct {
    // DWORD 0
    uint8_t cfl:5;          // command FIS length in DWORDS, 2 ~ 16
    uint8_t a:1;            // ATAPI
    uint8_t w:1;            // Write, 1: H2D, 0: D2H
    uint8_t p:1;            // prefetchable
    uint8_t r:1;            // reset
    uint8_t b:1;            // BIST
    uint8_t c:1;            // Clear busy upon R_OK
    uint8_t rsv0:1;         // reserved
    uint8_t pmport:4;       // port multiplier
    uint16_t prdtl;         // Physical Region Descriptor Table Length (in entries)

    // DWORD 1
    volatile 
    uint32_t prdbc;         // physical region descriptor byte count transferred

    // DWORD 2 & 3
    uint32_t ctba;          // command table descriptor base address
    uint32_t ctbau;         // command table descriptor base address upper 32-bits

    // DWORD 4 ~ 7
    uint32_t rsv1[4];       // reserved

} __attribute__((packed)) hba_cmd_header_t;

typedef struct {
    uint32_t dba;           // data base address
    uint32_t dbau;          // data base address upper 32-bits
    uint32_t rsv0;          // reserved
    uint32_t dbc:22;        // byte count, 4M max
    uint32_t rsv1:9;        // reserved
    uint32_t i:1;           // interrupt on completion
} __attribute__((packed)) hba_prdt_entry_t;

typedef struct {
    // 0x00
    uint8_t cfis[64];       // command FIS

    // 0x40
    uint8_t acmd[16];       // ATAPI command, 12 or 16 bytes

    // 0x50
    uint8_t rsv[48];        // reserved

    // 0x80
    hba_prdt_entry_t prdt_entry[1]; // physical region descriptor table entries, 0 ~ 65535
} __attribute__((packed)) hba_cmd_tbl_t;

typedef struct {
    pci_device_hdr_t *pci_base_address;
    hba_mem_t *abar;
} ahci_driver_t;

void ahci_init(ahci_driver_t *driver, pci_device_hdr_t *pci_base_address);