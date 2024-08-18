#ifndef __PUBLIC_H
#define __PUBLIC_H

#define __UNUSED_ __attribute__((__unused__))
#define REPORT(x) \
	printf("\n\r %s:%s\n", __func__, x > 0 ? "SUCCESS!" : "FAIL!");
#define SUCCESS 1
#define FAIL 0

#define REPORT_CASE(_condition) \
	do {\
		if (_condition) {\
			printf("\n\r [Test Case]%s:%s\n", __func__, "SUCCESS!" );\
		} else { \
			printf("\n\r [Test Case]%s:%s\n", __func__, "FAIL!" );\
		}\
	} while(0);

enum {
	ATTR_RW = 0,
	ATTR_RO,
	ATTR_WO,
};

typedef struct {
	u64 addr;
	u32 def_val;
	u32 bit_mask;
	u32 attr;
} ST_REG_TEST;

#define SIZE_1M (1 << 20)
#define SIZE_2M (2ULL * SIZE_1M)
#define SIZE_16M (16ULL * SIZE_1M)
#define SIZE_1G (1ULL << 30)
#define LOW32_MASK (0xfffffffful)
#define SIZE_512k (1 << 19)
#define SIZE_256k (1 << 18)
#define SIZE_128k (1 << 17)
#define SIZE_64k (1 << 16)
#define SIZE_32k (1 << 15)
#define SIZE_16k (1 << 14)
#define SIZE_8k (1 << 13)
#define SIZE_4k (1 << 12)
#define SIZE_2k (1 << 11)
#define SIZE_1k (1 << 10)

#define VM_NUM (4)
#define APP_NUM (1)

typedef struct {
	u64 s2_map_offset[VM_NUM];
	u64 s1_map_offset[APP_NUM];
	/* only used for secure ste table */
	u32 s1_nscfg;
	/* PTE table whether is in secure world */
	u32 s1_nstable;
	u16 streamid[VM_NUM];
	u16 substreamid[APP_NUM];
	u64 cmn_offset;
} ST_SMMU_PARA;

enum {
	SMMU_STAGE1_ENABLE = 0,
	SMMU_STAGE2_ENABLE,
	SMMU_STAGE1_2_ENABLE,
};

enum {
	CD_NSCFG_SECURE = 0,
	CD_NSCFG_NON_SECURE,
};
enum {
	PTE_SECURE_TABLE = 0,
	PTE_NON_SECURE_TABLE,
};

enum {
	SMMU_STAGE1 = 0,
	SMMU_STAGE2,
	SMMU_STAGE1_2,
};

typedef union {
	struct {
		/* safety interrupt */
		u32 fmu_fault_int:1; //rw
		u32 fmu_err_int:1;
		u32 pmu_irpt_int:1; //BIT2 rw
		u32 ras_irpt_int:1; //BIT3 rW
		u32 global_irpt_s_int:1;
		u32 global_irpt_ns_int:1; //BIT5
		u32 cmd_sync_irpt_s_int:1; // BIT6
		u32 cmd_sync_irpt_ns_int:1; // BIT7

		// non-secure interrupt
		u32 pri_q_irpt_ns_int:1; //BIT8  rw
		u32 event_q_irpt_s_int:1; // BIT9 ro
		u32 event_q_irpt_ns_int:1; // BIT10

		u32 res0:21; //
	} bit;
	u32 data;
}UN_SMMU_TCU_INT_REG;

typedef struct {
	int irq;
	UN_SMMU_TCU_INT_REG status;
	u32 int_flag;
	/* if test tbu, then set tbu register otherwise set tcu register */
	u64 is_tbu;
	u32 tbu_id;
	u64 fmu_err_status;
} ST_SMMUV3_G_PARA;

typedef struct {
	int irq;
	u64 smmu_reg_base;
} ST_SMMUV3_IRQ_PARA;

typedef enum {
	SMMU_TCU_FMU_FAULT_INT = 0,
	SMMU_TCU_FMU_ERR_INT,
	SMMU_TCU_PMU_IRPT_INT,
	SMMU_TCU_RAS_IRPT_INT,
	SMMU_TCU_GLOBAL_IRPT_S_INT,
	SMMU_TCU_GLOBAL_IRPT_NS_INT = 5,
	SMMU_TCU_CMD_SYNC_IRPT_S_INT,
	SMMU_TCU_CMD_SYNC_IRPT_NS_INT,
	SMMU_TCU_PRI_Q_IRPT_NS_INT,
	SMMU_TCU_EVENT_Q_IRPT_S_INT,
	SMMU_TCU_EVENT_Q_IRPT_NS_INT = 10,
	SMMU_TCU_BUFF,
} EN_TCU_INT_TYPE;

