#ifndef _REGTEST_H
#define _REGTEST_H
#define REG32_READ(addr)	(* ((volatile u32 *)((unsigned long)addr)) )
#define REG32_WRITE(addr, val)	(* ((volatile u32 *)((unsigned long)addr)) = ((u32)val))
#define REG64_READ(addr)	(* ((volatile u64 *)((unsigned long)addr)) )
#define REG64_WRITE(addr, val)	(* ((volatile u64 *)((unsigned long)addr)) = ((u64)val))
#define REG8_READ(addr)	(* ((volatile u8 *)((unsigned long)addr)) )
#define REG8_WRITE(addr, val)	(* ((volatile u8 *)((unsigned long)addr)) = ((u8)val))
#define REG16_READ(addr)	(* ((volatile u16 *)((unsigned long)addr)) )
#define REG16_WRITE(addr, val)	(* ((volatile u16 *)((unsigned long)addr)) = ((u16)val))

#define REG_BIT_SET32(address, mask)      ((*(volatile unsigned long*)(address))|= (mask))
#define REG_BIT_CLEAR32(address, mask)    ((*(volatile unsigned long*)(address))&= (~(mask)))

#define REG_RMW32(address, mask, value)   (REG32_WRITE((address), ((REG32_READ(address)& ((unsigned long)~(mask)))| (value))))

#define REG32(addr) (* ((volatile u32 *)((unsigned long)addr)) )
#define REG8(addr)	(* ((volatile u8 *)((unsigned long)addr)) )

#define REG_RW_TEST(addr, bit_strobe) do {\
	u32 _i; \
	REG32_WRITE(addr, 0x0&(bit_strobe)); \
	HCOMP(0x0&(bit_strobe), REG32_READ(addr)&(bit_strobe)); \
	REG32_WRITE(addr, 0xFFFFFFFF&(bit_strobe)); \
	HCOMP(0xFFFFFFFF&(bit_strobe), REG32_READ(addr)&(bit_strobe)); \
	REG32_WRITE(addr, 0x0&(bit_strobe)); \
	HCOMP(0x0&(bit_strobe), REG32_READ(addr)&(bit_strobe)); \
	REG32_WRITE(addr, 0x55555555&(bit_strobe)); \
	HCOMP(0x55555555&(bit_strobe), REG32_READ(addr)&(bit_strobe)); \
	REG32_WRITE(addr, 0xAAAAAAAA&bit_strobe); \
	HCOMP(0xAAAAAAAA&(bit_strobe), REG32_READ(addr)&(bit_strobe)); \
	for(_i = 0; _i < 32; _i++) {\
		if((0x1<<_i) & (bit_strobe)) {\
			REG32_WRITE(addr, (0x0<<_i)); \
			HCOMP(0x0<<_i, REG32_READ(addr)&(0x1<<_i)); \
			REG32_WRITE(addr, (0x1<<_i)); \
			HCOMP(0x1<<_i, REG32_READ(addr)&(0x1<<_i)); \
			REG32_WRITE(addr, (0x0<<_i)); \
			HCOMP(0x0<<_i, REG32_READ(addr)&(0x1<<_i)); \
		} \
	} \
}while(0)

#define REG_RO_TEST(addr, bit_strobe) do {\
	u32 _i, _val; \
	_val = REG32_READ(addr); \
	REG32_WRITE(addr, 0x0&bit_strobe); \
	HCOMP(_val&bit_strobe, REG32_READ(addr)&bit_strobe); \
	REG32_WRITE(addr, 0xFFFFFFFF&bit_strobe); \
	HCOMP(_val&bit_strobe, REG32_READ(addr)&bit_strobe); \
	REG32_WRITE(addr, 0x55555555&bit_strobe); \
	HCOMP(_val&bit_strobe, REG32_READ(addr)&bit_strobe); \
	REG32_WRITE(addr, 0xAAAAAAAA&bit_strobe); \
	HCOMP(_val&bit_strobe, REG32_READ(addr)&bit_strobe); \
	for(_i = 0; _i < 32; _i++) {\
		if((0x1<<_i) & bit_strobe) {\
			REG32_WRITE(addr, (0x0<<_i)); \
			HCOMP(_val&(0x1<<_i), REG32_READ(addr)&(0x1<<_i)); \
			REG32_WRITE(addr, (0x1<<_i)); \
			HCOMP(_val&(0x1<<_i), REG32_READ(addr)&(0x1<<_i)); \
		} \
	} \
}while(0)


/**
@{
* @brief Bit masks declarations.
*/
#ifndef BIT31
    #define BIT31   ((unsigned long)0x80000000UL)
#else 
      #if (BIT31 != 0x80000000UL) 
             #error "BIT31 is defined outside MCAL and it is value is not 0x80000000UL". 
       #endif 
