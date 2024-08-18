#define pr_fmt(fmt) KBUILD_BASENAME ": " fmt

#include <common.h>
#include <sysreg_base.h>

#include <asm/sysreg.h>
#include <asm/system.h>
#include <asm/cputype.h>

#include <public.h>
#include "irq-gic-common.h"
#include "arm-gic-v3.h"

#include "gicv3_its.h"

extern struct irq_gic_v3 gic_data;

GICv3_its *gits = (GICv3_its *)(GICV3_DIST_BASE + SOC_GIC_ITS0_OFFSET);
GICv3_its *gits0 = (GICv3_its *)(GICV3_DIST_BASE + SOC_GIC_ITS0_OFFSET);
GICv3_its *gits1 = (GICv3_its *)(GICV3_DIST_BASE + SOC_GIC_ITS1_OFFSET);


/**********************************************************************/

void init_ITS_command_queue(uint64_t address, uint64_t attributes, uint64_t num_pages) 
{
    uint64_t tmp;

    address = address & GENMASK_ULL(51, 12);
    attributes = attributes & (uint64_t)0xF800000000000C00;
    num_pages = num_pages & GENMASK_ULL(7, 0);

    // check number of pages is not 0
    if (num_pages == 0) {
        return;  // error
    }

    // TODO: Add check for sane number of pages

    // work out queue size in bytes, then zero memory
    // This code assumes that VA=PA for allocated memory
//    tmp = num_pages * 0x1000;
//  memset((void*)address, 0, tmp);

    // Combine address, attributes and size
    tmp = address | attributes | (num_pages - 1);
    gits->its_cbaser = tmp;

    /*
     * This register contains the offset from the start, hence setting to 0.
     */
    gits->its_cwriter = 0;

    /*
     * TODO: add a barrier here
     */
}

/*
 * Adds a command to the currently selected ITS's queue.
 * Inputs:
 *  command - pointer to 32 bytes of memory containing a ITS command
 * Returns:
 *  <nothing>
 * NOTE:
 *  This function is intended for internal use only.
 */
static void add_ITS_command(its_cmd_block *command) 
{
    uint32_t qsize, i;
    uint64_t new_cwriter, qbase, qwriter, qreader;
    uint64_t *entry;

    /*
     * its_cbaser.Size returns the number of 4K pages, minus -1.
     */
    qsize = ((gits->its_cbaser & GENMASK_ULL(7, 0)) + 1) * 0x1000;
    qbase = gits->its_cbaser & GENMASK_ULL(51, 12);
    /*
     * its_cwriter contains the offset.
     */
    qwriter = gits->its_cwriter;

    do {
        qreader = gits->its_creadr;

        /*
         * The queue is full when its_cwriter points at the entry before its_creadr.
         * To make the check simpler I've moved the its_creadr down one entry.
         */
        if (qreader == 0) {
            qreader = qsize - COMMAND_SIZE;
        } else {
            qreader = qreader - COMMAND_SIZE;
        }
        /*
         * In practice it should very rarely be full.
         */
    } while (qwriter == qreader);

    /*
     * Copy command into queue
     */
    entry = (uint64_t *)(qbase + qwriter);
    for (i = 0; i < 4; i++) {
        entry[i] = command->raw_cmd64[i];
    }
    dsb();

    /*
     * Check for roll-over
     */
    new_cwriter = qwriter + COMMAND_SIZE;
    if (new_cwriter == qsize) {
        new_cwriter = 0;
    }

    /*
     * Update its_cwriter, which also tells the ITS there is a new command.
     */
    gits->its_cwriter = new_cwriter;
}

/*
 * Issues a INT command to the currently selected ITS
 * Inputs:
 *  device_id   - specifies the requesting device.
 *  event_id    - identifies an interrupt source associated with a device.
 *                The ITS then translates this into an LPI INTID.
 * Returns:
 *  <nothing>
 * NOTE:
 *  This command translates the event defined by EventID and DeviceID into an ICID and pINTID,
 *  and instructs the appropriate Redistributor to set the interrupt pending.
 */
void add_ITS_INT(uint32_t device_id, uint32_t event_id) 
{
    its_cmd_block cmd;

    cmd.raw_cmd32[0] = GITS_CMD_INT;
    cmd.raw_cmd32[1] = device_id;
    cmd.raw_cmd32[2] = event_id;
    cmd.raw_cmd32[3] = 0x0;
    cmd.raw_cmd64[2] = 0x0;
    cmd.raw_cmd64[3] = 0x0;
    add_ITS_command(&cmd);
}

