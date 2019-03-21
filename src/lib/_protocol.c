/*****************************************************************************************************************
 *
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
 *          New Create at   2018/08/03 V0.1   [Heyn] Initialization.
 *                          2018/08/13 V0.2   [Heyn] New deepSleep() idelTime() idel().
 *                          2018/09/21 V0.3   [Heyn] Optimized code.
 *                          2018/09/28 V0.4   [Heyn] Optimized code for cross compile.
 *                          2018/09/29 V1.0   [Heyn] Bugfix: dangerous relocation: call8: call target out of range: calloc
 *                                                   call target out of range: memcpy / call target out of range: free.
 *                          2018/10/09 V1.1   [Heyn] Modify setRFPower's power type (uint8_t -> float)
 *                                                   Rename setRFPower to setPaPower / Rename getRFPower to getPaPower
 *                          2018/10/11 V1.1   [Heyn] Get valid epc data and new crc16-ccitt checksum.
 *                          2018/10/12 V1.2.0 [Heyn] Modify setSelectParam/readData/writeData interface.
 *                          2018/10/15 V1.2.1 [Heyn] New add writeEPC(...) function.
 *                          2018/10/16 V1.3.0 [Heyn] New add setRSSIParameter(...), fixed rssi value.
 *                          2018/10/18 V1.4.0 [Heyn] Modify version() and gpio() function.
 *                                                   New add insertRFChannel() function. Optimized unpackFrame() function.
 *                          2018/10/25 V1.4.1 [Heyn] Fixed ( warning C4018: "<"). New add packetHandler() and RingBuffer function.
 *                          2018/10/29 V1.4.2 [Heyn] Optimized code
 *                          2018/12/05 V1.5.0 [Heyn] New windows(x86) export dll method. (EXTERN_C LIBMAGICRF_API)
 * 
*****************************************************************************************************************/

/*
    ::clang build windows dll method.
    ::x86:
    clang -c -o _protocol.o _protocol.c -target i686-pc-windows-msvc
    clang -shared -v -o libmagicrf.dll _protocol.o -target i686-pc-windows-msvc
    ::x86-64:
    clang -c -o _protocol.o _protocol.c -target x86_64-pc-windows-gnu
    clang -shared -v -o libmagicrf.dll _protocol.o -target x86_64-pc-windows-gnu
*/

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "_protocol.h"

static char __antennaGain    =   3;
static char __coupling       = -20;

#define HEXIN_USHORT_MSB(x)             ((unsigned char)((x) >> 8) & 0xFF)
#define HEXIN_USHORT_LSB(x)             ((unsigned char)((x) >> 0) & 0xFF)

#define HEXIN_UCHAR2USHORT(msb, lsb)    ((((unsigned short)msb << 8) & 0xFF00) | \
                                         (((unsigned short)lsb << 0) & 0x00FF))

#define HEXIN_EPC_DATA_MAX_SIZE         (128)


static unsigned int   __chk_ecp_crc16 ( const unsigned char *pbuf, unsigned int len, unsigned short crc16 );
static unsigned short __get_ecp_crc16 ( const unsigned char *pbuf, unsigned int len );


static unsigned int __hex2str( unsigned char *dest, const unsigned char *src, unsigned int len )
{
    unsigned int  i   = 0x00;
    unsigned char msb = 0x00, lsb = 0x00;

    for ( i=0; i<len; i++ ) {
        msb = ((src[i] >> 4) & 0x0F);
        lsb = ((src[i] >> 0) & 0x0F);

        msb += 0x30;
        dest[ (i<<1) + 0]  = ((msb > 0x39 )? (msb + 0x07) : msb);
        lsb += 0x30;
        dest[ (i<<1) + 1 ] = ((lsb > 0x39 )? (lsb + 0x07) : lsb);
    }

    return ( len * 2 );
}

static unsigned int __str2hex( unsigned char *dest, const unsigned char *src, unsigned int len )
{
    unsigned int i    = 0x00;
    unsigned char msb = 0x00, lsb = 0x00;

    for ( i=0; i<len; i += 2 ) {
        msb  = toupper(src[i + 0]);
        lsb  = toupper(src[i + 1]);
        msb -= ((msb > 0x39) ? 0x37 : 0x30);
        lsb -= ((lsb > 0x39) ? 0x37 : 0x30);

        dest[i/2] = (( msb << 4 ) & 0xF0) | ( lsb & 0x0F );
    }

    return ( len / 2 );
}

/**
 * @brief Calculated data checksum.
 *
 * @param [in] request_packet_t
 * 
 * @retval sum
 * @see None.
 */
static unsigned char __checksum( const request_packet_t *packet )
{
    unsigned int  i   = 0x00;
    unsigned char sum = 0x00;

    sum += packet->type;
    sum += packet->command;
    sum += HEXIN_USHORT_MSB(packet->length);
    sum += HEXIN_USHORT_LSB(packet->length);

    for ( i=0; i<packet->length; i++ ) {
        sum += packet->payload[i];
    }
    
    return (sum & 0xFF);
}

inline static void __pack( request_packet_t *packet, unsigned char command, unsigned char *param, unsigned int length )
{
    packet->head     = HEXIN_MAGICRF_HEAD;
    packet->type     = TYPE_COMMAND;
    packet->command  = command;
    packet->length   = length;  /* Parameter length. */
    packet->payload  = param;
    packet->checksum = __checksum( packet );
    packet->tail     = HEXIN_MAGICRF_TAIL;
}

