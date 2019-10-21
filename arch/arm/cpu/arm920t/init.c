#if 0
/* NAND FLASH控制器 */  
#define NFCONF (*((volatile unsigned long *)0x4E000000))  
/*#define NFCONT (*((volatile unsigned long *)0x4E000004))  */
#define NFCMMD (*((volatile unsigned char *)0x4E000004))  
#define NFADDR (*((volatile unsigned char *)0x4E000008))  
#define NFDATA (*((volatile unsigned char *)0x4E00000C))  
#define NFSTAT (*((volatile unsigned char *)0x4E000010))  
/* GPIO */  
#define GPHCON              (*(volatile unsigned long *)0x56000070)  
#define GPHUP               (*(volatile unsigned long *)0x56000078)  
/* UART registers*/  
#define ULCON0              (*(volatile unsigned long *)0x50000000)  
#define UCON0               (*(volatile unsigned long *)0x50000004)  
#define UFCON0              (*(volatile unsigned long *)0x50000008)  
#define UMCON0              (*(volatile unsigned long *)0x5000000c)  
#define UTRSTAT0            (*(volatile unsigned long *)0x50000010)  
#define UTXH0               (*(volatile unsigned char *)0x50000020)  
#define URXH0               (*(volatile unsigned char *)0x50000024)  
#define UBRDIV0             (*(volatile unsigned long *)0x50000028)  
#define TXD0READY   (1<<2)  


#define NAND_SECTOR_SIZE 512    /*For K9F1208, SECTOR SIZE IS 512 BYTE*/

void nand_read_ll(unsigned int addr, unsigned char *buf, unsigned int len);  
static int isBootFromNorFlash(void)  
{  
#if 0
    volatile int *p = (volatile int *)0;  
    int val;  
    val = *p;  
    *p = 0x12345678;  
    if (*p == 0x12345678)  
    {  
        /* 写成功, 是nand启动 */  
        *p = val;  
        return 0;  
    }  
    else  
    {  
        /* NOR不能像内存一样写 */  
        return 1;  
    } 
#endif
	return 0;
}  
void copy_code_to_sdram(unsigned char *src, unsigned char *dest, unsigned int len)  
{     
    int i = 0;  
    /* 如果是NOR启动 */  
    if (isBootFromNorFlash())  
    {  
        while (i < len)  
        {  
            dest[i] = src[i];  
            i++;  
        }  
    }  
    else  
    {  
        //nand_init();  
        nand_read_ll((unsigned int)src, dest, len);  
    }  
}  
void clear_bss(void)  
{  
    extern int __bss_start, __bss_end;  
    int *p = &__bss_start;  
    for (; p < &__bss_end; p++)  
        *p = 0;  
}  
void nand_init_ll(void)  
{  
#define TACLS   0  
#define TWRPH0  1  
#define TWRPH1  0  
    /* 设置时序 */  
	/* 使能NAND Flash控制器, 初始化ECC, 禁止片选 */  
    NFCONF = (TACLS<<8)|(TWRPH0<<4)|(TWRPH1<<0) | (1<<15)|(1<<12)|(1<<11);  
  
}  
static void nand_select(void)  
{  
    NFCONF &= ~(1<<11);   
}  
static void nand_deselect(void)  
{  
    NFCONF |= (1<<11);     
}  
static void nand_cmd(unsigned char cmd)  
{  
    volatile int i;  
    NFCMMD = cmd;  
    for (i = 0; i < 10; i++);  
}  
static void nand_addr(unsigned int addr)  
{  
    unsigned int col  = addr % NAND_SECTOR_SIZE;  
    unsigned int page = addr / NAND_SECTOR_SIZE;  
    volatile int i;  
    NFADDR = col & 0xff;  
    for (i = 0; i < 10; i++);  
    NFADDR = (col >> 8) & 0xff;  
    for (i = 0; i < 10; i++);  
    NFADDR  = page & 0xff;  
    for (i = 0; i < 10; i++);  
    NFADDR  = (page >> 8) & 0xff;  
    for (i = 0; i < 10; i++);  
    NFADDR  = (page >> 16) & 0xff;  
    for (i = 0; i < 10; i++);     
}  
static void nand_wait_ready(void)  
{  
    while (!(NFSTAT & 1));  
}  
static unsigned char nand_data(void)  
{  
    return NFDATA;  
}  


void nand_read_ll(unsigned int addr, unsigned char *buf, unsigned int len)  
{  
    int col = addr % NAND_SECTOR_SIZE;  
    int i = 0;  
    /* 1. 选中 */  
    nand_select();  
    while (i < len)  
    {  
        /* 2. 发出读命令00h */  
        nand_cmd(0x00);  
        /* 3. 发出地址(分5步发出) */  
        nand_addr(addr);  
        /* 4. 发出读命令30h */  
        nand_cmd(0x30);  
        /* 5. 判断状态 */  
        nand_wait_ready();  
        /* 6. 读数据 */  
        for (; (col < NAND_SECTOR_SIZE) && (i < len); col++)  
        {  
            buf[i] = nand_data();  
            i++;  
            addr++;  
        }  
        col = 0;  
    }  
    /* 7. 取消选中 */         
    nand_deselect();  
}  
#else


#include <config.h>

#define __REGb(x)	(*(volatile unsigned char *)(x))
#define __REGi(x)	(*(volatile unsigned int *)(x))
#define NF_BASE		0x4e000000
#define NFCONF		__REGi(NF_BASE + 0x0)
#define NFCMD		__REGb(NF_BASE + 0x4)
#define NFADDR		__REGb(NF_BASE + 0x8)
#define NFDATA		__REGb(NF_BASE + 0xc)
#define NFSTAT		__REGb(NF_BASE + 0x10)

#define BUSY 1
inline void wait_idle(void) {
    int i;

    while(!(NFSTAT & BUSY))
      for(i=0; i<10; i++);
}

#define NAND_SECTOR_SIZE	512
#define NAND_BLOCK_MASK		(NAND_SECTOR_SIZE - 1)

/* low level nand read function */
int
nand_read_ll(unsigned char *buf, unsigned long start_addr, int size)
{
    int i, j;

    if ((start_addr & NAND_BLOCK_MASK) || (size & NAND_BLOCK_MASK)) {
        return -1;	/* invalid alignment */
    }

    /* chip Enable */
    NFCONF &= ~0x800;
    for(i=0; i<10; i++);

    for(i=start_addr; i < (start_addr + size);) {
      /* READ0 */
      NFCMD = 0;

      /* Write Address */
      NFADDR = i & 0xff;
      NFADDR = (i >> 9) & 0xff;
      NFADDR = (i >> 17) & 0xff;
      NFADDR = (i >> 25) & 0xff;

      wait_idle();

      for(j=0; j < NAND_SECTOR_SIZE; j++, i++) {
	*buf = (NFDATA & 0xff);
	buf++;
      }
    }

    /* chip Disable */
    NFCONF |= 0x800;	/* chip disable */

    return 0;
}

void copy_code_to_sdram(unsigned char *src, unsigned char *dest, unsigned int len)  
{     
    int i = 0;  
    /* 如果是NOR启动 */  
    /*if (isBootFromNorFlash())  
    {  
        while (i < len)  
        {  
            dest[i] = src[i];  
            i++;  
        }  
    }  
    else */ 
    {  
        //nand_init();  
        nand_read_ll(dest,(unsigned long)src, len);  
    }  
}  


void clear_bss(void)  
{  
    extern int __bss_start, __bss_end;  
    int *p = &__bss_start;  
    for (; p < &__bss_end; p++)  
        *p = 0;  
} 

#endif

