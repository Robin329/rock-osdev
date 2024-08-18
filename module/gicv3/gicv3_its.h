#ifndef __GICV3_ITS_H_
#define __GICV3_ITS_H_

#include <asm/sysreg.h>
#include <asm/barriers.h>

#define GITS_SGIR_VPEID			GENMASK_ULL(47, 32)
#define GITS_SGIR_VINTID		GENMASK_ULL(3, 0)

#define GITS_CTLR_ENABLE		(1U << 0)
#define GITS_CTLR_ImDe			(1U << 1)
#define	GITS_CTLR_ITS_NUMBER_SHIFT	4
#define	GITS_CTLR_ITS_NUMBER		(0xFU << GITS_CTLR_ITS_NUMBER_SHIFT)
#define GITS_CTLR_QUIESCENT		(1U << 31)

#define GITS_TYPER_PLPIS		(1UL << 0)
#define GITS_TYPER_VLPIS		(1UL << 1)
#define GITS_TYPER_ITT_ENTRY_SIZE_SHIFT	4
//#define GITS_TYPER_ITT_ENTRY_SIZE	GENMASK_ULL(7, 4)
#define GITS_TYPER_IDBITS_SHIFT		8
#define GITS_TYPER_DEVBITS_SHIFT	13
//#define GITS_TYPER_DEVBITS		GENMASK_ULL(17, 13)
#define GITS_TYPER_PTA			(1UL << 19)
#define GITS_TYPER_HCC_SHIFT		24
#define GITS_TYPER_HCC(r)		(((r) >> GITS_TYPER_HCC_SHIFT) & 0xff)
#define GITS_TYPER_VMOVP		(1ULL << 37)
#define GITS_TYPER_VMAPP		(1ULL << 40)
#define GITS_TYPER_SVPET		GENMASK_ULL(42, 41)

#define GITS_IIDR_REV_SHIFT		12
#define GITS_IIDR_REV_MASK		(0xf << GITS_IIDR_REV_SHIFT)
#define GITS_IIDR_REV(r)		(((r) >> GITS_IIDR_REV_SHIFT) & 0xf)
#define GITS_IIDR_PRODUCTID_SHIFT	24

#define GITS_CBASER_VALID			(1ULL << 63)
#define GITS_CBASER_SHAREABILITY_SHIFT		(10)
#define GITS_CBASER_INNER_CACHEABILITY_SHIFT	(59)
#define GITS_CBASER_OUTER_CACHEABILITY_SHIFT	(53)
#define GITS_CBASER_SHAREABILITY_MASK					\
	GIC_BASER_SHAREABILITY(GITS_CBASER, SHAREABILITY_MASK)
#define GITS_CBASER_INNER_CACHEABILITY_MASK				\
	GIC_BASER_CACHEABILITY(GITS_CBASER, INNER, MASK)
#define GITS_CBASER_OUTER_CACHEABILITY_MASK				\
	GIC_BASER_CACHEABILITY(GITS_CBASER, OUTER, MASK)
#define GITS_CBASER_CACHEABILITY_MASK GITS_CBASER_INNER_CACHEABILITY_MASK

#define GITS_CBASER_InnerShareable					\
	GIC_BASER_SHAREABILITY(GITS_CBASER, InnerShareable)

#define GITS_CBASER_nCnB	GIC_BASER_CACHEABILITY(GITS_CBASER, INNER, nCnB)
#define GITS_CBASER_nC		GIC_BASER_CACHEABILITY(GITS_CBASER, INNER, nC)
#define GITS_CBASER_RaWt	GIC_BASER_CACHEABILITY(GITS_CBASER, INNER, RaWt)
//#define GITS_CBASER_RaWb	GIC_BASER_CACHEABILITY(GITS_CBASER, INNER, RaWb)
#define GITS_CBASER_WaWt	GIC_BASER_CACHEABILITY(GITS_CBASER, INNER, WaWt)
#define GITS_CBASER_WaWb	GIC_BASER_CACHEABILITY(GITS_CBASER, INNER, WaWb)
#define GITS_CBASER_RaWaWt	GIC_BASER_CACHEABILITY(GITS_CBASER, INNER, RaWaWt)
#define GITS_CBASER_RaWaWb	GIC_BASER_CACHEABILITY(GITS_CBASER, INNER, RaWaWb)

