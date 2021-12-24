#include "ahci.h"
#include "pagetable_manager.h"
#include "pageframe_allocator.h"
#include "globals.h"
#include "heap.h"
#include "memory.h"

#include "string.h"

#define MAX_READ_SPIN           1000000
#define ATA_DEV_BUSY            0x80
#define ATA_DEV_DRQ             0x08
#define ATA_CMD_READ_DMA_EX     0X25
#define	SATA_SIG_ATA	        0x00000101	/* SATA drive */
#define	SATA_SIG_ATAPI	        0xEB140101	/* SATAPI drive */
#define	SATA_SIG_SEMB	        0xC33C0101	/* Enclosure management bridge */
#define	SATA_SIG_PM	            0x96690101	/* Port multiplier */
#define HBA_PORT_IPM_ACTIVE     1
#define HBA_PORT_DET_PRESENT    3
#define HBA_PxCMD_ST            0x0001
#define HBA_PxCMD_FRE           0x0010
#define HBA_PxCMD_FR            0x4000
#define HBA_PxCMD_CR            0x8000
#define HBA_PxIS_TFES           (1 << 30)

static void __probe_ports(hba_mem_t *);
static AHCI_DEVICE_TYPE __check_device_type(hba_port_t *);
static void __configure_port(hba_port_t *);
static void __stop_cmd(hba_port_t *);
static void __start_cmd(hba_port_t *);

static ahci_port_t *_ports[32];
static uint8_t _port_count = 0;

char buf[128];

void ahci_init(ahci_driver_t *driver, pci_device_hdr_t *pci_base_address)
{
    driver->pci_base_address = pci_base_address;
    driver->abar = (hba_mem_t *)((uint64_t)((pci_general_device_t *)pci_base_address)->base_address5);
    pagetable_map(g_pml4, (void *)(driver->abar), (void *)(driver->abar));

    __probe_ports(driver->abar);

    for (int i = 0; i < _port_count; i++) {
        ahci_port_t *port = _ports[i];
        __configure_port(port->hba_port);

        port->buffer = (uint8_t *)pageframe_request();
        memzero(port->buffer, PAGE_SIZE);

        ahci_read(port, 0, 4, port->buffer);
        
        for (int i = 0; i < 1024; i++) {
            tty_putc(g_tty, port->buffer[i]);
        }
        tty_newline(g_tty);
    }
}