static unsigned int __packFrame( const request_packet_t *packet, unsigned char *pbuf )
{
    unsigned short i = 0x00;

    pbuf[i++] = packet->head;
    pbuf[i++] = packet->type;
    pbuf[i++] = packet->command;
    pbuf[i++] = HEXIN_USHORT_MSB(packet->length);
    pbuf[i++] = HEXIN_USHORT_LSB(packet->length);

    for ( ; i<(packet->length + 5); i++ ) { // Fixed: 20181025 warning C4018: "<"
        pbuf[i] = packet->payload[i - 5];
    }

    pbuf[i++] = packet->checksum;
    pbuf[i++] = packet->tail;

    return (packet->length + 7);
}

static unsigned int __makeFrame( unsigned char command, unsigned char *param, unsigned int length, unsigned char *pbuf )
{
    request_packet_t packet = { .head     = HEXIN_MAGICRF_HEAD,
                                .type     = TYPE_COMMAND,
                                .command  = 0x00,
                                .length   = 0x00,
                                .payload  = NULL,
                                .checksum = 0x00,
                                .tail     = HEXIN_MAGICRF_TAIL };
    /* Check parameters. */
    if ( ((NULL == param) && (length != 0x00)) || (NULL == pbuf) ) {
        return 0;
    }

    __pack( &packet, command, param, length );

    return __packFrame( &packet, pbuf );
}

void setRSSIParameter( unsigned char hardwareVersion )
{
    switch ( hardwareVersion ) {
        case 0: // M100 26dBm
            __antennaGain =   3;
            __coupling    = -20;
            break;

        case 1: // M100 20dBm
            __antennaGain =   1;
            __coupling    = -27;
            break;
        case 2: // M100 30dBm
            __antennaGain =   3;
            __coupling    = -10;
            break;

        case 3: // M100
            __antennaGain =   4;
            __coupling    = -10;
            break;
        default:
            break;
    }
}

/**
 * @brief Module version.
 *
 * @param [in]  mode    : 0x00 : Hardware version. 0x01 : Software version. 0x02 : Menufacturer.
 * @param [out] pbuf    : Frame buffer.
 * 
 * @retval *pbuf length
 * @see None.
 */

unsigned int version( unsigned char param, unsigned char *pbuf )
{
    if ( param > 2 ) {
        return 0x00;
    }
    return __makeFrame( HEXIN_MAGICRF_CMD_INFO, &param, 1, pbuf );
}

unsigned int query( unsigned short loop , unsigned char *pbuf )
{
    unsigned int length = 0;

    if (( loop <= 0 ) || ( loop > 65535 ) || ( NULL == pbuf )) {
        return length;
    }

    switch ( loop ) {
        case 1:
            {
                unsigned char i = 0;
                const unsigned char buffer[7] = { HEXIN_MAGICRF_HEAD,
                                          TYPE_COMMAND,
                                          HEXIN_MAGICRF_CMD_QUERY,
                                          0x00, 0x00,
                                          0x22, /* Checksum */
                                          HEXIN_MAGICRF_TAIL };
                for ( i=0; i<7; i++ ) {
                    pbuf[i] = buffer[i];
                }
                length = i;
            }
            break;

        default:
            {
                unsigned char param[3] = { 0x00 };

                param[0] = 0x22;    /* Reserved */
                param[1] = HEXIN_USHORT_MSB(loop);
                param[2] = HEXIN_USHORT_LSB(loop);

                length = __makeFrame( HEXIN_MAGICRF_CMD_MUL_QUERY, param, 3, pbuf );
            }
            break;
    }
    return length;
}

unsigned int stop( unsigned char *pbuf )
{
    return __makeFrame( HEXIN_MAGICRF_CMD_STOP, NULL, 0, pbuf );
}

/**
 * @brief   Set Select Paramters.
 *
 * @param   [in] select and so on
 * 
 * @retval  length
 * @see     2018-10-12 [Heyn] New add maskflag paramter.
 */

unsigned int setSelectParam( unsigned char select,      /* Target(3bit) + Action(3bit) + Membank(2bit) */
                             unsigned int  ptr,         /* 4Bytes */
                             const    char *mask,
                             unsigned char maskLen,
                             unsigned char maskflag,    /* Mask format. 0: Hex format; 1: String format. */
                             unsigned char truncate,
                             unsigned char *pbuf )
{
    unsigned int  i         = 0x00;
    unsigned int  length    = 0x00;
    unsigned int  maxsize   = maskLen + 7;
    unsigned char flag      = (maskflag == 0) ? 0x00 : 0x01;

    //[ERROR] dangerous relocation: call8: call target out of range: memcpy (line:188)
    volatile unsigned char param[HEXIN_EPC_DATA_MAX_SIZE];

    if ( (NULL == mask) || (maskLen == 0x00) || (maxsize > HEXIN_EPC_DATA_MAX_SIZE) || (NULL == pbuf)) {
        return 0;
    }

    if ( (select<0) || (select>=BANK_MAX) ) {
        return 0;
    }

    /*
        Target(3bit)  -> See EPC Gen2 protocol.
                         000 -> Session0    001 -> Session1
                         010 -> Session2    011 -> Session3
                         100 -> SL          101 -> RFU
                         110 -> RFU         111 -> RFU
        Action(3bit)  -> See EPC Gen2 protocol.
                         000 -> Sure SL flag or Session(x)->A
                         001 -> Sure SL flag or Session(x)->A
                         010 -> None
                         011 -> Not sure SL flag or Session(x)->A
        Membank(2bit) -> BANK_RFU=0x00 / BANK_EPC=0x01 / BANK_TID=0x02 / BANK_USER=0x03
    */
    param[0] = select;

    for ( i=0; i<4; i++ ) {
        param[1 + i] = ((unsigned char)(ptr >> ( 24 - 8*i )) & 0xFF);
    }

    param[5] = (unsigned char)(maskLen << 3);
    param[6] = ((0x00 == truncate) ? 0x00 : 0x80);  // 0x00 : Disable truncate; 0x80 : Enable truncate.

    switch ( flag ) {
        case 0: /* mask is hex */
            for ( i=0; i<maskLen; i++ ) {
                param[7 + i] = mask[i];
            }
            break;
            
        case 1: /* mask is string. */
            length   = __str2hex( (unsigned char *)(param + 7), (unsigned char *)mask, maskLen );
            maxsize  = length + 7;                      /* Redefine maxsize.*/
            param[5] = (unsigned char)(length << 3);
        default:
            break;
    }

    length = __makeFrame( HEXIN_MAGICRF_CMD_SET_SELECT, (unsigned char *)param, maxsize, pbuf );

    return length;
}