/*
 * Issues a INV command to the currently selected ITS
 * Inputs:
 *  device_id   - specifies the requesting device.
 *  event_id    - identifies an interrupt source associated with a device.
 *                The ITS then translates this into an LPI INTID.
 * Returns:
 *  <nothing>
 * NOTE:
 *  This command specifies that the ITS must ensure that any caching in
 *  the Redistributors associated with the specified
 *  EventID is consistent with the LPI Configuration tables held in memory.
 */
void add_ITS_INV(uint32_t device_id, uint32_t event_id) 
{
    its_cmd_block cmd;

    cmd.raw_cmd32[0] = GITS_CMD_INV;
    cmd.raw_cmd32[1] = device_id;
    cmd.raw_cmd32[2] = event_id;
    cmd.raw_cmd32[3] = 0x0;
    cmd.raw_cmd64[2] = 0x0;
    cmd.raw_cmd64[3] = 0x0;
    add_ITS_command(&cmd);
}

/*
 * Issues a INVALL command to the currently selected ITS
 * Inputs:
 *  icid - specifies the interrupt collection.
 * Returns:
 *  <nothing>
 * NOTE:
 *  This command specifies that the ITS must ensure any caching associated with
 *  the interrupt collection defined by ICID is consistent with the LPI Configuration
 *  tables held in memory for all Redistributors.
 */
void add_ITS_INVALL(uint32_t icid) 
{
    its_cmd_block cmd;

    cmd.raw_cmd64[0] = GITS_CMD_INVALL;
    cmd.raw_cmd64[1] = 0x0;
    cmd.raw_cmd64[2] = icid & GENMASK_ULL(15, 0);
    cmd.raw_cmd64[3] = 0x0;
    add_ITS_command(&cmd);
}

/*
 * Issues a MAPC command to the currently selected ITS
 * Inputs:
 *  rdbase  - specifies the target Redistributor to which interrupts in the collection are forwarded.
 *            Either the PA (64KB aligned) or the PE ID, depending on its_typer.PTA.
 *  icid    - specifies the interrupt collection that is to be mapped.
 * Returns:
 *  <nothing>
 * NOTE:
 *  This command maps the Collection table entry defined by ICID to
 *  the target Redistributor, defined by RDbase.
 */
void add_ITS_MAPC(uint64_t rdbase, uint32_t icid) 
{
    its_cmd_block cmd;

    cmd.raw_cmd64[0] = GITS_CMD_MAPC;
    cmd.raw_cmd64[1] = 0x0;
    cmd.raw_cmd64[2] = (1ull << 63) | (rdbase & GENMASK_ULL(50, 16)) | (icid & GENMASK_ULL(15, 0));
    cmd.raw_cmd64[3] = 0x0;
    add_ITS_command(&cmd);
}

/*
 * Issues a MAPD command to the currently selected ITS
 * Inputs:
 *  device_id - specifies the device that uses the ITT.
 *  itt_addr - specifies bits[51:8] of the physical address (must be in Non-Secure PA space, 256B aligned) of the ITT.
 *  size - is a 5-bit number that specifies the supported number of bits for the device, minus one.
 *         The size field enables range checking of EventID for translation requests for this DeviceID.
 * Returns:
 *  <nothing>
 * NOTE:
 *  This command maps the Device table entry associated with DeviceID to its associated ITT,
 *  defined by ITT_addr and Size.
 *
 *  The command records the size as (number - 1).
 *  This functions expects an modified value (i.e. pass 1, if 2 bits).
 */
void add_ITS_MAPD(uint32_t device_id, uint64_t itt_addr, uint32_t size) 
{
    its_cmd_block cmd;

    cmd.raw_cmd32[0] = GITS_CMD_MAPD;
    cmd.raw_cmd32[1] = device_id;
    cmd.raw_cmd64[1] = size & GENMASK_ULL(4, 0);
    cmd.raw_cmd64[2] = (1ull << 63) | (itt_addr & GENMASK_ULL(51, 8));
    cmd.raw_cmd64[3] = 0x0;
    add_ITS_command(&cmd);
}

