/*****************************************************************************************************************
 * Copyright (c) 2018-2019, Hexin (Dalian) Technology Co. Ltd All rights reserved.
 * Author  : Heyn (heyunhuan@gmail.com)
 * Version : V1.5.0
 * Web	   : http://www.hex-in.com
 * 
 * LICENSING TERMS:
 * -----------------
 * 
 * HISTORIC VERSION:
 * -----------------
 *          New Create at   2018/08/03 V1.0   [Heyn] Initialization.
 *                          2018/09/28 V1.1   [Heyn] Optimized code.
 *                          2018/10/16 V1.2.2 [Heyn] New add read mode.
 *                          2018/10/18 V1.4.0 [Heyn] New mocro HEXIN_MAGICRF_*** for trigger.
 *                          2018/10/25 V1.4.1 [Heyn] New hexin_ring_buffer_t struct. Removed #pragma pack(1)
 *                          2018/12/05 V1.5.0 [Heyn] New windows(x86) export dll method. (EXTERN_C LIBMAGICRF_API)
 *                                                   New add error code.
 * 
*****************************************************************************************************************/

#ifndef __MAGICRF_H__
#define __MAGICRF_H__

#define HEXIN_M100_BUFFER_MAX_SIZE                 (128)

#define HEXIN_TRUE                                 1
#define HEXIN_FALSE                                0

#define HEXIN_SEPARATOR                            ';'

#define HEXIN_MAGICRF_HEAD                         0xBB
#define HEXIN_MAGICRF_TAIL                         0x7E


#define HEXIN_MAGICRF_CMD_INFO                     0x03
#define HEXIN_MAGICRF_CMD_QUERY                    0x22
#define HEXIN_MAGICRF_CMD_MUL_QUERY                0x27
#define HEXIN_MAGICRF_CMD_STOP                     0x28
#define HEXIN_MAGICRF_CMD_SET_SELECT               0x0C
#define HEXIN_MAGICRF_CMD_SEND_SELECT              0x12
#define HEXIN_MAGICRF_CMD_READ_DATA                0x39
#define HEXIN_MAGICRF_CMD_WRITE_DATA               0x49
#define HEXIN_MAGICRF_CMD_LOCK                     0x82
#define HEXIN_MAGICRF_CMD_KILL                     0x65
#define HEXIN_MAGICRF_CMD_GET_QUERY                0x0D
#define HEXIN_MAGICRF_CMD_SET_QUERY                0x0E
#define HEXIN_MAGICRF_CMD_SET_REGION               0x07
#define HEXIN_MAGICRF_CMD_INSERT_RF_CHANNEL        0xA9     // @see UHF RFID Reader App v2.1 source code.
#define HEXIN_MAGICRF_CMD_SET_RF_CHANNEL           0xAB
#define HEXIN_MAGICRF_CMD_GET_RF_CHANNEL           0xAA
#define HEXIN_MAGICRF_CMD_SET_HFSS                 0xAD
#define HEXIN_MAGICRF_CMD_GET_RF_POWER             0xB7
#define HEXIN_MAGICRF_CMD_SET_RF_POWER             0xB6
#define HEXIN_MAGICRF_CMD_SET_RF_CARRIER           0xB0
#define HEXIN_MAGICRF_CMD_GET_RF_GAIN              0xF1
#define HEXIN_MAGICRF_CMD_SET_RF_GAIN              0xF0
#define HEXIN_MAGICRF_CMD_TEST_SCANJAMMER          0xF2
#define HEXIN_MAGICRF_CMD_TEST_RSSI                0xF3
#define HEXIN_MAGICRF_CMD_SET_MODE                 0xF5     // @see UHF RFID Reader App v2.1 source code.
#define HEXIN_MAGICRF_CMD_CTRL_IO                  0x1A
#define HEXIN_MAGICRF_CMD_DEEP_SLEEP               0x17
#define HEXIN_MAGICRF_CMD_DEEPSLEEP_TIME           0x1D
#define HEXIN_MAGICRF_CMD_IDLE                     0x04
#define HEXIN_MAGICRF_CMD_ERROR                    0xFF

/* For return. */
#define HEXIN_ERROR                                ( 0x00000000 )