/**
 * @brief Setting select mode.
 *
 * @param [in]  mode    [0x00, 0x01, 0x02]
 * @param [out] pbuf    Frame buffer.
 * 
 * @retval *pbuf length
 * @see None.
 */
unsigned int setSelectMode( unsigned char mode, unsigned char *pbuf )
{
    if ( (mode < 0) || (mode > 2) ) {
        return 0;
    }

    return __makeFrame( HEXIN_MAGICRF_CMD_SEND_SELECT, &mode, 1, pbuf );
}

unsigned int readData( const unsigned char *pwd,    /* Access password. */
                       unsigned char pwdflag,       /* Password format. 0: Hex format; 1: String format. */
                       module_memory_bank_t bank,   /* Memory bank. */
                       unsigned short sa,           /* Data address offset. */
                       unsigned short dl,           /* Data length. */
                       unsigned char *pbuf /*Output*/)
{
    unsigned char offset   = 0x00;
    unsigned char param[9] = { 0x00 };
    unsigned char flag     = (pwdflag == 0) ? 0x00 : 0x01;

    if ( (NULL == pwd) || (NULL == pbuf) || ( bank < 0 ) || (bank >= BANK_MAX ) ) {
        return 0;
    }

    switch ( flag ) {
        case 0: /* password is hex */
            for ( offset=0; offset<4; offset++ ) {
                param[offset] = pwd[offset];
            }
            break;

        case 1: /* password is string. */
            offset = __str2hex( (unsigned char *)param, (unsigned char *)pwd, 8 );
        default:
            break;
    }

    param[offset++] = (unsigned char)bank;
    param[offset++] = HEXIN_USHORT_MSB(sa);
    param[offset++] = HEXIN_USHORT_LSB(sa);
    param[offset++] = HEXIN_USHORT_MSB(dl);
    param[offset++] = HEXIN_USHORT_LSB(dl);

    return __makeFrame( HEXIN_MAGICRF_CMD_READ_DATA, param, offset, pbuf );
}

unsigned int writeData( const unsigned char *pwd,   /* Access password. */
                        unsigned char pwdflag,      /* Password format. 0: Hex format; 1: String format. */
                        module_memory_bank_t bank,  /* Memory bank. */
                        unsigned short sa,          /* Data address offset. */
                        unsigned short dl,          /* Data length. */
                        unsigned char *dt,          /* Write data buffer. */
                        unsigned char *pbuf )
{
    unsigned int  length = 0x00;
    unsigned char offset = 0x00;
    unsigned int  maxsize= dl*2 + 4 + 1 + 2 + 2;
    unsigned char param[HEXIN_EPC_DATA_MAX_SIZE];   //Don't init. dangerous relocation: call8: call target out of range: memset
    unsigned char flag   = (pwdflag == 0) ? 0x00 : 0x01;

    if ( (NULL == pwd) || (NULL == dt) || (NULL == pbuf) || (bank >= BANK_MAX ) ) {
        return 0;
    }

    if ( maxsize > HEXIN_EPC_DATA_MAX_SIZE ) {
        return 0;
    }

    switch ( flag ) {
        case 0: /* password is hex */
            for ( offset=0; offset<4; offset++ ) {
                param[offset] = pwd[offset];
            }
            break;

        case 1: /* password is string. */
            offset = __str2hex( (unsigned char *)param, (unsigned char *)pwd, 8 );
        default:
            break;
    }

    param[offset++] = (unsigned char)bank;
    param[offset++] = HEXIN_USHORT_MSB(sa);
    param[offset++] = HEXIN_USHORT_LSB(sa);
    param[offset++] = HEXIN_USHORT_MSB(dl/2);
    param[offset++] = HEXIN_USHORT_LSB(dl/2);

    for ( ; offset < (dl + 9); offset++ ) {
        param[offset] = dt[offset - 9];
    }

    // Data must be an integer multiple of 2bytes.
    if ( (dl % 2) != 0x00 ) {
        param[offset++] = 0x00;
    }

    length = __makeFrame( HEXIN_MAGICRF_CMD_WRITE_DATA, param, offset, pbuf );

    return length;
}

