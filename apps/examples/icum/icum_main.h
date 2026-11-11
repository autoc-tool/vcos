/**
 * file: code_hsm.c
 *
 * CopyRight: (C) 理想汽车有限公司版权所有
*/

/* Variable */

#ifndef __ICUM_MAIN_H__
#define __ICUM_MAIN_H__

#define HSM_VERSION                         (0x0000C000U)       // 00 00C 000表示 0.11.RC3版本

#define HSM_NULL ((void *)0)

# define VHSM_HAL_ICUM_CMDREG_base                                    0xFF1F0000u

# define VHSM_HAL_ICUM_ICU2PES                                        (*(volatile uint32_t *) (VHSM_HAL_ICUM_CMDREG_base + 0x0010u))
# define VHSM_HAL_ICUM_PE2ICUS                                        (*(volatile uint32_t *) (VHSM_HAL_ICUM_CMDREG_base + 0x0014u))
# define VHSM_HAL_ICUM_PE2ICUF                                        (*(volatile uint32_t *) (VHSM_HAL_ICUM_CMDREG_base + 0x0040u))
# define VHSM_HAL_ICUM_PE2ICUFS                                       (*(volatile uint32_t *) (VHSM_HAL_ICUM_CMDREG_base + 0x0044u))
# define VHSM_HAL_ICUM_PE2ICUFC                                       (*(volatile uint32_t *) (VHSM_HAL_ICUM_CMDREG_base + 0x0048u))
# define VHSM_HAL_ICUM_PE2ICUSIE                                      (*(volatile uint32_t *) (VHSM_HAL_ICUM_CMDREG_base + 0x004Cu))

//Trng
# define VHSM_HAL_ICUM_TRNGGC0_BASE                                    0xFFFEE0A0u

# define VHSM_HAL_ICUM_TRNGGC0SR                                       (*(volatile uint32_t *) (VHSM_HAL_ICUM_TRNGGC0_BASE + 0x0000u))
# define VHSM_HAL_ICUM_TRNGGC0SCLR                                     (*(volatile uint32_t *) (VHSM_HAL_ICUM_TRNGGC0_BASE + 0x0004u))
# define VHSM_HAL_ICUM_TRNGGC0CR                                       (*(volatile uint32_t *) (VHSM_HAL_ICUM_TRNGGC0_BASE + 0x0008u))
# define VHSM_HAL_ICUM_TRNGGC0RNRR                                     (*(volatile uint32_t *) (VHSM_HAL_ICUM_TRNGGC0_BASE + 0x000Cu))
# define VHSM_HAL_ICUM_TRNGGC0VER                                      (*(volatile uint32_t *) (VHSM_HAL_ICUM_TRNGGC0_BASE + 0x001Cu))

extern volatile uint32_t * volatile pHOST2HSMbuf;    //pointer to buffer for Host to HSM parameter transfer
extern volatile uint32_t * volatile pHSM2HOSTbuf;    //pointer to buffer for HSM to Host parameter transfer
/* Function */
void hsmLog(uint32_t node, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4);

#define AES_KEY_SIZE_SUPPORT                (16)
#define AES_IV_SIZE_SUPPORT                 (16)

// HSM相关寄存器单次处理4个uint32，使用如下宏
#define HSM_PARA_0(i)                       ((i) + 0)
#define HSM_PARA_1(i)                       ((i) + 1)
#define HSM_PARA_2(i)                       ((i) + 2)
#define HSM_PARA_3(i)                       ((i) + 3)
#define HSM_PARA_PROCESS_OFFSET             (4)

#define HSM_STATUS_SUCCESS                  (0xCB0001BCu)
#define HSM_STATUS_ERROR                    (0xCB0002BCu)
#define HSM_STATUS_CRASH                    (0xCB0003BCu)

#define CMD_WAIT                            (-2)    //Negative commands are commands which are used for intermediate state
#define CMD_NOTHINGTODO                     (-1)    //After receiving a command and correct execution this state should be reached
#define CMD_SETADDR_HOST2HSMBUF             (0)
#define CMD_SETADDR_HSM2HOSTBUF             (1)
#define CMD_RUN_TRNG                        (2)

#define CMD_RUN_AES                         (3)

#define UINT32_TO_UINT8_NUM                                         (4)

typedef enum {
    HSM_CMD_AES_ECB_ENCRYPT                 = 0xffu,
    HSM_CMD_AES_ECB_DECRYPT                 = 0xfeu,
    HSM_CMD_AES_CBC_ENCRYPT                 = 0xfdu,
    HSM_CMD_AES_CBC_DECRYPT                 = 0xfcu,
    HSM_CMD_AES_ECB_ENCRYPT_FOR_RAM         = 0xfbu,
    HSM_CMD_AES_ECB_DECRYPT_FOR_RAM         = 0xfau,
    HSM_CMD_AES_CBC_ENCRYPT_FOR_RAM         = 0xf9u,
    HSM_CMD_AES_CBC_DECRYPT_FOR_RAM         = 0xf8u,
} HSM_CMD_AES_TYPE;