#define HEXIN_MAGICRF_BASE                         ( 0x00000001 )
#define HEXIN_MAGICRF_INFO                         ( HEXIN_MAGICRF_BASE <<  0 )
#define HEXIN_MAGICRF_QUERY                        ( HEXIN_MAGICRF_BASE <<  1 )
#define HEXIN_MAGICRF_MUL_QUERY                    ( HEXIN_MAGICRF_BASE <<  2 )
#define HEXIN_MAGICRF_STOP                         ( HEXIN_MAGICRF_BASE <<  3 )
#define HEXIN_MAGICRF_SET_SELECT                   ( HEXIN_MAGICRF_BASE <<  4 )
#define HEXIN_MAGICRF_SEND_SELECT                  ( HEXIN_MAGICRF_BASE <<  5 )
#define HEXIN_MAGICRF_READ_DATA                    ( HEXIN_MAGICRF_BASE <<  6 )
#define HEXIN_MAGICRF_WRITE_DATA                   ( HEXIN_MAGICRF_BASE <<  7 )
#define HEXIN_MAGICRF_LOCK                         ( HEXIN_MAGICRF_BASE <<  8 )
#define HEXIN_MAGICRF_KILL                         ( HEXIN_MAGICRF_BASE <<  9 )
#define HEXIN_MAGICRF_GET_QUERY                    ( HEXIN_MAGICRF_BASE << 10 )
#define HEXIN_MAGICRF_SET_QUERY                    ( HEXIN_MAGICRF_BASE << 11 )
#define HEXIN_MAGICRF_SET_REGION                   ( HEXIN_MAGICRF_BASE << 12 )
#define HEXIN_MAGICRF_INSERT_RF_CHANNEL            ( HEXIN_MAGICRF_BASE << 13 )
#define HEXIN_MAGICRF_SET_RF_CHANNEL               ( HEXIN_MAGICRF_BASE << 14 )
#define HEXIN_MAGICRF_GET_RF_CHANNEL               ( HEXIN_MAGICRF_BASE << 15 )
#define HEXIN_MAGICRF_SET_HFSS                     ( HEXIN_MAGICRF_BASE << 16 )
#define HEXIN_MAGICRF_GET_RF_POWER                 ( HEXIN_MAGICRF_BASE << 17 )
#define HEXIN_MAGICRF_SET_RF_POWER                 ( HEXIN_MAGICRF_BASE << 18 )
#define HEXIN_MAGICRF_SET_RF_CARRIER               ( HEXIN_MAGICRF_BASE << 19 )
#define HEXIN_MAGICRF_GET_RF_GAIN                  ( HEXIN_MAGICRF_BASE << 20 )
#define HEXIN_MAGICRF_SET_RF_GAIN                  ( HEXIN_MAGICRF_BASE << 21 )
#define HEXIN_MAGICRF_TEST_SCANJAMMER              ( HEXIN_MAGICRF_BASE << 22 )
#define HEXIN_MAGICRF_TEST_RSSI                    ( HEXIN_MAGICRF_BASE << 23 )
#define HEXIN_MAGICRF_SET_MODE                     ( HEXIN_MAGICRF_BASE << 24 )
#define HEXIN_MAGICRF_CTRL_IO                      ( HEXIN_MAGICRF_BASE << 25 )
#define HEXIN_MAGICRF_DEEP_SLEEP                   ( HEXIN_MAGICRF_BASE << 26 )
#define HEXIN_MAGICRF_DEEPSLEEP_TIME               ( HEXIN_MAGICRF_BASE << 27 )
#define HEXIN_MAGICRF_IDLE                         ( HEXIN_MAGICRF_BASE << 28 )

#define HEXIN_MAGICRF_NOTHING                      ( HEXIN_MAGICRF_BASE << 30 )
#define HEXIN_MAGICRF_ERROR                        ( 0x10000000 )

/* Error Code. */
typedef enum {
    ERROR_OTHER         = 0x00,     /* Other error. */
    MEMORY_OVERRUN      = 0x03,     /* The tag memory area does not exist; or the label does not support the specified length of EPC. */
    MEMORY_LOCKED       = 0x04,     /* The tag data store is locked, and the lock status is not writable or unreadable.     */
    INSUFFICIENT_POWER  = 0x0B,     /* The tag does not receive enough power to write. */
    NON_SPECIFIC        = 0x0F
} tag_error_code_t;