/*
 * Issues a MAPI command to the currently selected ITS
 * Inputs:
 *  device_id - specifies the requesting device.
 *  event_id - identifies the interrupt, associated with a device, that is to be mapped.
 *  icid - specifies the interrupt collection that includes the specified interrupt.
 * Returns:
 *  <nothing>
 * NOTE:
 *  This command maps the event defined by EventID and DeviceID into
 *  an ITT entry with ICID and pINTID = EventID.
 */
void add_ITS_MAPI(uint32_t device_id, uint32_t event_id, uint32_t icid) 
{
    its_cmd_block cmd;

    cmd.raw_cmd32[0] = GITS_CMD_MAPI;
    cmd.raw_cmd32[1] = device_id;
    cmd.raw_cmd32[2] = event_id;
    cmd.raw_cmd32[3] = 0x0;
    cmd.raw_cmd64[2] = icid & GENMASK_ULL(15, 0);
    cmd.raw_cmd64[3] = 0x0;
    add_ITS_command(&cmd);
}

/*
 * Issues a MAPTI command to the currently selected ITS
 * Inputs:
 *  device_id - specifies the requesting device.
 *  event_id - identifies the interrupt, associated with a device, that is to be mapped.
 *  intid - is the INTID of the physical interrupt that is presented to software.
 *  icid - specifies the interrupt collection that includes the specified physical interrupt.
 * Returns:
 *  <nothing>
 * NOTE:
 *  This command maps the event defined by EventID and DeviceID to its associated ITE,
 *  defined by ICID and pINTID in the ITT associated with DeviceID.
 */
void Add_ITS_MAPTI(uint32_t device_id, uint32_t event_id, uint32_t intid, uint32_t icid) 
{
    its_cmd_block cmd;

    cmd.raw_cmd32[0] = GITS_CMD_MAPTI;
    cmd.raw_cmd32[1] = device_id;
    cmd.raw_cmd32[2] = event_id;
    cmd.raw_cmd32[3] = intid;
    cmd.raw_cmd64[2] = icid & GENMASK_ULL(15, 0);
    cmd.raw_cmd64[3] = 0x0;
    add_ITS_command(&cmd);
}

/*
 * Issues a SYNC command to the currently selected ITS
 * Inputs:
 *  rdbase  - specifies the physical address of the target Redistributor.
 *            The format of the target address is determined by its_typer.PTA.
 * Returns:
 *  <nothing>
 * NOTE:
 *  This command ensures all outstanding ITS operations associated with physical interrupts for the Redistributor
 *  specified by RDbase are globally observed before any further ITS commands are executed. Following the execution
 *  of a SYNC, the effects of all previous commands must apply to subsequent writes to GITS_TRANSLATER.
 */
void add_ITS_SYNC(uint64_t rdbase) 
{
    its_cmd_block cmd;

    cmd.raw_cmd64[0] = GITS_CMD_SYNC;
    cmd.raw_cmd64[1] = 0x0;
    cmd.raw_cmd64[2] = rdbase & GENMASK_ULL(50, 16);
    cmd.raw_cmd64[3] = 0x0;
    add_ITS_command(&cmd);
}

// Returns the type and entry size of its_baser[index] of the currently selected ITS
// index = which its_basern register to access, must be in the range 0 to 7
// type  = pointer to a uint32_t, function write the vale of the its_basern.Type to this pointer
// entry_size  = pointer to a uint32_t, function write the vale of the its_basern.Entry_Size to this pointer
uint64_t get_ITS_table_type(uint32_t index, uint32_t* type, uint32_t* entry_size) 
{
    // Range check (there are only 8 (0-7) registers)
    if (index > 7)
        return 0;  // error

    *type       = (uint32_t)(0x7  & (gits->its_baser[index] >> 56));
    *entry_size = (uint32_t)(0x1F & (gits->its_baser[index] >> 48));

    return 1;
}