#define GITS_CBASER_ADDRESS(cbaser)	((cbaser) & GENMASK_ULL(51, 12))

#define GITS_BASER_NR_REGS		8

#define GITS_BASER_VALID			(1ULL << 63)
#define GITS_BASER_INDIRECT			(1ULL << 62)

#define GITS_BASER_INNER_CACHEABILITY_SHIFT	(59)
#define GITS_BASER_OUTER_CACHEABILITY_SHIFT	(53)
#define GITS_BASER_INNER_CACHEABILITY_MASK				\
	GIC_BASER_CACHEABILITY(GITS_BASER, INNER, MASK)
#define GITS_BASER_CACHEABILITY_MASK		GITS_BASER_INNER_CACHEABILITY_MASK
#define GITS_BASER_OUTER_CACHEABILITY_MASK				\
	GIC_BASER_CACHEABILITY(GITS_BASER, OUTER, MASK)
#define GITS_BASER_SHAREABILITY_MASK					\
	GIC_BASER_SHAREABILITY(GITS_BASER, SHAREABILITY_MASK)

#define GITS_BASER_nCnB		GIC_BASER_CACHEABILITY(GITS_BASER, INNER, nCnB)
#define GITS_BASER_nC		GIC_BASER_CACHEABILITY(GITS_BASER, INNER, nC)
#define GITS_BASER_RaWt		GIC_BASER_CACHEABILITY(GITS_BASER, INNER, RaWt)
//#define GITS_BASER_RaWb		GIC_BASER_CACHEABILITY(GITS_BASER, INNER, RaWb)
#define GITS_BASER_WaWt		GIC_BASER_CACHEABILITY(GITS_BASER, INNER, WaWt)
#define GITS_BASER_WaWb		GIC_BASER_CACHEABILITY(GITS_BASER, INNER, WaWb)
#define GITS_BASER_RaWaWt	GIC_BASER_CACHEABILITY(GITS_BASER, INNER, RaWaWt)
#define GITS_BASER_RaWaWb	GIC_BASER_CACHEABILITY(GITS_BASER, INNER, RaWaWb)

#define GITS_BASER_TYPE_SHIFT			(56)
#define GITS_BASER_TYPE(r)		(((r) >> GITS_BASER_TYPE_SHIFT) & 7)
#define GITS_BASER_ENTRY_SIZE_SHIFT		(48)
#define GITS_BASER_ENTRY_SIZE(r)	((((r) >> GITS_BASER_ENTRY_SIZE_SHIFT) & 0x1f) + 1)
#define GITS_BASER_ENTRY_SIZE_MASK	GENMASK_ULL(52, 48)
#define GITS_BASER_PHYS_52_to_48(phys)					\
	(((phys) & GENMASK_ULL(47, 16)) | (((phys) >> 48) & 0xf) << 12)
#define GITS_BASER_ADDR_48_to_52(baser)					\
	(((baser) & GENMASK_ULL(47, 16)) | (((baser) >> 12) & 0xf) << 48)

#define GITS_BASER_SHAREABILITY_SHIFT	(10)
#define GITS_BASER_InnerShareable					\
	GIC_BASER_SHAREABILITY(GITS_BASER, InnerShareable)