typedef enum {
	SMMU_TBU_WUSER_M_CHK_INTR = 0,
	SMMU_TBU_WSTRB_M_CHK_INTR,
	SMMU_TBU_ARUSER_M_CHK,
	SMMU_TBU_AWUSER_M_CHK_INTR,
	SMMU_TBU_ARID_M_CHK_INTR,
	SMMU_TBU_AWID_M_CHK_INTR = 5,
	SMMU_TBU_BUSER_S_CHK_INTR,
	SMMU_TBU_BID_S_CHK_INTR,
	SMMU_TBU_RUSER_S_CHK_INTR,
	SMMU_TBU_RID_S_CHK_INTR,
	SMMU_TBU_PMU_IRPT = 10,
	SMMU_TBU_RAS_IRPT,
	SMMU_TBU_BUFF,
} EN_TBU_INT_TYPE;

#define SMMU_PCIE_TBU_NUM 2
#define SMMU_GPU_TBU_NUM 4
#define SMMU_SOC_DMA_TBU_NUM 1
#define SMMU_SOC_TBU_NUM 1
#define SMMU_COREIP_TBU_NUM 7
#define SMMU_D71_S1_TBU_NUM 3
#define SMMU_D71_S2_TBU_NUM 4
#define SMMU_CODEC_TBU_NUM 2

void smmu_clean_all_tbu_int(u64 smmu_reg_base, u32 tbu_id);
void smmu_tbu_trigger_pmu_int(u64 smmu_reg_base, u32 tbu_id);

int wait_and_check_int_for_smmu_tbu(EN_TBU_INT_TYPE int_type, u32 irq);
void smmuv3_irq_handle(void *arg);
void smmuv3_fusa_irq_handle(void *arg);
int wait_and_check_int_for_smmu(EN_TCU_INT_TYPE int_type, u32 irq);
void smmu_clean_all_tcu_int(u64 smmu_reg_base);
int cmd_error_to_smmu(u64 reg_base);
int is_smmu_already_init(u64 smmu_reg_base);
void smmu_tcu_trigger_pmu_int(u64 smmu_reg_base);
void smmu_tcu_trigger_ras_int(u64 smmu_reg_base);
void smmu_tbu_trigger_ras_int(u64 smmu_reg_base, u32 tbu_id);

int smmu_tcu_fmu_inject_error_test(u64 smmu_reg_base, u32 irq);
int smmu_tbu_fmu_inject_error_test(u64 smmu_reg_base, u32 irq, u32 tbu_num);
int smmu_tcu_fmu_pingnow_error_test(u64 smmu_reg_base, u32 irq);
int smmu_tbu_fmu_pingnow_error_test(u64 smmu_reg_base, u32 irq, u8 tbu_id);
int smmu_fmu_inject_error_test(u64 smmu_reg_base, u32 irq);


/* secure smmu use api */
int cmd_sync_to_s_smmu(u64 reg_base);
int cmd_error_to_s_smmu(u64 reg_base);

extern ST_SMMUV3_G_PARA smmuv3_para;

int smmu_init_s1_1g(u32 feature, u64 reg_base, u64 data_base, ST_SMMU_PARA *st_mmu);
int smmu_init_s1_2m(u32 feature, u64 reg_base, u64 data_base, ST_SMMU_PARA *st_mmu);
int smmu_init_s1_2m_with_2state(u32 feature, u64 reg_base, u64 data_base, ST_SMMU_PARA *st_mmu);
int smmu_init_s1_2m_tcu_cmn(u32 feature, u64 reg_base, u64 data_base, ST_SMMU_PARA *st_mmu);
int smmu_init_s1_2m_tcu_cmn_s2_2G_size(u32 feature, u64 reg_base, u64 data_base, ST_SMMU_PARA *st_mmu);

void smmu_disable(u64 smmu_reg_base);
void invalidate_smmu_tlb(void);
void modify_smmu_tlb(u32 stage, u64 data_base, ST_SMMU_PARA *st_mmu);
void inv_tlb_by_write_smmu_reg(u64 smmu_reg_base);
int cmd_sync_to_smmu(u64 reg_base);

enum {
	CASE_ID0 = 0,
	CASE_ID1,
	CASE_ID2,
	CASE_ID3,
	CASE_ID4,
	CASE_ID5,
	CASE_ID6,
	CASE_ID7,
	CASE_ID8,
	CASE_ID9,
	CASE_ID10,
};

int i2s_master_edp_output_32bit_shown(u32 * audio_file, u32 audio_size, bool pattern_en);

#endif