#define HEXIN_FAIL_READ                            ( 0x09 )
#define HEXIN_ERROR_READ                           ( 0xA0 )
#define HEXIN_ERROR_READ_NON_SPECIFIC              ( HEXIN_ERROR_READ | NON_SPECIFIC   )
#define HEXIN_ERROR_READ_MEMORY_LOCKED             ( HEXIN_ERROR_READ | MEMORY_LOCKED  )
#define HEXIN_ERROR_READ_MEMORY_OVERRUN            ( HEXIN_ERROR_READ | MEMORY_OVERRUN )
#define HEXIN_ERROR_READ_INSUFFICIENT_POWER        ( HEXIN_ERROR_READ | INSUFFICIENT_POWER )

#define HEXIN_FAIL_WRITE                           ( 0x10 )
#define HEXIN_ERROR_WRITE                          ( 0xB0 )
#define HEXIN_ERROR_WRITE_NON_SPECIFIC             ( HEXIN_ERROR_WRITE | NON_SPECIFIC   )
#define HEXIN_ERROR_WRITE_MEMORY_LOCKED            ( HEXIN_ERROR_WRITE | MEMORY_LOCKED  )
#define HEXIN_ERROR_WRITE_MEMORY_OVERRUN           ( HEXIN_ERROR_WRITE | MEMORY_OVERRUN )
#define HEXIN_ERROR_WRITE_INSUFFICIENT_POWER       ( HEXIN_ERROR_WRITE | INSUFFICIENT_POWER )

#define HEXIN_FAIL_LOCK                            ( 0x13 )
#define HEXIN_ERROR_LOCK                           ( 0xC0 )
#define HEXIN_ERROR_LOCK_NON_SPECIFIC              ( HEXIN_ERROR_LOCK | NON_SPECIFIC   )
#define HEXIN_ERROR_LOCK_MEMORY_LOCKED             ( HEXIN_ERROR_LOCK | MEMORY_LOCKED  )
#define HEXIN_ERROR_LOCK_MEMORY_OVERRUN            ( HEXIN_ERROR_LOCK | MEMORY_OVERRUN )
#define HEXIN_ERROR_LOCK_INSUFFICIENT_POWER        ( HEXIN_ERROR_LOCK | INSUFFICIENT_POWER )

#define HEXIN_FAIL_KILL                            ( 0x12 )
#define HEXIN_ERROR_KILL                           ( 0xD0 )
#define HEXIN_ERROR_KILL_NON_SPECIFIC              ( HEXIN_ERROR_KILL | NON_SPECIFIC   )
#define HEXIN_ERROR_KILL_MEMORY_LOCKED             ( HEXIN_ERROR_KILL | MEMORY_LOCKED  )
#define HEXIN_ERROR_KILL_MEMORY_OVERRUN            ( HEXIN_ERROR_KILL | MEMORY_OVERRUN )
#define HEXIN_ERROR_KILL_INSUFFICIENT_POWER        ( HEXIN_ERROR_KILL | INSUFFICIENT_POWER )

#define HEXIN_FAIL_BPL                             ( 0x14 ) /* BlockPermalock */
#define HEXIN_ERROR_BPL                            ( 0xE0 )
#define HEXIN_ERROR_BPL_NON_SPECIFIC               ( HEXIN_ERROR_BPL | NON_SPECIFIC   )
#define HEXIN_ERROR_BPL_MEMORY_LOCKED              ( HEXIN_ERROR_BPL | MEMORY_LOCKED  )
#define HEXIN_ERROR_BPL_MEMORY_OVERRUN             ( HEXIN_ERROR_BPL | MEMORY_OVERRUN )
#define HEXIN_ERROR_BPL_INSUFFICIENT_POWER         ( HEXIN_ERROR_BPL | INSUFFICIENT_POWER )

#define NXP_G2X_ERROR_CHANGE_CONFIG                ( 0x1A )
#define NXP_G2X_ERROR_READ_PROTECT                 ( 0x2A )
#define NXP_G2X_ERROR_RESET_PROTECT                ( 0x2B )
#define NXP_G2X_ERROR_CHANGE_EAS                   ( 0x1B )
#define NXP_G2X_ERROR_EAS_ALARM                    ( 0x1D )
#define NXP_G2X_ERROR_SPECIAL                      ( 0xE0 )