#define GITS_BASER_PAGE_SIZE_SHIFT	(8)
#define __GITS_BASER_PSZ(sz)		(GIC_PAGE_SIZE_ ## sz << GITS_BASER_PAGE_SIZE_SHIFT)
//#define GITS_BASER_PAGE_SIZE_4K		__GITS_BASER_PSZ(4K)
//#define GITS_BASER_PAGE_SIZE_16K	__GITS_BASER_PSZ(16K)
//#define GITS_BASER_PAGE_SIZE_64K	__GITS_BASER_PSZ(64K)
//#define GITS_BASER_PAGE_SIZE_MASK	__GITS_BASER_PSZ(MASK)
#define GITS_BASER_PAGES_MAX		256
#define GITS_BASER_PAGES_SHIFT		(0)
#define GITS_BASER_NR_PAGES(r)		(((r) & 0xff) + 1)

#define GITS_BASER_TYPE_NONE		0
#define GITS_BASER_TYPE_DEVICE		1
#define GITS_BASER_TYPE_VCPU		2
#define GITS_BASER_TYPE_RESERVED3	3
#define GITS_BASER_TYPE_COLLECTION	4
#define GITS_BASER_TYPE_RESERVED5	5
#define GITS_BASER_TYPE_RESERVED6	6
#define GITS_BASER_TYPE_RESERVED7	7

#define GITS_LVL1_ENTRY_SIZE           (8UL)

/*
 * ITS commands
 */
#define GITS_CMD_MAPD			0x08
#define GITS_CMD_MAPC			0x09
#define GITS_CMD_MAPTI			0x0a
#define GITS_CMD_MAPI			0x0b
#define GITS_CMD_MOVI			0x01
#define GITS_CMD_DISCARD		0x0f
#define GITS_CMD_INV			0x0c
#define GITS_CMD_MOVALL			0x0e
#define GITS_CMD_INVALL			0x0d
#define GITS_CMD_INT			0x03
#define GITS_CMD_CLEAR			0x04
#define GITS_CMD_SYNC			0x05

/*
 * GICv4 ITS specific commands
 */
#define GITS_CMD_GICv4(x)		((x) | 0x20)
#define GITS_CMD_VINVALL		GITS_CMD_GICv4(GITS_CMD_INVALL)
#define GITS_CMD_VMAPP			GITS_CMD_GICv4(GITS_CMD_MAPC)
#define GITS_CMD_VMAPTI			GITS_CMD_GICv4(GITS_CMD_MAPTI)
#define GITS_CMD_VMOVI			GITS_CMD_GICv4(GITS_CMD_MOVI)
#define GITS_CMD_VSYNC			GITS_CMD_GICv4(GITS_CMD_SYNC)
/* VMOVP, VSGI and INVDB are the odd ones, as they dont have a physical counterpart */
#define GITS_CMD_VMOVP			GITS_CMD_GICv4(2)
#define GITS_CMD_VSGI			GITS_CMD_GICv4(3)
#define GITS_CMD_INVDB			GITS_CMD_GICv4(0xe)

/*
 * ITS error numbers
 */
#define E_ITS_MOVI_UNMAPPED_INTERRUPT		0x010107
#define E_ITS_MOVI_UNMAPPED_COLLECTION		0x010109
#define E_ITS_INT_UNMAPPED_INTERRUPT		0x010307
#define E_ITS_CLEAR_UNMAPPED_INTERRUPT		0x010507
#define E_ITS_MAPD_DEVICE_OOR			0x010801
#define E_ITS_MAPD_ITTSIZE_OOR			0x010802
#define E_ITS_MAPC_PROCNUM_OOR			0x010902
#define E_ITS_MAPC_COLLECTION_OOR		0x010903
#define E_ITS_MAPTI_UNMAPPED_DEVICE		0x010a04
#define E_ITS_MAPTI_ID_OOR			0x010a05
#define E_ITS_MAPTI_PHYSICALID_OOR		0x010a06
#define E_ITS_INV_UNMAPPED_INTERRUPT		0x010c07
#define E_ITS_INVALL_UNMAPPED_COLLECTION	0x010d09
#define E_ITS_MOVALL_PROCNUM_OOR		0x010e01
#define E_ITS_DISCARD_UNMAPPED_INTERRUPT	0x010f07