unsigned int writeEPC( const unsigned char *pwd,   /* Access password. */
                       unsigned char pwdflag,      /* Password format. 0: Hex format; 1: String format. */
                       unsigned char *dt,          /* Write data buffer. */
                       unsigned short dl,          /* Data length. */
                       unsigned char *pbuf )
{
    unsigned int   length = 0x00;
    unsigned char  offset = 0x00;
    unsigned short pc     = (dl << 10) & 0xF800;
    unsigned int   maxsize= dl*2 + 4 + 1 + 2 + 2;
    unsigned char  param[HEXIN_EPC_DATA_MAX_SIZE];   //Don't init. dangerous relocation: call8: call target out of range: memset
    unsigned char  flag   = (pwdflag == 0) ? 0x00 : 0x01;

    if ( (NULL == pwd) || (NULL == dt) || (NULL == pbuf) ) {
        return 0;
    }

    if ( maxsize > HEXIN_EPC_DATA_MAX_SIZE ) {
        return 0;
    }

    switch ( flag ) {
        case 0: /* password is hex */
            for ( offset=0; offset<4; offset++ ) {
                param[offset] = pwd[offset];
            }
            break;

        case 1: /* password is string. */
            offset = __str2hex( (unsigned char *)param, (unsigned char *)pwd, 8 );
        default:
            break;
    }

    param[offset++] = (unsigned char)BANK_EPC;
    param[offset++] = 0x00;
    param[offset++] = 0x01;                         // Write offset from pc address. (2Bytes per address)
    param[offset++] = HEXIN_USHORT_MSB(dl/2 + 1);   // +1 for pc value.
    param[offset++] = HEXIN_USHORT_LSB(dl/2 + 1);
    param[offset++] = HEXIN_USHORT_MSB(pc);
    param[offset++] = HEXIN_USHORT_LSB(pc);

    for ( ; offset < (dl + 11); offset++ ) {
        param[offset] = dt[offset - 11];
    }

    // Data must be an integer multiple of 2bytes.
    if ( (dl % 2) != 0x00 ) {
        param[offset++] = 0x00;
    }

    length = __makeFrame( HEXIN_MAGICRF_CMD_WRITE_DATA, param, offset, pbuf );

    return length;
}

unsigned int lock( const unsigned char *pwd, const unsigned char *ld, unsigned char *pbuf )
{
    unsigned char offset = 0x00;
    unsigned char param[7] = { 0x00 };

    if ( (NULL == pwd) || (NULL == ld) ) {
        return 0;
    }

    for ( ; offset<4; offset++ ) {
        param[offset] = pwd[offset];
    }

    for ( ; offset<(4 + 3); offset++ ) {
        param[offset] = ld[offset-4];
    }

    return __makeFrame( HEXIN_MAGICRF_CMD_LOCK, param, offset, pbuf );
}

unsigned int kill( const unsigned char *pwd, unsigned char *pbuf )
{
    unsigned char offset = 0x00;
    unsigned char param[4] = { 0x00 };

    if ( (NULL == pwd) ) {
        return 0;
    }

    for ( ; offset<4; offset++ ) {
        param[offset] = pwd[offset];
    }

    return __makeFrame( HEXIN_MAGICRF_CMD_KILL, param, offset, pbuf );
}

unsigned int getQueryParam( unsigned char *pbuf )
{
    return __makeFrame( HEXIN_MAGICRF_CMD_GET_QUERY, NULL, 0, pbuf );
}

unsigned int setQueryParam( unsigned char select,
                            unsigned char session,
                            unsigned char target,
                            unsigned char qvalue,
                            unsigned char *pbuf )
{
    unsigned int  val = 0x00;
    unsigned char param[2] = { 0x00 };

    val = 0x1000 | ((unsigned int)(select  & 0x03) << 10) | \
                   ((unsigned int)(session & 0x03) <<  8) | \
                   ((unsigned int)(target  & 0x01) <<  7) | \
                   ((unsigned int)(qvalue  & 0x0F) <<  3);

    param[0] = HEXIN_USHORT_MSB(val);
    param[1] = HEXIN_USHORT_LSB(val);

    return __makeFrame( HEXIN_MAGICRF_CMD_SET_QUERY, param, sizeof(param), pbuf );
}

unsigned int setRegion( module_region_t region, unsigned char *pbuf )
{
    if ( (region >= REGION_MAX) || (region == REGION_RESERVED1)) {
        return 0;
    }

    return __makeFrame( HEXIN_MAGICRF_CMD_SET_REGION, (unsigned char *)(&region), 1, pbuf );
}

unsigned int setRFChannel( unsigned char index, unsigned char *pbuf )
{
    return __makeFrame( HEXIN_MAGICRF_CMD_SET_RF_CHANNEL, (&index), 1, pbuf );
}

unsigned int getRFChannel( unsigned char *pbuf )
{
    return __makeFrame( HEXIN_MAGICRF_CMD_GET_RF_CHANNEL, NULL, 0, pbuf );
}

unsigned int setHFSS( unsigned char status, unsigned char *pbuf )
{
    unsigned char param = 0x00;
    param = (status == 0x00 ? status : 0xFF);

    return __makeFrame( HEXIN_MAGICRF_CMD_SET_HFSS, &param, 1, pbuf );
}

unsigned int getPaPower( unsigned char *pbuf )
{ 
    return __makeFrame( HEXIN_MAGICRF_CMD_GET_RF_POWER, NULL, 0, pbuf );   
}

unsigned int setPaPower( float power, unsigned char *pbuf )
{
    unsigned char param[2] = { 0x00 };

    param[0] = HEXIN_USHORT_MSB((unsigned short)(power*100));
    param[1] = HEXIN_USHORT_LSB((unsigned short)(power*100));

    return __makeFrame( HEXIN_MAGICRF_CMD_SET_RF_POWER, param, sizeof(param), pbuf ); 
}

unsigned int setRFCarrier( unsigned char status, unsigned char *pbuf )
{
    unsigned char param = (status == 0x00 ? status : 0xFF);

    return __makeFrame( HEXIN_MAGICRF_CMD_SET_RF_CARRIER, &param, 1, pbuf );
}

unsigned int getRevDemodulatorParam( unsigned char *pbuf )
{
    return __makeFrame( HEXIN_MAGICRF_CMD_GET_RF_GAIN, NULL, 0, pbuf );
}