#define CMD_RUN_HASH                        (4)
typedef enum {
    HSM_CMD_HASH_SHA_256                    = 0xffu,
    HSM_CMD_HASH_SHA_512                    = 0xfeu,
} HSM_CMD_HASH_TYPE;

#define CMD_RUN_PKC                         (5)
#define CMD_RUN_HEARTBEAT                   (6)

#define CMD_RUN_KEY                         (7)
typedef enum {
    HSM_CMD_SET_KEY_ELEMENT                 = 0xffu,
    HSM_CMD_GET_KEY_ELEMENT                 = 0xfeu,
    HSM_CMD_SET_KEY_VALID                   = 0xfdu,
    HSM_CMD_GET_KEY_STATUS                  = 0xfcu,
} HSM_CMD_KEY_TYPE;

#define CMD_QUERY_INFORMATION               (8)
typedef enum {
    HSM_CMD_QUERY_INFORMATION_VERSION       = 0xff,
    HSM_CMD_QUERY_INFORMATION_SECURE_BOOT   = 0xfe,
    HSM_CMD_QUERY_ANY_INFORMATION           = 0xfd,
} HSM_CMD_QUERY_INFORMATION_TYPE;

#define CMD_RSA2048                         (9)

#define CMD_RUN_CMAC                        (10)
typedef enum {
    HSM_CMD_CMAC_GENERATE                   = 0xffu,
    HSM_CMD_CMAC_VERIFY                     = 0xfeu,
} HSM_CMD_CMAC_TYPE;

typedef enum {
    READ_TYPE,
    WRITE_TYPE,
} RWType;

typedef enum {
    Direct,
    Bridge,
    Forbidden,
} ACCESS_METHOD;

typedef struct {
    uint32_t start;
    uint32_t end;
    ACCESS_METHOD readAllowed;
    ACCESS_METHOD writeAllowed;
} ADDR_RANGE;

// reserved
#define CMD_RUN_SYSTICK                     (25)
#define CMD_ENCRYPTK0                       (26)
#define CMD_DECRYPTK0                       (27)
#define CMD_SETSIZE_HOST2HSMBUF             (28)
#define CMD_RUN_TIMER                       (29)
#define CMD_RUN_WATCHDOG                    (30)

// test
#define CMD_RUN_TEST                        (31)
typedef enum {
    HSM_CMD_TEST_LOG                        = 0xffu,
    HSM_CMD_TEST_COM                        = 0xfeu,
    HSM_CMD_TEST_RESET                      = 0xfdu,
} HSM_CMD_TEST_TYPE;

//get info type
#if (HS_GET_INFO == STD_ON)
typedef enum {
    MACRO,
    ADDRESS,
} InfoType;

typedef struct {
    InfoType type;
    uint32_t value;
} Hsm_InfoCfgType;
#endif

#define LOG_MODULE_CMD_NOT_SUPPORT      (1234567)
#define LOG_MODULE_ADDR_CHECK           (100)
#define LOG_MODULE_ADDR_WRITE           (101)
#define LOG_MODULE_ADDR_READ            (102)

#define LOG_MODULE_AES_ECB_ENCRYPT                          (10)
#define LOG_MODULE_AES_ECB_DECRYPT                          (11)
#define LOG_MODULE_AES_CBC_ENCRYPT                          (12)
#define LOG_MODULE_AES_CBC_DECRYPT                          (13)
#define LOG_MODULE_AES_ECB_ENCRYPT_FOR_RAM                  (14)
#define LOG_MODULE_AES_ECB_DECRYPT_FOR_RAM                  (15)
#define LOG_MODULE_AES_CBC_ENCRYPT_FOR_RAM                  (16)
#define LOG_MODULE_AES_CBC_DECRYPT_FOR_RAM                  (17)

#define LOG_MODULE_SECURE_BOOT_FAILED_HASH_SIZE             (103)
#define LOG_MODULE_SECURE_BOOT_FAILED_HASH_RESULT_1         (104)
#define LOG_MODULE_SECURE_BOOT_FAILED_HASH_RESULT_2         (105)
#define LOG_MODULE_SECURE_BOOT_FAILED_HASH_RESULT_3         (106)
#define LOG_MODULE_SECURE_BOOT_FAILED_HASH_RESULT_4         (107)
#define LOG_MODULE_SECURE_BOOT_FAILED_HASH_RESULT_5         (108)
#define LOG_MODULE_SECURE_BOOT_FAILED_HASH_RESULT_6         (109)
#define LOG_MODULE_SECURE_BOOT_FAILED_HASH_RESULT_7         (110)
#define LOG_MODULE_CMAC_VERIFY                              (120)
#define LOG_MODULE_CMAC_GENERATE                            (121)
#define LOG_MODULE_ED25519SIGN_FAILED_INPUT                 (122)
#define LOG_MODULE_ED25519SIGN_FAILED_INPUTSIZE             (123)
#define LOG_MODULE_ED25519VER_FAILED_INPUT                  (124)
#define LOG_MODULE_ED25519VER_FAILED_INPUTSIZE              (125)
#define UINT8(num)                              ((num) & 0xff)
#define CMD_ERROR_ID(cmd, type1, type2, id)     (UINT8(cmd) << 24 | UINT8(type1) << 16 | UINT8(type2) << 8 | UINT8(id))