#define COMMAND_SIZE  (32)
#define GICV3_ITS_TABLE_TYPE_UNIMPLEMENTED (0x0)
#define GICV3_ITS_TABLE_TYPE_DEVICE        (0x1)
#define GICV3_ITS_TABLE_TYPE_VIRTUAL       (0x2)
#define GICV3_ITS_TABLE_TYPE_COLLECTION    (0x4)

#define GICV3_ITS_TABLE_PAGE_SIZE_4K       (0x0)
#define GICV3_ITS_TABLE_PAGE_SIZE_16K      (0x1)
#define GICV3_ITS_TABLE_PAGE_SIZE_64K      (0x2)


typedef struct {
    uint32_t cmd_nr;
    uint32_t device_id;
    uint32_t event_id;
    uint32_t padding0;
} its_cmd;

/*
 * The ITS command block, which is what the ITS actually parses.
 */
typedef struct {
    union {
        uint64_t raw_cmd64[4];
        uint32_t raw_cmd32[8];
        uint16_t raw_cmd16[16];
        uint8_t  raw_cmd8[32];
    };
} its_cmd_block;

#define ITS_CMD_QUEUE_SZ        SZ_64K
#define ITS_CMD_QUEUE_NR_ENTRIES    (ITS_CMD_QUEUE_SZ / sizeof(struct its_cmd_block))
#define SOC_GIC_ITS0_OFFSET (0x40000)
#define SOC_GIC_ITS1_OFFSET (SOC_GIC_ITS0_OFFSET + SZ_128K)

#define LPI_ID_BITS_MINI_VALUE 14
#define ITS_TABLE_VALID_BIT (1ull << 63)
void init_ITS_command_queue(uint64_t address, uint64_t attributes, uint64_t num_pages);
void set_LPI_config_table_addr(void *gicr_rdbase, uint64_t addr, 
	uint64_t attributes, uint32_t IDbits);
void set_LPI_pending_tableAddr(void *gicr_rdbase, uint64_t addr, 
	uint64_t attributes, uint32_t IDbits);
void enable_LPIs(void *gicr_rdbase);
void set_ITS_table_addr(uint32_t index, uint64_t addr, 
	uint64_t attributes, uint32_t page_size, uint32_t num_pages);
void enable_ITS(void);
void add_ITS_MAPD(uint32_t device_id, uint64_t itt_addr, uint32_t size);
void add_ITS_MAPC(uint64_t rdbase, uint32_t icid);
void add_ITS_SYNC(uint64_t rdbase);
void Add_ITS_MAPTI(uint32_t device_id, uint32_t event_id, uint32_t intid, uint32_t icid);

/*
 * ITS register map
 */
typedef struct {
        volatile uint32_t its_ctlr;             // +0x00000 - RW - ITS Control Register
  const volatile uint32_t its_iidr;             // +0x00004 - RO - IMPLEMENTATION DEFINED / ITS Identification Register
  const volatile uint64_t its_typer;            // +0x00008 - RO - IMPLEMENTATION DEFINED / ITS Type register
  const volatile uint32_t padding1[4];           // +0x00010 - RESERVED
        volatile uint32_t its_impdef[8];        // +0x00020 - RW - IMPLEMENTATION DEFINED
  const volatile uint32_t padding2[16];          // +0x00040 - RESERVED
        volatile uint64_t its_cbaser;           // +0x00080 - RW - ITS Command Queue Descriptor
        volatile uint64_t its_cwriter;          // +0x00088 - RW - ITS Write register
        volatile uint64_t its_creadr;           // +0x00090 - RW - ITS Read register
  const volatile uint32_t padding3[26];          // +0x00098 - RESERVED
        volatile uint64_t its_baser[8];         // +0x00100 - RW - ITS Translation Table Descriptors
  const volatile uint32_t padding4[16320];       // +0x00140 - RESERVED
        volatile uint32_t its_translater;       // +0x10040 - WO - ITS Translation register
  const volatile uint32_t padding5[16367];       // +0x10044 - RESERVED
} GICv3_its;


#endif /* !__GICV3_ITS_H_ */