unsigned int setRevDemodulatorParam( module_mixer_gain_t mixer,     /* Mixer gain.  */
                                     module_if_gain_t ifgain,       /* Intermediate frequency AMP gain. */
                                     unsigned short thrd,           /* Threshold.   */
                                     unsigned char *pbuf )          /* Output */
{
    unsigned char param[4] = { 0x00 };

    if ((mixer >= MIXER_GAIN_MAX) || (mixer < 0)) {
        return 0;
    }

    if ((ifgain >= IF_GAIN_MAX) || (ifgain < 0)) {
        return 0;
    }

    param[0] = (unsigned char)mixer;
    param[1] = (unsigned char)ifgain;
    param[2] = HEXIN_USHORT_MSB(thrd);
    param[3] = HEXIN_USHORT_LSB(thrd);

    return __makeFrame( HEXIN_MAGICRF_CMD_SET_RF_GAIN, param, sizeof(param), pbuf );
}

unsigned int scanJammer( unsigned char *pbuf )
{
    return __makeFrame( HEXIN_MAGICRF_CMD_TEST_SCANJAMMER, NULL, 0, pbuf );
}

unsigned int testRSSI( unsigned char *pbuf )
{
    return __makeFrame( HEXIN_MAGICRF_CMD_TEST_RSSI, NULL, 0, pbuf );
}

/**
 * @brief Configure GPIO.
 *
 * @param [in]  port  : Port number.
 * @param [in]  type  : 0x00 : Setting IO in/out. 0x01 : Setting IO value. 0x02 : Read IO value.
 * @param [in]  value : type == 0x00 : 0x00 -> Input mode, 0x01 -> Output mode;
 *                      type == 0x01 : 0x00 -> Output 0  , 0x01 -> Output 1.
 *                      type == 0x02 : The parameter is invaild.
 * @param [out] pbuf  : Frame buffer.
 * @retval *pbuf length
 * @see None.
 */

unsigned int gpio( module_gpio_pin_t port, module_gpio_type_t type, unsigned char value, unsigned char *pbuf )
{
    unsigned char param[3] = { 0x00 };

    if ((port >= GPIO_PIN_MAX) || (port <= 0)) {
        return 0;
    }

    param[0] = type;
    param[1] = port;
    param[2] = (value == 0x00 ? 0x00 : 0x01);

    return __makeFrame( HEXIN_MAGICRF_CMD_CTRL_IO, param, sizeof(param), pbuf );
}

unsigned int deepSleep( unsigned char *pbuf )
{
    return __makeFrame( HEXIN_MAGICRF_CMD_DEEP_SLEEP, NULL, 0, pbuf );
}

/**
 * @brief Enter deep sleep after n minutes.
 *
 * @param [in]  minute  : 1 <= minute <= 30
 * @param [out] pbuf    : Frame buffer.
 * @retval *pbuf length
 * @see None.
 */

unsigned int deepSleepTime( unsigned char minute, unsigned char *pbuf )
{
    unsigned char param = minute;

    if ( (minute<1) || (minute>30) ) {
        return 0;
    }

    return __makeFrame( HEXIN_MAGICRF_CMD_DEEPSLEEP_TIME, &param, 1, pbuf );
}

/**
 * @brief Module IDLE mode.
 *
 * @param [in]  mode    : 0x00 Exit IDEL; 0x01 Enter IDEL.
 * @param [in]  minute  : 0x00 No enter IDEL; (1 <= minute <= 30)
 * @param [out] pbuf    : Frame buffer.
 * 
 * @retval *pbuf length
 * @see None.
 */
unsigned int idle( unsigned char mode, unsigned char minute, unsigned char *pbuf )
{
    unsigned char param[3] = { 0x00 };

    if ( (minute<0) || (minute>30) ) {
        return 0;
    }

    param[0] = (mode == 0x00 ? 0x00 : 0x01);
    param[1] = 0x01;    // Reserved
    param[2] = minute;

    return __makeFrame( HEXIN_MAGICRF_CMD_IDLE, param, sizeof(param), pbuf );
}

/**
 * @brief Set module mode.
 *
 * @param [in]  mode    : 0x00 High sensitivity; 0x01 Dense reader.
 * @param [out] pbuf    : Frame buffer.
 * 
 * @retval *pbuf length
 * @see UHF RFID Reader App v2.1 source code.
 */
unsigned int setMode( unsigned char mode, unsigned char *pbuf )
{
    unsigned char param = (mode == 0x00 ? 0x00 : 0x01);
    return __makeFrame( HEXIN_MAGICRF_CMD_SET_MODE, &param, 1, pbuf );
}

/**
 * @brief Insert rf channel.
 *
 * @param [in]  start   : RF channel start index.
 * @param [in]  stop    : RF channel stop  index.
 * @param [out] pbuf    : Frame buffer.
 * 
 * @retval *pbuf length
 * @see UHF RFID Reader App v2.1 source code.
 */
unsigned int insertRFChannel( unsigned char start, unsigned char stop, unsigned char *pbuf )
{
    unsigned char param[HEXIN_EPC_DATA_MAX_SIZE];
    unsigned char offset   = 0, i = start;
    unsigned int  maxsize  = stop - start + 1 + 7;

    if ( (maxsize > HEXIN_EPC_DATA_MAX_SIZE) || (start > stop) ) {
        return 0;
    }

    param[offset++] = stop - start + 1;
    for ( ; i<=stop; i++ ) {
        param[offset++] = i;
    }

    return __makeFrame( HEXIN_MAGICRF_CMD_INSERT_RF_CHANNEL, param, offset, pbuf );
}