#define RSA_MODULUS_LEN                 256
#define RSA_EXPONENT_LEN                3

#define HSM_CRASH_DEFAULT               0xefff00FFu
#define HSM_CRASH_NOMASKABLEINT         0xefff0001u
#define HSM_CRASH_HARDFAULT             0xefff0002u
#define HSM_CRASH_MEMORYMANAGEMENT      0xefff0003u
#define HSM_CRASH_BUSFAULT              0xefff0004u
#define HSM_CRASH_USAGEFAULT            0xefff0005u
#define HSM_CRASH_SVCALL                0xefff0006u
#define HSM_CRASH_DEBUGMONITOR          0xefff0007u
#define HSM_CRASH_PENDSV                0xefff0008u
    // systick
    // time0
    // time1
    // trng
#define HSM_CRASH_BRIDGEIRQ             0xefef0013u
#define HSM_CRASH_BRIDGEERR             0xefef0014u
#define HSM_CRASH_SENSINT               0xefef0015u
#define HSM_CRASH_EXTINT                0xefef0016u
#define HSM_CRASH_PKCREADY              0xefef0017u
#define HSM_CRASH_PKCERROR              0xefef0018u

// service id
#define AES_ECB_BY_RAM                      (0xffffffff)
#define AES_ECB_SERVICE_ID                  ((HSM_CMD_AES_ECB_ENCRYPT << 8) | HSM_CMD_AES_ECB_DECRYPT)
#define AES_CBC_SERVICE_ID                  ((HSM_CMD_AES_CBC_ENCRYPT << 8) | HSM_CMD_AES_CBC_DECRYPT)
#define QUERY_ANY_INFO_SERVICE_ID           ((CMD_QUERY_INFORMATION << 8) | HSM_CMD_QUERY_ANY_INFORMATION)

// error id
#define HSM_FAILED                          1
#define HSM_SUCCESS                         (0x0)
#define AES_ECB_E_PARA_PTR_NULL             CMD_ERROR_ID(CMD_RUN_AES, HSM_CMD_AES_ECB_ENCRYPT, HSM_CMD_AES_ECB_DECRYPT, 0x1)
#define AES_ECB_E_GET_PARA_KEY_PTR          CMD_ERROR_ID(CMD_RUN_AES, HSM_CMD_AES_ECB_ENCRYPT, HSM_CMD_AES_ECB_DECRYPT, 0x2)
#define AES_ECB_E_ENCRYPT_PARA              CMD_ERROR_ID(CMD_RUN_AES, HSM_CMD_AES_ECB_ENCRYPT, 0, 0x3)
#define AES_ECB_E_ENCRYPT_KEY_PTR           CMD_ERROR_ID(CMD_RUN_AES, HSM_CMD_AES_ECB_ENCRYPT, 0, 0x4)
#define AES_ECB_E_DECRYPT_PARA              CMD_ERROR_ID(CMD_RUN_AES, HSM_CMD_AES_ECB_DECRYPT, 0, 0x5)
#define AES_ECB_E_DECRYPT_KEY_PTR           CMD_ERROR_ID(CMD_RUN_AES, HSM_CMD_AES_ECB_DECRYPT, 0, 0x6)

#define AES_CBC_E_PARA_PTR_NULL             CMD_ERROR_ID(CMD_RUN_AES, HSM_CMD_AES_CBC_ENCRYPT, HSM_CMD_AES_CBC_DECRYPT, 0x1)
#define AES_CBC_E_GET_PARA_KEY_PTR          CMD_ERROR_ID(CMD_RUN_AES, HSM_CMD_AES_CBC_ENCRYPT, HSM_CMD_AES_CBC_DECRYPT, 0x2)
#define AES_CBC_E_GET_PARA_IV_PTR           CMD_ERROR_ID(CMD_RUN_AES, HSM_CMD_AES_CBC_ENCRYPT, HSM_CMD_AES_CBC_DECRYPT, 0x3)
#define AES_CBC_E_ENCRYPT_PARA              CMD_ERROR_ID(CMD_RUN_AES, HSM_CMD_AES_CBC_ENCRYPT, 0, 0x3)
#define AES_CBC_E_ENCRYPT_KEY_PTR           CMD_ERROR_ID(CMD_RUN_AES, HSM_CMD_AES_CBC_ENCRYPT, 0, 0x4)
#define AES_CBC_E_DECRYPT_PARA              CMD_ERROR_ID(CMD_RUN_AES, HSM_CMD_AES_CBC_DECRYPT, 0, 0x5)
#define AES_CBC_E_DECRYPT_KEY_PTR           CMD_ERROR_ID(CMD_RUN_AES, HSM_CMD_AES_CBC_DECRYPT, 0, 0x6)