// Sets GITS_BASER[entry]
// index = which GITS_BASERn register to access, must be in the range 0 to 7
// addr  = phsyical address of allocated memory.  Must be at least 4K aligned.
// attributes = Cacheability, shareabilty, validity and indirect settings
// page_size  = Size of allocated pages (4K=0x0, 16K=0x100, 64K=0x200)
// num_pages  = The number of allocated pages.  Must be greater than 0.
//
// NOTE: The registers encodes as (number - 1), this function expecst then
// actual number
void set_ITS_table_addr(uint32_t index, uint64_t addr, 
	uint64_t attributes, uint32_t page_size, uint32_t num_pages) 
{
    uint64_t tmp;

    // Range check:
    // - there are only 8 (0-7) registers
    // - code doesn't allow for 0 pages (minimum that can be encoded into the register is 1)
    if ((index > 7) || (num_pages == 0))
        return;  // error

    // Combine fields to form entery
    tmp = (num_pages - 1) & 0xFF;
    tmp = tmp | ((page_size << 8)  & 0x300);
    tmp = tmp | (addr       & (uint64_t)0x0000FFFFFFFFF000);
    tmp = tmp | (attributes & (uint64_t)0xF800000000000C00);

    gits->its_baser[index] = tmp;

    // Zero memory
    // work out queue size in bytes, then zero memory
    // This code assumes that VA=PA for allocated memory
    if (page_size == GICV3_ITS_TABLE_PAGE_SIZE_4K)
        tmp = 0x1000 * num_pages;
    else if (page_size == GICV3_ITS_TABLE_PAGE_SIZE_16K)
        tmp = 0x4000 * num_pages;
    else if (page_size == GICV3_ITS_TABLE_PAGE_SIZE_64K)
        tmp = 0x10000 * num_pages;

}

uint32_t get_ITS_PTA(void) 
{
    return ((gits->its_typer >> 19) & 0x1);
}

void enable_ITS(void) 
{
    gits->its_ctlr = 1;
}

void disable_ITS(void) 
{
    gits->its_ctlr = 0;
}

// Set address of LPI config table for the currently selected Redistributor
// addr       = physical address of allocated memory
// attributes = cacheability/shareabilty settings
// IDbits     = number of ID bits
//
// NOTE: If IDbits > GICD_TYPER.IDbits, GICD_CTLR.IDbits will be used
// NOTE: The amount of memory allocated must be enough for the number of IDbits!

void set_LPI_config_table_addr(void *gicr_rdbase, uint64_t addr, 
	uint64_t attributes, uint32_t IDbits) 
{
    uint64_t tmp = 0;

    // The function takes the number of ID bits, the register expects (ID bits - 1)
    // If the number of IDbits is less than 14, then there are no LPIs
    // Also the maximum size of the EventID is 32 bit
    if ((IDbits < 14) || (IDbits > 32))
        return;  // error

    tmp = (1 << IDbits) - 8192;
    IDbits = IDbits - 1;

    // Write GICR_PROPBASER
    addr = addr & 0x0000FFFFFFFFF000; // PA is bits 47:12
    tmp = addr | (attributes & 0x0700000000000F80); // Attributes are in bits 58:56 and 11:7
    tmp = tmp | (IDbits & 0x000000000000001F); // IDBits is bits 4:0

	writeq(tmp, gicr_rdbase + GICR_PROPBASER);
}


// Set address of the LPI pending table for the currently selected Redistributor
// addr       = physical address of allocated memory
// attributes = cacheability/shareabilty settings
// IDbits     = number of ID bits (needed in order to work how much memory to zero)
//
// NOTE: The amount of memory allocated must be enough for the number of IDbits!
void set_LPI_pending_tableAddr(void *gicr_rdbase, uint64_t addr, 
	uint64_t attributes, uint32_t IDbits) 
{
    uint64_t tmp = 0;

    // The function takes the number of ID bits, the register expects (ID bits - 1)
    // If the number of IDbits is less than 14, then there are no LPIs
    if ((IDbits < 14) || (IDbits > 32))
        return;  // error

    // Zero table
    // This is ensure that all interrupts have a known (inactive) initial state
    // TBD: Fix memset to only clear required bytes!
    tmp = 1 << IDbits;

    // Write GICR_PENDBASER
    addr =         addr       & 0x0000FFFFFFFF0000;    // PA is bits 47:16
    tmp  = addr | (attributes & 0x0700000000000F80);   // Attributes are in bits 58:56 and 11:7
    tmp |= 1ull << 62;
	writeq(tmp, gicr_rdbase + GICR_PENDBASER);
}

void enable_LPIs(void *gicr_rdbase) 
{
	u32 gicr_ctlr_reg = readl(gicr_rdbase + GICR_CTLR);
	writel(gicr_ctlr_reg | 0x1, gicr_rdbase + GICR_CTLR);
}


void disable_LPIs(void *gicr_rdbase) 
{
	u32 gicr_ctlr_reg = readl(gicr_rdbase + GICR_CTLR);

	writel(gicr_ctlr_reg & ~0x1, gicr_rdbase + GICR_CTLR);
}