unsigned int unpackFrame( unsigned char *frame, unsigned char *param, unsigned int *length )
{
    request_packet_t packet;
    unsigned char i  = 0x00;
    unsigned int ret = HEXIN_MAGICRF_NOTHING;

    *length = 0;

    if ( (NULL == frame) || (NULL == param) ) {
        return HEXIN_ERROR;
    }

    if ( HEXIN_MAGICRF_HEAD != *frame ) {
        HPRINT_MAGICRF_ERROR( (" Frame head is ERROR!\r\n") );
        return HEXIN_ERROR;
    }

    packet.head     = frame[HEAD_OFFSET];
    packet.type     = frame[TYPE_OFFSET];
    packet.command  = frame[COMMAND_OFFSET];
    packet.length   = HEXIN_UCHAR2USHORT(frame[LENGTH_MSB_OFFSET], frame[LENGTH_LSB_OFFSET]);
    packet.payload  = &frame[PAYLOAD_OFFSET];
    packet.checksum = frame[packet.length + PAYLOAD_OFFSET];
    packet.tail     = frame[packet.length + PAYLOAD_OFFSET + 1];

    if ( packet.tail != HEXIN_MAGICRF_TAIL ) {
        HPRINT_MAGICRF_ERROR( (" Frame tail is ERROR!\r\n") );
        return HEXIN_ERROR;
    }

    if ( packet.checksum != __checksum( &packet ) ) {
        HPRINT_MAGICRF_ERROR( (" Frame checksum is ERROR! %02X\r\n", __checksum( &packet )) );
        return HEXIN_ERROR;
    }

    switch( packet.command ) {
        case HEXIN_MAGICRF_CMD_INFO:
            HPRINT_MAGICRF_DEBUG( ("MODULE VER  : %d\r\n",  packet.payload[0]) );
            HPRINT_MAGICRF_DEBUG( ("MODULE INFO : %s\r\n", &packet.payload[1]) );
            for ( i=0; i<packet.length; i++ ) {
                param[i] = packet.payload[i + 1];
            }
            ret     = HEXIN_MAGICRF_INFO;
            *length = packet.length - 1;
            break;

        case HEXIN_MAGICRF_CMD_QUERY:   /* RSSI(1B) + PC(2B) + EPC(nB) + CRC(2B)*/
        case HEXIN_MAGICRF_CMD_MUL_QUERY:
            /* Get EPC data. */
            {
                unsigned char  rssi = packet.payload[0];
                unsigned short pc   = HEXIN_UCHAR2USHORT(packet.payload[1], packet.payload[2]);
                unsigned int   len  = ((pc & 0xF800) >> 10) & 0x003E;   // Valid EPC data
                unsigned short crc  = HEXIN_UCHAR2USHORT(packet.payload[packet.length-2], packet.payload[packet.length-1]);

                if ( rssi > 127 ) {
                    rssi =  -( (-rssi) & 0xFF );
                }

                rssi = rssi - __coupling;
                rssi = rssi - __antennaGain;
                rssi = 255  - rssi + 1;

                // Check crc16 value. [x16 + x12 +x5 + 1]
                if ( __chk_ecp_crc16( packet.payload + 1, packet.length - 3, crc ) == 0 ) {
                    HPRINT_MAGICRF_ERROR( (" EPC crc is error!\r\n") );
                    return HEXIN_ERROR;
                }

                // Pack message to return [ EPC,RSSI; ]
                len = __hex2str( param, packet.payload + 3, len );      // Change EPC to string.
                param[len++] = ',';
                *length = len + __hex2str( param + len, &rssi, 1 );
            }
            ret = (HEXIN_MAGICRF_QUERY | HEXIN_MAGICRF_MUL_QUERY);
            break;

        case HEXIN_MAGICRF_CMD_STOP:
        case HEXIN_MAGICRF_CMD_SET_HFSS:
        case HEXIN_MAGICRF_CMD_SET_MODE:
        case HEXIN_MAGICRF_CMD_SET_QUERY:
        case HEXIN_MAGICRF_CMD_GET_QUERY:
        case HEXIN_MAGICRF_CMD_SET_SELECT:  /* setSelectParam(...) */
        case HEXIN_MAGICRF_CMD_INSERT_RF_CHANNEL:
            break;

        case HEXIN_MAGICRF_CMD_SET_RF_POWER:
            *length  = 1;
            param[0] = packet.payload[0];
            ret      = HEXIN_MAGICRF_SET_RF_POWER;            
            break;

        case HEXIN_MAGICRF_CMD_GET_RF_POWER:
            for ( i=0; i<packet.length; i++ ) {
                param[i] = packet.payload[i];
            }
            *length = packet.length;
            ret     = HEXIN_MAGICRF_GET_RF_POWER;
            break;

        case HEXIN_MAGICRF_CMD_TEST_RSSI:
        case HEXIN_MAGICRF_CMD_TEST_SCANJAMMER:
            HPRINT_MAGICRF_INFO( ("JAMMER/RSSI CHL   : %d\r\n",  packet.payload[0]) );
            HPRINT_MAGICRF_INFO( ("JAMMER/RSSI CHH   : %d\r\n",  packet.payload[1]) );
            HPRINT_MAGICRF_INFO( ("JAMMER/RSSI       : ") );
            for ( i=2; i<(packet.length - 2); i++ ) {
                HPRINT_MAGICRF_INFO( ("%X", packet.payload[i]) );
            }
            HPRINT_MAGICRF_INFO( ("\r\n") );

            for ( i=0; i<packet.length; i++ ) {
                param[i] = packet.payload[i];
            }

            *length = packet.length;
            ret     = (HEXIN_MAGICRF_TEST_RSSI | HEXIN_MAGICRF_TEST_SCANJAMMER);
            break;

        case HEXIN_MAGICRF_CMD_READ_DATA:
            {
                /* PC+EPC length (1Byte) + PC(2Bytes) + EPC(nBytes) + Data(nBytes)*/
                unsigned char  ul = packet.payload[0];        // ul = (PC + EPC)'s length.
                for ( i=0; i<( packet.length - ul - 1 ); i++ ) {
                    param[i] = packet.payload[i + 1 + ul];
                }
                *length = packet.length - ul - 1;
                ret     = HEXIN_MAGICRF_READ_DATA;
#ifdef HPRINT_ENABLE_MAGICRF_INFO
                unsigned short pc   = HEXIN_UCHAR2USHORT(packet.payload[1], packet.payload[2]);
                unsigned char *epc  = &packet.payload[3];

                HPRINT_MAGICRF_INFO( ("READ UL   : %d\r\n",      ul) );
                HPRINT_MAGICRF_INFO( ("READ PC   : 0x%04X\r\n",  pc) );
                HPRINT_MAGICRF_INFO( ("READ EPC  : "));
                for ( i=3; i<(ul - 2); i++ ) {
                    HPRINT_MAGICRF_INFO( ("%X", packet.payload[i]) );
                }
                HPRINT_MAGICRF_INFO( ("\r\n") );
                HPRINT_MAGICRF_INFO( ("READ DATA : "));
                for ( i=1+ul; i<(packet.length - ul - 1); i++ ) {
                    HPRINT_MAGICRF_INFO( ("%X", packet.payload[i]) );
                }
                HPRINT_MAGICRF_INFO( ("\r\n") );                            
#endif
            }
            break;

        case HEXIN_MAGICRF_CMD_WRITE_DATA:
            param[0] = packet.payload[1 + packet.payload[0]];   // Get parameter.
            *length  = 1;
            ret      = HEXIN_MAGICRF_WRITE_DATA;
            break;

        case HEXIN_MAGICRF_CMD_SET_RF_CHANNEL:
            break;
            
        case HEXIN_MAGICRF_CMD_GET_RF_CHANNEL:
            param[0] = packet.payload[0];                       // Get channel index.
            *length  = packet.length;
            ret      = HEXIN_MAGICRF_GET_RF_CHANNEL;
            break;

        case HEXIN_MAGICRF_CMD_CTRL_IO:
            if ( packet.length != GPIO_TYPE_MAX ) {
                return HEXIN_ERROR;
            }

            switch ( packet.payload[0] ) {
                case GPIO_TYPE_INIT:
                case GPIO_TYPE_WRITE:
                    HPRINT_MAGICRF_ERROR( ("<ERROR> GPIO : %d\r\n", packet.payload[0]) );
                    break;
                case GPIO_TYPE_READ:
                    param[i] = packet.payload[2];
                    *length  = 1;
                    break;
                default:
                    break;
            }
            ret = HEXIN_MAGICRF_CTRL_IO;
            break;

        case HEXIN_MAGICRF_CMD_ERROR:
            *length  = 1;
            param[0] = packet.payload[0];
            ret      = HEXIN_MAGICRF_ERROR;
            HPRINT_MAGICRF_INFO( ("<ERROR> EXECUTION COMMAND ERROR CODE = 0x%02X\r\n", packet.payload[0]) );
            break;

        default:
            HPRINT_MAGICRF_ERROR( ("<ERROR> INVALID COMMAND : %d\r\n", packet.command) );
            break;
    }

    return ret;
}