#endif 
    
#ifndef BIT30
    #define BIT30   ((unsigned long)0x40000000UL)
#else 
      #if (BIT30 != 0x40000000UL) 
             #error "BIT30 is defined outside MCAL and it is value is not 0x40000000UL". 
       #endif 
#endif 

#ifndef BIT29
    #define BIT29   ((unsigned long)0x20000000UL)
#else 
      #if (BIT29 != 0x20000000UL) 
             #error "BIT29 is defined outside MCAL and it is value is not 0x20000000UL". 
       #endif 
#endif 

#ifndef BIT28
    #define BIT28   ((unsigned long)0x10000000UL)
#else 
      #if (BIT28 != 0x10000000UL) 
             #error "BIT28 is defined outside MCAL and it is value is not 0x10000000UL". 
       #endif 
#endif 

#ifndef BIT27
    #define BIT27   ((unsigned long)0x08000000UL)
#else 
      #if (BIT27 != 0x08000000UL) 
             #error "BIT27 is defined outside MCAL and it is value is not 0x08000000UL". 
       #endif 
#endif 

#ifndef BIT26
    #define BIT26   ((unsigned long)0x04000000UL)
#else 
      #if (BIT26 != 0x04000000UL) 
             #error "BIT26 is defined outside MCAL and it is value is not 0x04000000UL". 
       #endif 
#endif 

#ifndef BIT25
    #define BIT25   ((unsigned long)0x02000000UL)
#else 
      #if (BIT25 != 0x02000000UL) 
             #error "BIT25 is defined outside MCAL and it is value is not 0x02000000UL". 
       #endif 
#endif 

#ifndef BIT24
    #define BIT24   ((unsigned long)0x01000000UL)
#else 
      #if (BIT24 != 0x01000000UL) 
             #error "BIT24 is defined outside MCAL and it is value is not 0x01000000UL". 
       #endif 
#endif 

#ifndef BIT23
    #define BIT23   ((unsigned long)0x00800000UL)
#else 
      #if (BIT23 != 0x00800000UL) 
             #error "BIT23 is defined outside MCAL and it is value is not 0x00800000UL". 
       #endif 
#endif 

#ifndef BIT22
    #define BIT22   ((unsigned long)0x00400000UL)
#else 
      #if (BIT22 != 0x00400000UL) 
             #error "BIT22 is defined outside MCAL and it is value is not 0x00400000UL". 
       #endif 
#endif 

#ifndef BIT21
    #define BIT21   ((unsigned long)0x00200000UL)
#else 
      #if (BIT21 != 0x00200000UL) 
             #error "BIT21 is defined outside MCAL and it is value is not 0x00200000UL". 
       #endif 
#endif 

#ifndef BIT20
    #define BIT20   ((unsigned long)0x00100000UL)
#else 
      #if (BIT20 != 0x00100000UL) 
             #error "BIT20 is defined outside MCAL and it is value is not 0x00100000UL". 
       #endif 
#endif 

#ifndef BIT19
    #define BIT19   ((unsigned long)0x00080000UL)
#else 
      #if (BIT19 != 0x00080000UL) 
             #error "BIT19 is defined outside MCAL and it is value is not 0x00080000UL". 
       #endif 
#endif 

#ifndef BIT18
    #define BIT18   ((unsigned long)0x00040000UL)
#else 
      #if (BIT18 != 0x00040000UL) 
             #error "BIT18 is defined outside MCAL and it is value is not 0x00040000UL". 
       #endif 
#endif 

#ifndef BIT17
    #define BIT17   ((unsigned long)0x00020000UL)
#else 
      #if (BIT17 != 0x00020000UL) 
             #error "BIT17 is defined outside MCAL and it is value is not 0x00020000UL". 
       #endif 
#endif 

#ifndef BIT16
    #define BIT16   ((unsigned long)0x00010000UL)
#else 
      #if (BIT16 != 0x00010000UL) 
             #error "BIT16 is defined outside MCAL and it is value is not 0x00010000UL". 
       #endif 
#endif 

#ifndef BIT15
    #define BIT15   ((unsigned long)0x00008000UL)
#else 
      #if (BIT15 != 0x00008000UL) 
             #error "BIT15 is defined outside MCAL and it is value is not 0x00008000UL". 
       #endif 
#endif 

#ifndef BIT14
    #define BIT14   ((unsigned long)0x00004000UL)
#else 
      #if (BIT14 != 0x00004000UL) 
             #error "BIT14 is defined outside MCAL and it is value is not 0x00004000UL". 
       #endif 
#endif 

#ifndef BIT13
    #define BIT13   ((unsigned long)0x00002000UL)