#define CMAC_E_PARA_PTR_NULL                CMD_ERROR_ID(CMD_RUN_CMAC, HSM_CMD_CMAC_GENERATE, HSM_CMD_CMAC_VERIFY, 0x1)

#define HASH_AES256_E_PTR_NULL              CMD_ERROR_ID(CMD_RUN_HASH, HSM_CMD_HASH_SHA_256, 0, 0x1)
#define HASH_AES256_E_VERSION               CMD_ERROR_ID(CMD_RUN_HASH, HSM_CMD_HASH_SHA_256, 0, 0x2)

#define HASH_X25519_E_PTR_NULL              CMD_ERROR_ID(CMD_RUN_HASH, 0, 0, 0x1)

#define VERSION_E_PTR_NULL                  CMD_ERROR_ID(CMD_QUERY_INFORMATION, HSM_CMD_QUERY_INFORMATION_VERSION, 0, 0x1)
#define VERSION_E_VERSION                   CMD_ERROR_ID(CMD_QUERY_INFORMATION, HSM_CMD_QUERY_INFORMATION_VERSION, 0, 0x2)

#define GET_INFO_E_PTR_NULL                 CMD_ERROR_ID(CMD_QUERY_INFORMATION, HSM_CMD_QUERY_ANY_INFORMATION, 0, 0x1)
#define GET_INFO_E_OVER_RANGE               CMD_ERROR_ID(CMD_QUERY_INFORMATION, HSM_CMD_QUERY_ANY_INFORMATION, 0, 0x2)
#define GET_INFO_E_TYPE_ERROR               CMD_ERROR_ID(CMD_QUERY_INFORMATION, HSM_CMD_QUERY_ANY_INFORMATION, 0, 0x3)
#define GET_INFO_E_FUNC_DISABLE             CMD_ERROR_ID(CMD_QUERY_INFORMATION, HSM_CMD_QUERY_ANY_INFORMATION, 0, 0x4)


#define RSA_ENCRYPT_ERROR                   CMD_ERROR_ID(CMD_RSA2048, 0, 0, 0x01)
#define RSA_DECRYPT_ERROR                   CMD_ERROR_ID(CMD_RSA2048, 0, 0, 0x02)
#define RSA_SIGNATURE_ERROR                 CMD_ERROR_ID(CMD_RSA2048, 0, 0, 0x03)
#define RSA_VERIFY_ERROR                    CMD_ERROR_ID(CMD_RSA2048, 0, 0, 0x04)
#define RSA_E_PTR_NULL                      CMD_ERROR_ID(CMD_RSA2048, 0, 0, 0x05)