unsigned int packetHandler( hexin_ring_buffer_t *ringbuffer,
                            unsigned int trigger,
                            const unsigned char *frame,
                            unsigned int frame_length,
                            void (*callback)( unsigned char *, unsigned int ) ) 
{
    unsigned int     i = 0;
    unsigned char data = 0;
    unsigned int  size = 0, length = 0;
    unsigned char param[HEXIN_M100_BUFFER_MAX_SIZE];
    static packet_handler_t packet = { .offset = 0, .in_packet = HEXIN_FALSE };

    for ( i=0; i<frame_length; i++ ) {
        data  = frame[i];
        switch ( data ) {
            case HEXIN_MAGICRF_HEAD:    // Check protocol head.
                if ( packet.in_packet == HEXIN_FALSE ) {
                    packet.in_packet = HEXIN_TRUE;
                    packet.offset    = 0;
                }
                packet.buffer[packet.offset++] = HEXIN_MAGICRF_HEAD;
                break;

            case HEXIN_MAGICRF_TAIL:    // Check protocol tail.
                packet.buffer[packet.offset++] = HEXIN_MAGICRF_TAIL;
                // Header(1B) + Type(1B) + Command(1B) + PL(2B) + ... + CRC(1B) + End(1B)
                size = 7 + HEXIN_UCHAR2USHORT( packet.buffer[LENGTH_MSB_OFFSET],
                                               packet.buffer[LENGTH_LSB_OFFSET] );

                if ( packet.offset > size ) {       // Package is ERROR.
                    packet.offset    = 0;
                    packet.in_packet = HEXIN_FALSE;
                    break;
                }

                if ( packet.offset < size) {        // Not finished package.
                    break;
                }

                packet.in_packet = HEXIN_FALSE;
                packet.offset    = 0;

                if ( trigger & unpackFrame( packet.buffer, param, &length ) ) {
                    param[length++] = HEXIN_SEPARATOR;
                    if ( hexinRingBufferWrite( ringbuffer, param, length ) != length ) {    // RingBuffer is full.
                        callback( param, length );
                    }
                } /* trigger & unpackFrame(...) */
                
                break;

            default:
                if ( packet.offset >= HEXIN_M100_BUFFER_MAX_SIZE ) {
                    packet.offset    = 0;
                    packet.in_packet = HEXIN_FALSE;
                }
                if ( packet.in_packet == HEXIN_TRUE ) {
                    packet.buffer[packet.offset++] = data;
                }
                break;
        }
    }

    length = hexinRingBufferUsedSpace( ringbuffer );
    if ( length > 0 ) {
        callback( param, length );  // User get all data from ringbuffer.
    }

    return HEXIN_TRUE;
}