// NXP g2x or Impinj Monza Q
#define NXP_G2X_ERROR_SPECIAL_NON_SPECIFIC         ( HEXIN_ERROR_BPL | NON_SPECIFIC )
#define NXP_G2X_ERROR_SPECIAL_MEMORY_LOCKED        ( HEXIN_ERROR_BPL | MEMORY_LOCKED )
#define NXP_G2X_ERROR_SPECIAL_MEMORY_OVERRUN       ( HEXIN_ERROR_BPL | MEMORY_OVERRUN )
#define NXP_G2X_ERROR_SPECIAL_INSUFFICIENT_POWER   ( HEXIN_ERROR_BPL | INSUFFICIENT_POWER )

#define HEXIN_ERROR_INVENTORY                      ( 0x15 )
#define HEXIN_ERROR_ACCESS                         ( 0x16 )
#define HEXIN_ERROR_COMMAND                        ( 0x17 )
#define HEXIN_ERROR_FHSS                           ( 0x20 )

/******************************************************
 *                      Enumerations
 ******************************************************/

typedef enum {
    TYPE_COMMAND = 0x00,    /* Command . host to m100 */
    TYPE_RESPONSE,          /* Response. m100 to host */
    TYPE_NOTICE             /* Notice  . m100 to host */
} module_packet_type_t;

typedef enum {
    BANK_RFU = 0x00,
    BANK_EPC,
    BANK_TID,
    BANK_USER,
    BANK_MAX
} module_memory_bank_t;

typedef enum {
    REGION_CHINA_900 = 0x01,    //freq = 920.125 ~ 924.875MHz (China 900MHz)  Step:0.25MHz
    REGION_AMERICA,             //freq = 902.250 ~ 927.750MHz (US)            Step:0.5MHz
    REGION_EUROPE,              //freq = 865.100 ~ 867.900MHz (Europe)        Step:0.2MHz
    REGION_CHINA_800,           //freq = 840.125 ~ 844.875MHz (China 800MHz)  Step:0.25MHz
    REGION_RESERVED1,           //Reserved.
    REGION_KOREA,               //freq = 917.100 ~ 923.300MHz (Korea)         Step:0.2MHz
    REGION_MAX
} module_region_t;

typedef enum {
    MIXER_GAIN_0DB = 0x00,
    MIXER_GAIN_3DB,
    MIXER_GAIN_6DB,
    MIXER_GAIN_9DB,
    MIXER_GAIN_12DB,
    MIXER_GAIN_15DB,
    MIXER_GAIN_16DB,
    MIXER_GAIN_MAX
} module_mixer_gain_t;

typedef enum {
    IF_GAIN_12DB = 0x00,
    IF_GAIN_18DB,
    IF_GAIN_21DB,
    IF_GAIN_24DB,
    IF_GAIN_27DB,
    IF_GAIN_30DB,
    IF_GAIN_36DB,
    IF_GAIN_40DB,
    IF_GAIN_MAX
} module_if_gain_t;   /* Intermediate frequency gain. */

typedef enum {
    GPIO_PIN_1 = 0x01,
    GPIO_PIN_2,
    GPIO_PIN_3,
    GPIO_PIN_4,
    GPIO_PIN_MAX
} module_gpio_pin_t;

typedef enum {
    GPIO_TYPE_INIT = 0x00,  /* Init GPIO direction  */
    GPIO_TYPE_WRITE,        /* Write value to GPIO  */
    GPIO_TYPE_READ,         /* Read value from GPIO */
    GPIO_TYPE_MAX
} module_gpio_type_t;

typedef enum {
    HEAD_OFFSET = 0x00,
    TYPE_OFFSET,
    COMMAND_OFFSET,
    LENGTH_MSB_OFFSET,
    LENGTH_LSB_OFFSET,
    PAYLOAD_OFFSET
} request_packet_offset_t;

/******************************************************
 *               Type Definitions
 ******************************************************/