bool ahci_read(ahci_port_t *port, uint64_t sector, uint32_t sector_count, void *buffer)
{
    uint32_t sectorl = (uint32_t)sector;
    uint32_t sectorh = (uint32_t)(sector >> 32);

    port->hba_port->is = (uint32_t)-1; // clear pending int bits
    hba_cmd_header_t *cmd = (hba_cmd_header_t *)((uint64_t)port->hba_port->clb);
    cmd->cfl = sizeof(FIS_TYPE_REG_H2D)/sizeof(uint32_t); // command FIS size
    cmd->w = 0; // read from device
    cmd->prdtl = 1;

    hba_cmd_tbl_t *cmdtbl = (hba_cmd_tbl_t *)((uint64_t)cmd->ctba);
    memset((void *)cmdtbl, 0, sizeof(hba_cmd_tbl_t) + (cmd->prdtl-1) * sizeof(hba_prdt_entry_t));

    cmdtbl->prdt_entry[0].dba = (uint32_t)(uint64_t)buffer;
    cmdtbl->prdt_entry[0].dbau = (uint32_t)((uint64_t)buffer >> 32);
    cmdtbl->prdt_entry[0].dbc = (sector_count << 9) - 1; // 512 bytes per sector
    cmdtbl->prdt_entry[0].i = 1;

    fis_reg_h2d_t *fis = (fis_reg_h2d_t *)(&cmdtbl->cfis);
    fis->fis_type = FIS_TYPE_REG_H2D;
    fis->c = 1; // command
    fis->command = ATA_CMD_READ_DMA_EX;
    fis->lba0 = (uint8_t)sectorl;
    fis->lba1 = (uint8_t)(sectorl >> 8);
    fis->lba2 = (uint8_t)(sectorl >> 16);
    fis->lba3 = (uint8_t)sectorh;
    fis->lba4 = (uint8_t)(sectorh >> 8);
    fis->lba5 = (uint8_t)(sectorh >> 16);
    fis->device = 1 << 6;   // LBA mode
    fis->countl = sector_count & 0xFF;
    fis->counth = (sector_count >> 8) & 0xFF;

    // The below loop waits until the port is no longer busy before issuing a new command
    uint64_t spin = 0; // spin lock timeout counter
    while ((port->hba_port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < MAX_READ_SPIN) {
        spin++;
    }

    if (spin == MAX_READ_SPIN) {
        return false;
    }

    port->hba_port->ci = 1; // issue command

    // wait for completion
    while (true) {
        if (port->hba_port->ci == 0) break;
        if (port->hba_port->is & HBA_PxIS_TFES) { 
            // read unsuccessful
            return false;
        }
    }

    return true;
}

static void __probe_ports(hba_mem_t *abar)
{
    uint32_t pi = abar->pi;
    for (int i = 0; i < 32; i++, pi >> 1) {
        if (pi & (1 << i)) {
            int dt = __check_device_type(&abar->ports[i]);

            if (dt == AHCI_DEVICE_TYPE_SATA || dt == AHCI_DEVICE_TYPE_SATAPI) {
                ahci_port_t *port = (ahci_port_t *)heap_alloc(sizeof(ahci_port_t));
                port->type = dt;
                port->hba_port = &abar->ports[i];
                port->portnum = _port_count;
                _ports[_port_count++] = port;
            }            
        }
    }
}

static AHCI_DEVICE_TYPE __check_device_type(hba_port_t *port)
{
    uint32_t ssts = port->ssts;

    uint8_t ipm = (ssts >> 8) & 0x0F;
    uint8_t det = ssts & 0x0F;

    if (det != HBA_PORT_DET_PRESENT) return AHCI_DEVICE_TYPE_NULL;
    if (ipm != HBA_PORT_IPM_ACTIVE) return AHCI_DEVICE_TYPE_NULL;

    switch(port->sig) {
        case SATA_SIG_ATAPI:
            return AHCI_DEVICE_TYPE_SATAPI;
        case SATA_SIG_SEMB:
            return AHCI_DEVICE_TYPE_SEMB;
        case SATA_SIG_PM:
            return AHCI_DEVICE_TYPE_PM;
        default:
            return AHCI_DEVICE_TYPE_SATA;
    }
}

static void __configure_port(hba_port_t *port)
{
    __stop_cmd(port);

    //todo: improve memory efficiency (ref: https://wiki.osdev.org/AHCI)
    uint64_t ahci_base = (uint64_t)pageframe_request();
    port->clb = (uint32_t)ahci_base;
    port->clbu = (uint32_t)(ahci_base >> 32);
    memset((void *)ahci_base, 0, 1024);

    uint64_t fis_base = (uint64_t)pageframe_request();
    port->fb = (uint32_t)fis_base;
    port->fbu = (uint32_t)(fis_base >> 32);
    memset((void *)fis_base, 0, 256);

    hba_cmd_header_t *cmd = (hba_cmd_header_t *)((uint64_t)port->clb + ((uint64_t)port->clbu << 32));
    for (int i = 0; i < 32; i++) {
        // 8 prdt entries per command table, 256b per command table, 64+16+48+16*8
        cmd[i].prdtl = 8;

        uint64_t cmdtbl_addr = (uint64_t)pageframe_request() + (i << 8);
        cmd[i].ctba = (uint32_t)cmdtbl_addr;
        cmd[i].ctbau = (uint32_t)((uint64_t)cmdtbl_addr >> 32);
        memset((void *)cmdtbl_addr, 0, 256);
    }

    __start_cmd(port);
}

static void __start_cmd(hba_port_t *port)
{
    // wait until CR (bit 15) is cleared
    while (port->cmd & HBA_PxCMD_CR)
        ;
    
    // set FRE (bit 4) and ST (bit 0)
    port->cmd |= HBA_PxCMD_FRE;
    port->cmd |= HBA_PxCMD_ST;
}

static void __stop_cmd(hba_port_t *port)
{
    // clear ST (bit 0)
    port->cmd &= ~HBA_PxCMD_ST;

    // clear FRE (bit 4)
    port->cmd &= ~HBA_PxCMD_FRE;

    // wait until FR (bit 14), CR (bit 15) are cleared
    while (true) {
        if (port->cmd & HBA_PxCMD_FR) continue;
        if (port->cmd & HBA_PxCMD_CR) continue;
        break;
    }
}