/****************************************************************************************************************/
const unsigned short crc16_table[256] = {   0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
                                            0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
                                            0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
                                            0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
                                            0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
                                            0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
                                            0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
                                            0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
                                            0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
                                            0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
                                            0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
                                            0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
                                            0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
                                            0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
                                            0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
                                            0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
                                            0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
                                            0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
                                            0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
                                            0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
                                            0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
                                            0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
                                            0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
                                            0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
                                            0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
                                            0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
                                            0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
                                            0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
                                            0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
                                            0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
                                            0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
                                            0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0 };

static unsigned short __hexin_update_crc16_1021( unsigned short crc16, unsigned char c )
{
    unsigned short crc = crc16;
    unsigned short tmp, short_c;

    short_c  = 0x00FF & (unsigned short) c;
    tmp = (crc >> 8) ^ short_c;
    crc = (crc << 8) ^ crc16_table[tmp];

    return crc;
}

static unsigned short __hexin_calc_crc16_1021( const unsigned char *pbuf, unsigned int len, unsigned short crc16 )
{
    unsigned int i = 0;
    unsigned short crc = crc16;

	for ( i=0; i<len; i++ ) {
		crc = __hexin_update_crc16_1021( crc, pbuf[i] );
	}
	return crc;
}

static unsigned short __get_ecp_crc16( const unsigned char *pbuf, unsigned int len )
{
    unsigned short crc = 0x0000;
    crc = __hexin_calc_crc16_1021( pbuf, len, 0xFFFF );
    return (crc ^ 0xFFFF);
}

static unsigned int  __chk_ecp_crc16( const unsigned char *pbuf, unsigned int len, unsigned short crc16 )
{
    if ( __get_ecp_crc16( pbuf, len ) == crc16 ) {
        return 1;
    }

    return 0;
}

/****************************************************************************************************************/

unsigned int hexinRingBufferInit( hexin_ring_buffer_t* ring_buffer,
                                  unsigned char* buffer, unsigned int buffer_size )
{
    if ( (NULL == ring_buffer) || (NULL == buffer) || (0 == buffer_size) ) {
        return HEXIN_FALSE;
    }

    ring_buffer->buffer = (unsigned char*)buffer;
    ring_buffer->size   = buffer_size;
    ring_buffer->head   = 0;
    ring_buffer->tail   = 0;
    return HEXIN_TRUE;
}

unsigned int hexinRingBufferSize( hexin_ring_buffer_t* ring_buffer )
{
    return ring_buffer->size;
}

void hexinRingBufferGetData( hexin_ring_buffer_t* ring_buffer, unsigned char** data,
                             unsigned int* contiguous_bytes )
{
    unsigned int head_to_end = ring_buffer->size - ring_buffer->head;

    *data = &ring_buffer->buffer[ring_buffer->head];
    *contiguous_bytes = HEXIN_MIN( head_to_end, (head_to_end + ring_buffer->tail) % ring_buffer->size );
}

void hexinRingBufferConsume( hexin_ring_buffer_t* ring_buffer, unsigned int bytes_consumed )
{
    /* Consume elements by updating the head */
    ring_buffer->head = ( ring_buffer->head + bytes_consumed ) % ring_buffer->size;
}

unsigned int hexinRingBufferFreeSpace( hexin_ring_buffer_t* ring_buffer )
{
    unsigned int tail_to_end = ring_buffer->size - ring_buffer->tail;
    return ((tail_to_end - 1 + ring_buffer->head) % ring_buffer->size);
}

unsigned int hexinRingBufferUsedSpace( hexin_ring_buffer_t* ring_buffer )
{
    unsigned int head_to_end = ring_buffer->size - ring_buffer->head;
    return ( (head_to_end + ring_buffer->tail) % ring_buffer->size );
}

void hexinRingBufferRead( hexin_ring_buffer_t* ring_buffer, unsigned char* data,
                          unsigned int data_length, unsigned int* number_of_bytes_read )
{
    unsigned int i      = 0;
    unsigned int max_bytes_to_read = 0;
    unsigned int head   = ring_buffer->head;

    max_bytes_to_read = HEXIN_MIN( data_length, hexinRingBufferUsedSpace(ring_buffer) );

    if ( max_bytes_to_read != 0 ) {
        for ( i = 0; i != max_bytes_to_read; i++, ( head = ( head + 1 ) % ring_buffer->size ) ) {
            data[ i ] = ring_buffer->buffer[ head ];
        }
        hexinRingBufferConsume( ring_buffer, max_bytes_to_read );
    }

    *number_of_bytes_read = max_bytes_to_read;
}

unsigned int hexinRingBufferWrite( hexin_ring_buffer_t* ring_buffer, const unsigned char* data,
                                   unsigned int data_length )
{
    unsigned int i = 0;
    unsigned int tail_to_end = ring_buffer->size - ring_buffer->tail;
    unsigned int amount_to_copy = HEXIN_MIN( data_length, (tail_to_end - 1 + ring_buffer->head) % ring_buffer->size );

    if ( (NULL == ring_buffer->buffer) || (NULL == data) ) {
        return HEXIN_FALSE;
    }

    if ( hexinRingBufferFreeSpace( ring_buffer ) < data_length ) {
        return HEXIN_FALSE;
    }

    for ( i=0; i<HEXIN_MIN( amount_to_copy, tail_to_end ); i++ ) {
        ring_buffer->buffer[ring_buffer->tail + i] = data[i];
    }

    if ( tail_to_end < amount_to_copy ) {
        for ( i=0; i<amount_to_copy - tail_to_end; i++ ) {
            ring_buffer->buffer[i] = data[tail_to_end + i];
        }
    }

    /* Update the tail */
    ring_buffer->tail = (ring_buffer->tail + amount_to_copy) % ring_buffer->size;

    return amount_to_copy;
}

/* End */