// #pragma pack(1)
typedef struct {
    unsigned char  head;         /* MagicRF M100/QM100 frame head is 0xBB */
    unsigned char  type;         /* module_packet_type_t    */
    unsigned char  command;      /* Packet command.         */
    unsigned short length;       /* Parameter length.       */
    unsigned char  *payload;     /* Parameter.              */
    unsigned char  checksum;     /* From type to payload.   */
    unsigned char  tail;         /* MagicRF M100/QM100 frame tail is 0x7E */
} request_packet_t;


typedef struct {
    unsigned char          buffer[HEXIN_M100_BUFFER_MAX_SIZE];
    volatile unsigned int  offset;
    volatile unsigned char in_packet;
} packet_handler_t;

/****************************************************************************************************************/

#ifndef HEXIN_MIN
#define HEXIN_MIN( x, y )                       ( x < y ? x : y )
#endif

typedef struct
{
    unsigned char           *buffer;
    unsigned int            size;
    volatile unsigned int   head; /* Index of the buffer to read from */
    volatile unsigned int   tail; /* Index of the buffer to write to */
} hexin_ring_buffer_t;

unsigned int hexinRingBufferInit        ( hexin_ring_buffer_t* ring_buffer,
                                          unsigned char* buffer,
                                          unsigned int   buffer_size );

unsigned int hexinRingBufferWrite       ( hexin_ring_buffer_t* ring_buffer,
                                          const unsigned char* data,
                                          unsigned int data_length );

void hexinRingBufferRead                ( hexin_ring_buffer_t* ring_buffer,
                                          unsigned char* data,
                                          unsigned int   data_length,
                                          unsigned int*  number_of_bytes_read );

unsigned int hexinRingBufferUsedSpace   ( hexin_ring_buffer_t* ring_buffer );
unsigned int hexinRingBufferFreeSpace   ( hexin_ring_buffer_t* ring_buffer );

/****************************************************************************************************************/

/* magicRF prints */
// #define HPRINT_ENABLE_MAGICRF_INFO
// #define HPRINT_ENABLE_MAGICRF_DEBUG
// #define HPRINT_ENABLE_MAGICRF_ERROR

/* MAFICRF printing macros for dongle server */
#ifdef HPRINT_ENABLE_MAGICRF_INFO
    #define HPRINT_MAGICRF_INFO(args)    do { printf args; } while( 0==1 )
#else
    #define HPRINT_MAGICRF_INFO(args)
#endif

#ifdef HPRINT_ENABLE_MAGICRF_DEBUG
    #define HPRINT_MAGICRF_DEBUG(args)   do { printf args; } while( 0==1 )
#else
    #define HPRINT_MAGICRF_DEBUG(args)
#endif

#ifdef HPRINT_ENABLE_MAGICRF_ERROR
    #define HPRINT_MAGICRF_ERROR(args)   do { printf args; } while( 0==1 )
#else
    #define HPRINT_MAGICRF_ERROR(args)
#endif

#if defined( _WIN32 )
    #define LIBMAGICRF_API      __declspec(dllexport)
    #ifdef __cplusplus
        #define EXTERN_C        extern "C"
    #else
        #define EXTERN_C        extern
    #endif
#else
    #define LIBMAGICRF_API
    #define EXTERN_C            extern
#endif

EXTERN_C LIBMAGICRF_API unsigned int version    (       unsigned char param,  unsigned char *pbuf );
EXTERN_C LIBMAGICRF_API unsigned int query      (       unsigned short loop , unsigned char *pbuf );
EXTERN_C LIBMAGICRF_API unsigned int kill       ( const unsigned char *pwd,   unsigned char *pbuf );
EXTERN_C LIBMAGICRF_API unsigned int getPaPower (       unsigned char *pbuf );
EXTERN_C LIBMAGICRF_API unsigned int setPaPower (               float power,  unsigned char *pbuf );
EXTERN_C LIBMAGICRF_API unsigned int setMode    (       unsigned char  mode,  unsigned char *pbuf );
EXTERN_C LIBMAGICRF_API unsigned int stop       (                             unsigned char *pbuf );
EXTERN_C LIBMAGICRF_API unsigned int deepSleep  (                             unsigned char *pbuf );
EXTERN_C LIBMAGICRF_API unsigned int testRSSI   (                             unsigned char *pbuf );
EXTERN_C LIBMAGICRF_API unsigned int scanJammer (                             unsigned char *pbuf );
EXTERN_C LIBMAGICRF_API unsigned int setHFSS    (       unsigned char status, unsigned char *pbuf );
EXTERN_C LIBMAGICRF_API unsigned int setRegion  (     module_region_t region, unsigned char *pbuf );