#define RSA_MAX_MODULUS_BITS                2048
#define RSA_MAX_MODULUS_LEN                 ((RSA_MAX_MODULUS_BITS + 7) / 8)
#define RSA_MAX_PRIME_BITS                  ((RSA_MAX_MODULUS_BITS + 1) / 2)
#define RSA_MAX_PRIME_LEN                   ((RSA_MAX_PRIME_BITS + 7) / 8)
typedef struct {
    uint32_t ICUID;          /* [R]   ICUM ID register                                   <ICUM_CMDREG_base>+0000H */
    uint32_t ICUVER;         /* [R]   ICUM version register                              <ICUM_CMDREG_base>+0004H */
    uint8_t  dummy1[8];
    uint32_t ICU2PES0;       /* [R/W] ICUMHB internal status information register 0      <ICUM_CMDREG_base>+0010H */
    uint32_t PE2ICUS0;       /* [R]   CPU (PE) internal status information register 0    <ICUM_CMDREG_base>+0014H */
    uint32_t ACTFLAG0;       /* [R]   ICUMHB activating flag register                    <ICUM_CMDREG_base>+0018H */
    uint8_t  dummy2[36];
    uint32_t PE2ICUF0;       /* [R]   CPU (PE) interrupt request flag register 0         <ICUM_CMDREG_base>+0040H */
    uint32_t PE2ICUFS0;      /* [-]   ICUM_PE2ICUF0 set register                         <ICUM_CMDREG_base>+0044H */
    uint32_t PE2ICUFC0;      /* [W]   ICUM_PE2ICUF0 clear register                       <ICUM_CMDREG_base>+0048H */
    uint32_t PE2ICUIE0;      /* [R/W] ICUM_PE2ICUF0 enable register                      <ICUM_CMDREG_base>+004CH */
    uint32_t ICU2PEF0;       /* [R]   ICUP interrupt request flag register 0             <ICUM_CMDREG_base>+0050H */
    uint32_t ICU2PEFS0;      /* [W]   ICUM_ICU2PEF0 set register                         <ICUM_CMDREG_base>+0054H */
    uint32_t ICU2PEFC0;      /* [-]   ICUM_ICU2PEF0 clear register                       <ICUM_CMDREG_base>+0058H */
    uint32_t ICU2PEIE0;      /* [R]   ICUM_ICU2PEF0 enable register                      <ICUM_CMDREG_base>+005CH */
    uint8_t  dummy3[4];
    uint32_t PE2ICUFSA0;     /* [R/W] ICUM_PE2ICUFS0 authority control setting register  <ICUM_CMDREG_base>+0064H */
    uint8_t  dummy4[8];
    uint32_t PE2ICUSF;       /* [R]   CPU(PE) interrupt single request flag register     <ICUM_CMDREG_base>+0070H */
    uint32_t PE2ICUSFS;      /* [-]   ICUM_PE2ICUSF set register                         <ICUM_CMDREG_base>+0074H */
    uint32_t PE2ICUSFC;      /* [W]   ICUM_PE2ICUSF clear register                       <ICUM_CMDREG_base>+0078H */
    uint32_t PE2ICUSIE;      /* [R/W] ICUM_PE2ICUSF enable register                      <ICUM_CMDREG_base>+007CH */
    uint32_t ICU2PESF;       /* [R]   ICUP interrupt single request flag register        <ICUM_CMDREG_base>+0080H */
    uint32_t ICU2PESFS;      /* [W]   ICUM_ICU2PESF set register                         <ICUM_CMDREG_base>+0084H */
    uint32_t ICU2PESFC;      /* [-]   ICUM_ICU2PESF clear register                       <ICUM_CMDREG_base>+0088H */
    uint32_t ICU2PESIE;      /* [R]   ICUM_ICU2PESF enable register                      <ICUM_CMDREG_base>+008CH */
    uint32_t ICU2PESIS;      /* [R]   Interrupt of ICUM_ICU2PESF dest selection register <ICUM_CMDREG_base>+0090H */
    uint32_t PE2ICUSFSA;     /* [R]   ICUM_PE2ICUSFS authority control setting register  <ICUM_CMDREG_base>+0094H */
    uint8_t  dummy5[8];
    uint32_t SEMAPE0;        /* [R/W] CPU0 (PE0) Semaphore register                      <ICUM_CMDREG_base>+00A0H */
    uint32_t SEMAPE1;        /* [R/W] CPU1 (PE1) Semaphore register                      <ICUM_CMDREG_base>+00A4H */
    uint32_t SEMAPE2;        /* [R/W] CPU2 (PE2) Semaphore register                      <ICUM_CMDREG_base>+00A8H */
    uint32_t SEMAPE3;        /* [R/W] CPU3 (PE3) Semaphore register                      <ICUM_CMDREG_base>+00ACH */
    uint32_t SEMAPE4;        /* [R/W] CPU4 (PE4) Semaphore register                      <ICUM_CMDREG_base>+00B0H */
    uint32_t SEMAPE5;        /* [R/W] CPU5 (PE5) Semaphore register                      <ICUM_CMDREG_base>+00B4H */
    uint8_t  dummy6[72];
    uint32_t PE2ICUF1;       /* [R]   CPU (PE) interrupt request flag register 1         <ICUM_CMDREG_base>+0100H */
    uint32_t PE2ICUFS1;      /* [-]   ICUM_PE2ICUF1 set register                         <ICUM_CMDREG_base>+0104H */
    uint32_t PE2ICUFC1;      /* [W]   ICUM_PE2ICUF1 clear register                       <ICUM_CMDREG_base>+0108H */
    uint32_t PE2ICUIE1;      /* [R/W] ICUM_PE2ICUF1 enable register                      <ICUM_CMDREG_base>+010CH */
    uint32_t ICU2PEF1;       /* [R]   ICUP interrupt request flag register 1             <ICUM_CMDREG_base>+0110H */
    uint32_t ICU2PEFS1;      /* [W]   ICUM_ICU2PEF1 set register                         <ICUM_CMDREG_base>+0114H */
    uint32_t ICU2PEFC1;      /* [-]   ICUM_ICU2PEF1 clear register                       <ICUM_CMDREG_base>+0118H */
    uint32_t ICU2PEIE1;      /* [R]   ICUM_ICU2PEF1 enable register                      <ICUM_CMDREG_base>+011CH */
    uint8_t  dummy7[4];
    uint32_t PE2ICUFSA1;     /* [R/W] ICUM_PE2ICUFS1 authority control setting register  <ICUM_CMDREG_base>+0124H */
    uint32_t ICU2PES1;       /* [R/W] ICUMHB internal status information register 1      <ICUM_CMDREG_base>+0128H */
    uint32_t PE2ICUS1;       /* [R]   CPU (PE) internal status information register 1    <ICUM_CMDREG_base>+012CH */
    uint32_t PE2ICUF2;       /* [R]   CPU (PE) interrupt request flag register 2         <ICUM_CMDREG_base>+0130H */
    uint32_t PE2ICUFS2;      /* [-]   ICUM_PE2ICUF2 set register                         <ICUM_CMDREG_base>+0134H */
    uint32_t PE2ICUFC2;      /* [W]   ICUM_PE2ICUF2 clear register                       <ICUM_CMDREG_base>+0138H */
    uint32_t PE2ICUIE2;      /* [R/W] ICUM_PE2ICUF2 enable register                      <ICUM_CMDREG_base>+013CH */
    uint32_t ICU2PEF2;       /* [R]   ICUP interrupt request flag register 2             <ICUM_CMDREG_base>+0140H */
    uint32_t ICU2PEFS2;      /* [W]   ICUM_ICU2PEF2 set register                         <ICUM_CMDREG_base>+0144H */
    uint32_t ICU2PEFC2;      /* [-]   ICUM_ICU2PEF2 clear register                       <ICUM_CMDREG_base>+0148H */
    uint32_t ICU2PEIE2;      /* [R]   ICUM_ICU2PEF2 enable register                      <ICUM_CMDREG_base>+014CH */
    uint8_t  dummy8[4];
    uint32_t PE2ICUFSA2;     /* [R/W] ICUM_PE2ICUFS2 authority control setting register  <ICUM_CMDREG_base>+0154H */
    uint32_t ICU2PES2;       /* [R/W] ICUMHB internal status information register 2      <ICUM_CMDREG_base>+0158H */
    uint32_t PE2ICUS2;       /* [R]   CPU (PE) internal status information register 2    <ICUM_CMDREG_base>+015CH */
    uint32_t PE2ICUF3;       /* [R]   CPU (PE) interrupt request flag register 3         <ICUM_CMDREG_base>+0160H */
    uint32_t PE2ICUFS3;      /* [-]   ICUM_PE2ICUF3 set register                         <ICUM_CMDREG_base>+0164H */
    uint32_t PE2ICUFC3;      /* [W]   ICUM_PE2ICUF3 clear register                       <ICUM_CMDREG_base>+0168H */
    uint32_t PE2ICUIE3;      /* [R/W] ICUM_PE2ICUF3 enable register                      <ICUM_CMDREG_base>+016CH */
    uint32_t ICU2PEF3;       /* [R]   ICUP interrupt request flag register 3             <ICUM_CMDREG_base>+0170H */
    uint32_t ICU2PEFS3;      /* [W]   ICUM_ICU2PEF3 set register                         <ICUM_CMDREG_base>+0174H */
    uint32_t ICU2PEFC3;      /* [-]   ICUM_ICU2PEF3 clear register                       <ICUM_CMDREG_base>+0178H */
    uint32_t ICU2PEIE3;      /* [R]   ICUM_ICU2PEF3 enable register                      <ICUM_CMDREG_base>+017CH */
    uint8_t  dummy9[4];
    uint32_t PE2ICUFSA3;     /* [R/W] ICUM_PE2ICUFS3 authority control setting register  <ICUM_CMDREG_base>+0184H */
    uint32_t ICU2PES3;       /* [R/W] ICUMHA internal status information register 3      <ICUM_CMDREG_base>+0188H */
    uint32_t PE2ICUS3;       /* [R]   CPU (PE) internal status information register 3    <ICUM_CMDREG_base>+018CH */
    uint32_t PE2ICUF4;       /* [R]   CPU (PE) interrupt request flag register 4         <ICUM_CMDREG_base>+0190H */
    uint32_t PE2ICUFS4;      /* [-]   ICUM_PE2ICUF4 set register                         <ICUM_CMDREG_base>+0194H */
    uint32_t PE2ICUFC4;      /* [W]   ICUM_PE2ICUF4 clear register                       <ICUM_CMDREG_base>+0198H */
    uint32_t PE2ICUIE4;      /* [R/W] ICUM_PE2ICUF4 enable register                      <ICUM_CMDREG_base>+019CH */
    uint32_t ICU2PEF4;       /* [R]   ICUP interrupt request flag register 4             <ICUM_CMDREG_base>+01A0H */
    uint32_t ICU2PEFS4;      /* [W]   ICUM_ICU2PEF4 set register                         <ICUM_CMDREG_base>+01A4H */
    uint32_t ICU2PEFC4;      /* [-]   ICUM_ICU2PEF4 clear register                       <ICUM_CMDREG_base>+01A8H */
    uint32_t ICU2PEIE4;      /* [R]   ICUM_ICU2PEF4 enable register                      <ICUM_CMDREG_base>+01ACH */
    uint8_t  dummy10[4];
    uint32_t PE2ICUFSA4;     /* [R/W] ICUM_PE2ICUFS4 authority control setting register  <ICUM_CMDREG_base>+01B4H */
    uint32_t ICU2PES4;       /* [R/W] ICUMHA internal status information register 4      <ICUM_CMDREG_base>+01B8H */
    uint32_t PE2ICUS4;       /* [R]   CPU (PE) internal status information register 4    <ICUM_CMDREG_base>+01BCH */
    uint32_t PE2ICUF5;       /* [R]   CPU (PE) interrupt request flag register 5         <ICUM_CMDREG_base>+01C0H */
    uint32_t PE2ICUFS5;      /* [-]   ICUM_PE2ICUF5 set register                         <ICUM_CMDREG_base>+01C4H */
    uint32_t PE2ICUFC5;      /* [W]   ICUM_PE2ICUF5 clear register                       <ICUM_CMDREG_base>+01C8H */
    uint32_t PE2ICUIE5;      /* [R/W] ICUM_PE2ICUF5 enable register                      <ICUM_CMDREG_base>+01CCH */
    uint32_t ICU2PEF5;       /* [R]   ICUP interrupt request flag register 5             <ICUM_CMDREG_base>+01D0H */
    uint32_t ICU2PEFS5;      /* [W]   ICUM_ICU2PEF5 set register                         <ICUM_CMDREG_base>+01D4H */
    uint32_t ICU2PEFC5;      /* [-]   ICUM_ICU2PEF5 clear register                       <ICUM_CMDREG_base>+01D8H */
    uint32_t ICU2PEIE5;      /* [R]   ICUM_ICU2PEF5 enable register                      <ICUM_CMDREG_base>+01DCH */
    uint8_t  dummy11[4];
    uint32_t PE2ICUFSA5;     /* [R/W] ICUM_PE2ICUFS5 authority control setting register  <ICUM_CMDREG_base>+01E4H */
    uint32_t ICU2PES5;       /* [R/W] ICUMHA internal status information register 5      <ICUM_CMDREG_base>+01E8H */
    uint32_t PE2ICUS5;       /* [R]   CPU (PE) internal status information register 5    <ICUM_CMDREG_base>+01ECH */
    uint32_t PE2ICUF6;       /* [R]   CPU (PE) interrupt request flag register 6         <ICUM_CMDREG_base>+01F0H */
    uint32_t PE2ICUFS6;      /* [-]   ICUM_PE2ICUF6 set register                         <ICUM_CMDREG_base>+01F4H */
    uint32_t PE2ICUFC6;      /* [W]   ICUM_PE2ICUF6 clear register                       <ICUM_CMDREG_base>+01F8H */
    uint32_t PE2ICUIE6;      /* [R/W] ICUM_PE2ICUF6 enable register                      <ICUM_CMDREG_base>+01FCH */
    uint32_t ICU2PEF6;       /* [R]   ICUP interrupt request flag register 6             <ICUM_CMDREG_base>+0200H */
    uint32_t ICU2PEFS6;      /* [W]   ICUM_ICU2PEF6 set register                         <ICUM_CMDREG_base>+0204H */
    uint32_t ICU2PEFC6;      /* [-]   ICUM_ICU2PEF6 clear register                       <ICUM_CMDREG_base>+0208H */
    uint32_t ICU2PEIE6;      /* [R]   ICUM_ICU2PEF6 enable register                      <ICUM_CMDREG_base>+020CH */
    uint8_t  dummy12[4];
    uint32_t PE2ICUFSA6;     /* [R/W] ICUM_PE2ICUFS6 authority control setting register  <ICUM_CMDREG_base>+0214H */
    uint32_t ICU2PES6;       /* [R/W] ICUMHA internal status information register 6      <ICUM_CMDREG_base>+0218H */
    uint32_t PE2ICUS6;       /* [R]   CPU (PE) internal status information register 6    <ICUM_CMDREG_base>+021CH */
    uint32_t PE2ICUF7;       /* [R]   CPU (PE) interrupt request flag register 7         <ICUM_CMDREG_base>+0220H */
    uint32_t PE2ICUFS7;      /* [-]   ICUM_PE2ICUF7 set register                         <ICUM_CMDREG_base>+0224H */
    uint32_t PE2ICUFC7;      /* [W]   ICUM_PE2ICUF7 clear register                       <ICUM_CMDREG_base>+0228H */
    uint32_t PE2ICUIE7;      /* [R/W] ICUM_PE2ICUF7 enable register                      <ICUM_CMDREG_base>+022CH */
    uint32_t ICU2PEF7;       /* [R]   ICUP interrupt request flag register 7             <ICUM_CMDREG_base>+0230H */
    uint32_t ICU2PEFS7;      /* [W]   ICUM_ICU2PEF7 set register                         <ICUM_CMDREG_base>+0234H */
    uint32_t ICU2PEFC7;      /* [-]   ICUM_ICU2PEF7 clear register                       <ICUM_CMDREG_base>+0238H */
    uint32_t ICU2PEIE7;      /* [R]   ICUM_ICU2PEF7 enable register                      <ICUM_CMDREG_base>+023CH */
    uint8_t  dummy13[4];
    uint32_t PE2ICUFSA7;     /* [R/W] ICUM_PE2ICUFS7 authority control setting register  <ICUM_CMDREG_base>+0244H */
    uint32_t ICU2PES7;       /* [R/W] ICUMHA internal status information register 7      <ICUM_CMDREG_base>+0248H */
    uint32_t PE2ICUS7;       /* [R]   CPU (PE) internal status information register 7    <ICUM_CMDREG_base>+024CH */
    uint8_t  dummy14[176];
    uint32_t PE2ICUSA0;      /* [R/W] ICUM_PE2ICUS0 authority control setting register   <ICUM_CMDREG_base>+0300H */
    uint32_t PE2ICUSA1;      /* [R/W] ICUM_PE2ICUS1 authority control setting register   <ICUM_CMDREG_base>+0304H */
    uint32_t PE2ICUSA2;      /* [R/W] ICUM_PE2ICUS2 authority control setting register   <ICUM_CMDREG_base>+0308H */
    uint32_t PE2ICUSA3;      /* [R/W] ICUM_PE2ICUS3 authority control setting register   <ICUM_CMDREG_base>+030CH */
    uint32_t PE2ICUSA4;      /* [R/W] ICUM_PE2ICUS4 authority control setting register   <ICUM_CMDREG_base>+0310H */
    uint32_t PE2ICUSA5;      /* [R/W] ICUM_PE2ICUS5 authority control setting register   <ICUM_CMDREG_base>+0314H */
    uint32_t PE2ICUSA6;      /* [R/W] ICUM_PE2ICUS6 authority control setting register   <ICUM_CMDREG_base>+0318H */
    uint32_t PE2ICUSA7;      /* [R/W] ICUM_PE2ICUS7 authority control setting register   <ICUM_CMDREG_base>+031CH */
    uint8_t  dummy15[16];
    uint32_t RACSCC0;        /* [R/W] Code Flash secure region start address configuration change register 0
                                                                                         <ICUM_CMDREG_base>+0330H */
    uint32_t RACECC0;        /* [R/W] Code Flash secure region end address configuration change register 0
                                                                                         <ICUM_CMDREG_base>+0334H */
    uint32_t RACSCC1;        /* [R/W] Code Flash secure region start address configuration change register 1
                                                                                         <ICUM_CMDREG_base>+0338H */
    uint32_t RACECC1;        /* [R/W] Code Flash secure region end address configuration change register 1
                                                                                         <ICUM_CMDREG_base>+033CH */
    uint32_t CCPROT;         /* [R/W] Configuration change protection register           <ICUM_CMDREG_base>+0340H */
} REG_ICUM_CMDREG_t;