#else 
      #if (BIT13 != 0x00002000UL) 
             #error "BIT13 is defined outside MCAL and it is value is not 0x00002000UL". 
       #endif 
#endif 

#ifndef BIT12
    #define BIT12   ((unsigned long)0x00001000UL)
#else 
      #if (BIT12 != 0x00001000UL) 
             #error "BIT12 is defined outside MCAL and it is value is not 0x00001000UL". 
       #endif 
#endif 

#ifndef BIT11  
    #define BIT11   ((unsigned long)0x00000800UL)
#else 
      #if (BIT11 != 0x00000800UL) 
             #error "BIT11 is defined outside MCAL and it is value is not 0x00000800UL". 
       #endif 
#endif 

#ifndef BIT10
    #define BIT10   ((unsigned long)0x00000400UL)
#else 
      #if (BIT10 != 0x00000400UL) 
             #error "BIT10 is defined outside MCAL and it is value is not 0x00000400UL". 
       #endif 
#endif 

#ifndef BIT9
    #define BIT9    ((unsigned long)0x00000200UL)
#else 
      #if (BIT9 != 0x00000200UL) 
             #error "BIT9 is defined outside MCAL and it is value is not 0x00000200UL". 
       #endif 
#endif 

#ifndef BIT8
    #define BIT8    ((unsigned long)0x00000100UL)
#else 
      #if (BIT8 != 0x00000100UL) 
             #error "BIT8 is defined outside MCAL and it is value is not 0x00000100UL ". 
       #endif 
#endif 

#ifndef BIT7
    #define BIT7    ((unsigned long)0x00000080UL)
#else 
      #if (BIT7 != 0x00000080UL) 
             #error "BIT7 is defined outside MCAL and it is value is not 0x00000080UL". 
       #endif 
#endif 

#ifndef BIT6
    #define BIT6    ((unsigned long)0x00000040UL)
#else 
      #if (BIT6 != 0x00000040UL) 
             #error "BIT6 is defined outside MCAL and it is value is not 0x00000040UL". 
       #endif 
#endif 

#ifndef BIT5
    #define BIT5    ((unsigned long)0x00000020UL)
#else 
      #if (BIT5 != 0x00000020UL) 
             #error "BIT5 is defined outside MCAL and it is value is not 0x00000020UL". 
       #endif 
#endif 

#ifndef BIT4
    #define BIT4    ((unsigned long)0x00000010UL)
#else 
      #if (BIT4 != 0x00000010UL) 
             #error "BIT4 is defined outside MCAL and it is value is not 0x00000010UL". 
       #endif 
#endif 

#ifndef BIT3
    #define BIT3    ((unsigned long)0x00000008UL)
#else 
      #if (BIT3 != 0x00000008UL) 
             #error "BIT3 is defined outside MCAL and it is value is not 0x00000008UL". 
       #endif 
#endif 

#ifndef BIT2
    #define BIT2    ((unsigned long)0x00000004UL)
#else 
      #if (BIT2 != 0x00000004UL) 
             #error "BIT2 is defined outside MCAL and it is value is not 0x00000004UL". 
       #endif 
#endif 

#ifndef BIT1
    #define BIT1    ((unsigned long)0x00000002UL)
#else 
      #if (BIT1 != 0x00000002UL) 
             #error "BIT1 is defined outside MCAL and it is value is not 0x00000002UL". 
       #endif 
#endif 

#ifndef BIT0
    #define BIT0    ((unsigned long)0x00000001UL)
#else 
      #if (BIT0 != 0x00000001UL) 
             #error "BIT0 is defined outside MCAL and it is value is not 0x00000001UL". 
       #endif 
#endif

/**
* @brief 32 bits bits clearing macro.
* @violates @ref StdRegMacros_h_REF_1 MISRA 2004 Advisory Rule 19.7, A function should be used in
* preference to a function-like macro.
*/
#if !defined(REG_BIT_CLEAR32)
#define REG_BIT_CLEAR32(address, mask)    ((*(volatile unsigned int*)(address))&= (~(mask)))
#endif

/**
* @brief 32 bits bits getting macro.
* @violates @ref StdRegMacros_h_REF_1 MISRA 2004 Advisory Rule 19.7, A function should be used in
* preference to a function-like macro.
*/
#define REG_BIT_GET32(address, mask)      ((*(volatile unsigned int*)(address))& (mask))

/**
* @brief 32 bits bits setting macro.
* @violates @ref StdRegMacros_h_REF_1 MISRA 2004 Advisory Rule 19.7, A function should be used in
* preference to a function-like macro.
*/
#if !defined(REG_BIT_SET32)
#define REG_BIT_SET32(address, mask)      ((*(volatile unsigned int*)(address))|= (mask))
#endif

#endif