EXTERN_C LIBMAGICRF_API unsigned int deepSleepTime  ( unsigned char minute,   unsigned char *pbuf );
EXTERN_C LIBMAGICRF_API unsigned int setRFCarrier   ( unsigned char status,   unsigned char *pbuf );
EXTERN_C LIBMAGICRF_API unsigned int setRFChannel   ( unsigned char index,    unsigned char *pbuf );
EXTERN_C LIBMAGICRF_API unsigned int setSelectMode  ( unsigned char mode,     unsigned char *pbuf );

EXTERN_C LIBMAGICRF_API unsigned int getRevDemodulatorParam( unsigned char *pbuf );
EXTERN_C LIBMAGICRF_API unsigned int setRevDemodulatorParam( module_mixer_gain_t mixer,     /* Mixer gain.  */
                                                             module_if_gain_t ifgain,       /* Intermediate frequency AMP gain. */
                                                             unsigned short thrd,           /* Threshold.   */
                                                             unsigned char *pbuf );         /* Output */
EXTERN_C LIBMAGICRF_API
unsigned int idle           ( unsigned char mode,
                              unsigned char minute,
                              unsigned char *pbuf );

EXTERN_C LIBMAGICRF_API
unsigned int lock           ( const unsigned char *pwd,
                              const unsigned char *ld,
                              unsigned char *pbuf );

EXTERN_C LIBMAGICRF_API
unsigned int insertRFChannel( unsigned char start,
                              unsigned char stop,
                              unsigned char *pbuf );

EXTERN_C LIBMAGICRF_API
unsigned int setQueryParam  ( unsigned char select,
                              unsigned char session,
                              unsigned char target,
                              unsigned char qvalue,
                              unsigned char *pbuf );

EXTERN_C LIBMAGICRF_API
unsigned int setSelectParam(  unsigned char select,      /* Target(3bit) + Action(3bit) + Membank(2bit) */
                              unsigned int  ptr,         /* 4Bytes */
                              const    char *mask,
                              unsigned char maskLen,
                              unsigned char maskflag,    /* Mask format. 0: Hex format; 1: String format. */
                              unsigned char truncate,
                              unsigned char *pbuf );

EXTERN_C LIBMAGICRF_API
unsigned int readData       ( const unsigned char *pwd,    /* Access password. */
                              unsigned char pwdflag,       /* Password format. 0: Hex format; 1: String format. */
                              module_memory_bank_t bank,   /* Memory bank. */
                              unsigned short sa,           /* Data address offset. */
                              unsigned short dl,           /* Data length. */
                              unsigned char *pbuf /*Output*/);

EXTERN_C LIBMAGICRF_API
unsigned int writeData      ( const unsigned char *pwd,   /* Access password. */
                              unsigned char pwdflag,      /* Password format. 0: Hex format; 1: String format. */
                              module_memory_bank_t bank,  /* Memory bank. */
                              unsigned short sa,          /* Data address offset. */
                              unsigned short dl,          /* Data length. */
                              unsigned char *dt,          /* Write data buffer. */
                              unsigned char *pbuf );

EXTERN_C LIBMAGICRF_API
unsigned int writeEPC       ( const unsigned char *pwd,   /* Access password. */
                              unsigned char pwdflag,      /* Password format. 0: Hex format; 1: String format. */
                              unsigned char *dt,          /* Write data buffer. */
                              unsigned short dl,          /* Data length. */
                              unsigned char *pbuf );

EXTERN_C LIBMAGICRF_API
unsigned int unpackFrame    ( unsigned char *frame, unsigned char *param, unsigned int *length );

EXTERN_C LIBMAGICRF_API
unsigned int packetHandler  ( hexin_ring_buffer_t *ringbuffer,
                              unsigned int trigger,
                              const unsigned char *frame,
                              unsigned int frame_length,
                              void (*callback)( unsigned char *, unsigned int ) );
#endif /* ifndef __MAGICRF_H__ */