#define REG_ICUM_CMDREG_BASE (0xFF1F0000uL)

#define REG_ICUM_CMDREG (*(volatile REG_ICUM_CMDREG_t *)REG_ICUM_CMDREG_BASE)

/* PE2ICUFSA register setting value */
#define ICUM_COMM_PE2ICUFSA0_SET        (0x00000000UL)      /* PE0 */
#define ICUM_COMM_PE2ICUFSA1_SET        (0x00000001UL)      /* PE1 */
#define ICUM_COMM_PE2ICUFSA2_SET        (0x00000002UL)      /* PE2 */
                                        /*  b20-16 : IDMASK[4:0] = 11111: invalid bit of AUTHID   */
                                        /*  b4-0   : AUTHID[4:0] = SPID                           */

/* PE2ICUSA register setting value */
#define ICUM_COMM_PE2ICUSA0_SET         (0x00000000UL)      /* PE0 */
#define ICUM_COMM_PE2ICUSA1_SET         (0x00000001UL)      /* PE1 */
#define ICUM_COMM_PE2ICUSA2_SET         (0x00000002UL)      /* PE2 */
                                        /*  b20-16 : IDMASK[4:0] = 11111: invalid bit of AUTHID   */
                                        /*  b4-0   : AUTHID[4:0] = SPID                           */

/* PE2ICUSFSA register setting value */
#define ICUM_COMM_PE2ICUSFSA_SET        (0x00000000UL)
                                        /*  b20-16 : IDMASK[4:0] = 11111: invalid bit of AUTHID    */
                                        /*  b4-0   : AUTHID[4:0] = SPID                            */

/* ICUM_PE2ICUIE Register setting value */
// #define ICUM_COMM_PE2ICUIE_SET          (0xFF00007CU)
#define ICUM_COMM_PE2ICUIE_SET          (0x00000001UL)
#define ICUM_COMM_ICH_FM_SINGLE_INTERRUPT (0x00000001UL) /* Service execution request single interrupt           */
extern void u2a16icum_cmdreg_init(void);
//Helper Functions for uncached read and write to Host Address Room
void hostwrite32(uint32_t addr, uint32_t offset, uint32_t value);
void hostwrite8(uint32_t addr, uint32_t offset, uint32_t value);
uint32_t hostread32(uint32_t addr, uint32_t offset);
uint8_t hostread8(uint32_t addr, uint32_t offset);

#endif
