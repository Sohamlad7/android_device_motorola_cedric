/*Copyright (C) 2010-2013 Motorola, Inc.
 *All Rights Reserved.
 *Motorola Confidential Restricted (MCR).
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>     // sscanf
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cutils/log.h>
#include <cutils/properties.h>
#include <inttypes.h>
#include <unistd.h>

#include "CRC32.h"
#include "SensorHub.hpp"

#ifdef MODULE_stml0xx
    #include <linux/stml0xx.h>
    #define STM_DRIVER "/dev/stml0xx"
    #define FLASH_SIZE (0x00ffFFUL) // L0 = 64k
    // The value used to fill unused buffer space / flash.
    #define FLASH_FILL (0x00)
    #define STM_MAX_PACKET_LENGTH 256
    #define MOTOSH_PASSTHROUGH_SIZE STML0XX_PASSTHROUGH_SIZE

    // IOCTL mappings
    #define MOTOSH_IOCTL_BOOTLOADERMODE     STML0XX_IOCTL_BOOTLOADERMODE
    #define MOTOSH_IOCTL_NORMALMODE         STML0XX_IOCTL_NORMALMODE
    #define MOTOSH_IOCTL_MASSERASE          STML0XX_IOCTL_MASSERASE
    #define MOTOSH_IOCTL_SETSTARTADDR       STML0XX_IOCTL_SETSTARTADDR
    #define MOTOSH_IOCTL_TEST_READ          STML0XX_IOCTL_TEST_READ
    #define MOTOSH_IOCTL_TEST_WRITE         STML0XX_IOCTL_TEST_WRITE
    #define MOTOSH_IOCTL_TEST_WRITE_READ    STML0XX_IOCTL_TEST_WRITE_READ
    #define MOTOSH_IOCTL_TEST_BOOTMODE      STML0XX_IOCTL_TEST_BOOTMODE
    #define MOTOSH_IOCTL_SET_DEBUG          STML0XX_IOCTL_SET_DEBUG
    #define MOTOSH_IOCTL_SET_FACTORY_MODE   STML0XX_IOCTL_SET_FACTORY_MODE
    #define MOTOSH_IOCTL_WRITE_REG          STML0XX_IOCTL_WRITE_REG
    #define MOTOSH_IOCTL_READ_REG           STML0XX_IOCTL_READ_REG
    #define MOTOSH_IOCTL_SET_LOWPOWER_MODE  STML0XX_IOCTL_SET_LOWPOWER_MODE
    #define MOTOSH_IOCTL_PASSTHROUGH        STML0XX_IOCTL_PASSTHROUGH
#else // MODULE_motosh
    #include <linux/motosh.h>
    #define STM_DRIVER "/dev/motosh"
    #define FLASH_SIZE (0x0FffFFUL) // L4 = 1M
    // The value used to fill unused buffer space / flash.
    #define FLASH_FILL (0xff)
    #define STM_MAX_PACKET_LENGTH 248
#endif

/******************************* # defines **************************************/
#define CAPSENSE_FW_UPDATE  "/sys/class/capsense/fw_update"
#define CS_MAX_LEN 8

/** The firmware blacklist that this flasher will ignore */
#define STM_FIRMWARE_BLACKLIST "/system/etc/firmware/sensorhub-blacklist.txt"
/** Maximum filesystem path length */
#define STM_MAX_PATH 256
#define STM_SUCCESS 0
#define STM_FAILURE -1
#define STM_VERSION_MISMATCH -1
#define STM_VERSION_MATCH 1
#define STM_DOWNLOADRETRIES 3
/* 512 matches the read buffer in kernel */
#define STM_MAX_GENERIC_DATA 512
#define STM_MAX_GENERIC_HEADER 4
#define STM_MAX_GENERIC_COMMAND_LEN 3
#define STM_FORCE_DOWNLOAD_MSG  "Use -f option to ignore version check eg: motosh boot -f\n"
#define FLASH_START_ADDRESS (0x08000000)

#define ANTCAP_CAL_FILE "/persist/antcap/captouch_caldata.bin"


#define CHECK_RETURN_VALUE(ret, msg)  if ((ret) < 0) {\
                     ALOGE("%s: %s \n",msg, strerror(errno)); \
                     printf("%s: %s \n",msg, strerror(errno)); \
                     goto EXIT; \
                        }

#define CHECKIFHEX(c)  ((c >= 'A' && c <= 'F') || ( c >= '0' && c <='9') || ( c >= 'a' && c <= 'f'))

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

#define LOGERROR(format, ...) {\
        ALOGE(format,## __VA_ARGS__); \
        printf(format,##__VA_ARGS__); \
}

#define LOGINFO(format, ...) {\
        ALOGI(format,## __VA_ARGS__); \
        printf(format,##__VA_ARGS__); \
}

#ifdef _DEBUG
#define LOGDEBUG(format, ...) ALOGE(format,## __VA_ARGS__);
#else
#define LOGDEBUG(format, ...) ;
#endif

/****************************** structures & enums *****************************/
typedef enum tag_stmmode
{
    BOOTLOADER,
    NORMAL,
    DEBUG,
    FACTORY,
    VERSION,
    TBOOT,
    TREAD,
    TWRITE,
    TMWRITE,
    TMWRRD,
    READWRITE,
    PASSTHROUGH,
    LOWPOWER_MODE,
    MASS_ERASE_PART,
    INVALID
} eStm_Mode;

typedef enum {
    FW_STOCK,   //< Stock firmware
    FW_APK      //< Firmware uploaded by a PlayStore APK
} FirmwareType;

/****************************** globals ****************************************/

 /** A file descriptor for the ioctl to communicate with the SensorHub. */
int devFd = -1;

using namespace std;
using namespace mot;

SensorHub sensorHub;

/****************************** functions **************************************/

int stm_convertAsciiToHex(char * input, unsigned char * output, int inlen);

static inline int motosh_ioctl (int fd, int ioctl_number, ...) {
    va_list ap;
    void * arg;
    int status = 0;
    int error = 0;

    va_start(ap, ioctl_number);
    arg = va_arg(ap, void *);
    va_end(ap);

    do {
        status = ioctl(fd, ioctl_number, arg);
        error = errno;
    } while ((status != 0) && (error == -EINTR));
    return status;
}

#ifdef MODULE_motosh
/* download cal/cfg data and enable capsense */
void configure_capsense() {

    FILE *fp;
    int i;
    int ant_data;
    int err = 0;
    unsigned char enable;
    unsigned char buf[MOTOSH_ANTCAP_CAL_BUFF_SIZE] = {0};

    LOGINFO("Sensorhub hal antcap disable");
    enable = 0;
    err = motosh_ioctl(devFd, MOTOSH_IOCTL_SET_ANTCAP_ENABLE, &enable);
    if (err) {
        LOGERROR("Can't send Ant Disable: %d\n", err);
    }

    if (!err) {
        LOGINFO("Sensorhub hal antcap cfg");
        /* note:  antcap_cfg is now resident in device tree, mAntCfg unused */
        err = motosh_ioctl(devFd, MOTOSH_IOCTL_SET_ANTCAP_CFG, &buf);
        if (err) {
            LOGERROR("Unable to send SET_ANTCAP_ENABLE ioctl: %d\n", err);
        }
    }

    if (!err) {
        LOGINFO("Sensorhub hal antcap cal");
        if ((fp = fopen(ANTCAP_CAL_FILE, "r")) == NULL) {
            LOGERROR("Unable to open antcap cal file %s, exiting", ANTCAP_CAL_FILE);
            err = 1;
        }
        else {
            LOGINFO("Using antcap cal file %s", ANTCAP_CAL_FILE);
        }

        if (fp != NULL) {
            for (i=0; i<MOTOSH_ANTCAP_CAL_BUFF_SIZE; i++) {
                ant_data = fgetc(fp);
                if (ant_data == EOF) {
                    memset(buf, 0xff, sizeof(buf));
                    break;
                }
                buf[i] = ant_data;
            }
            fclose(fp);
            err = motosh_ioctl(devFd, MOTOSH_IOCTL_SET_ANTCAP_CAL, &buf);
            if (err < 0) {
                LOGERROR("Unable to send SET_ANTCAP_CAL ioctl: %d\n", err);
            }
        }
    }

    if (!err) {
        LOGINFO("Sensorhub hal antcap enable");
        enable = 1;
        err = motosh_ioctl(devFd, MOTOSH_IOCTL_SET_ANTCAP_ENABLE, &enable);
        if (err) {
            LOGERROR("Unable to send SET_ANTCAP_ENABLE ioctl: %d\n", err);
        }
    }
}

/* force a check and flash of capsense if needed */
void flash_capsense(void) {

    char checksum[CS_MAX_LEN];
    int cs = 0;
    int checks = 5;
    char *end;
    int c;
    FILE *fp;
    struct stat buf;

    /* exit if there is no capsense flash control path available */
    if (stat(CAPSENSE_FW_UPDATE, &buf) < 0)
        return;

    fp = fopen(CAPSENSE_FW_UPDATE, "w");
    if(fp){
        LOGINFO("Opened capsense flash control\n")
        fputc('1',fp);
        fclose(fp);
    } else
         LOGERROR("Failed to open capsense flash control\n")

    /* look for a non-zero checksum on the same fd */
    while(checks--) {
        sleep(1);
        fp = fopen(CAPSENSE_FW_UPDATE, "r");
        if (fp) {
            int i = 0;
            while ((c = fgetc(fp)) != '\0' &&
                   c != EOF &&
                   i < CS_MAX_LEN-1) {
                checksum[i++] = c;
            }
            checksum[i] = '\0';

            cs = (int)strtol(checksum, &end, 16);
            if (cs != 0)
                checks = 0;

            fclose(fp);
        } else
            LOGERROR("Failed to read capsense flash status\n")
    }
    LOGINFO("Capsense checksum 0x%X\n", cs)
    sleep(2);
}
#endif


/** Computes the absolute firmware file path for firmware of the given type.
 *
 * @param fileName A pointer to a buffer of at least STM_MAX_PATH bytes. A
 * null-terminated string containing the full/absolute path to the firmware
 * binary will be written to this buffer.
 * @param type The firmware type to obtain the path for.
 *
 * @return A negative value on error.
 */
int stm_getFwFile(char *fileName, FirmwareType type) {
    int ret;

    switch (type) {
        case FW_APK:
            ret = snprintf(fileName, STM_MAX_PATH, "/data/misc/sensorhub/sensorhubfw.bin");
            break;
        case FW_STOCK:
            ret = snprintf(fileName, STM_MAX_PATH, "/system/etc/firmware/sensorhubfw.bin");
            break;
        default:
            return -1; // Invalid firmware type
    }

    if (ret >= STM_MAX_PATH) return -2; // Output was truncated.

    if (access(fileName, R_OK) != 0) {
        return -3; // No read access (file doesn't exist)
    }

    return 0;
}

/** Computes the absolute firmware file path for the firmware binary that
 * should be loaded on the SensorHub.
 *
 * If the APK provided version is present, it will be preferred over the system
 * build-time version.
 *
 * @param fileName A pointer to a buffer of at least STM_MAX_PATH bytes. A
 * null-terminated string containing the full/absolute path to the firmware
 * binary will be written to this buffer.
 *
 * @return A negative value on error.
 * */
int stm_getFwFile(char *fileName) {
    static bool reported = false;
    int ret;

    // Prefer the APK FW version if present.
    if ((ret = stm_getFwFile(fileName, FW_APK)) < 0) {
        ret = stm_getFwFile(fileName, FW_STOCK);
    }

    if (ret >= 0 && !reported) {
        LOGINFO("MOTOSH using firmware: %s\n", fileName);
        reported = true;
    }

    return ret;
}

/**
 * Processes the firmware black list and deletes any APK installed firmware
 * that is in the list.
 */
void stm_processBlackList() {
    int res;
    char path[STM_MAX_PATH];

    if (access(STM_FIRMWARE_BLACKLIST, R_OK) != 0) return; // We don't have a blacklist

    if ((res = stm_getFwFile(path, FW_APK)) < 0) {
        return; // No APK firmware to check/delete
    }

    uint32_t fileCrc, blCrc;
    res = calculateFileCrc32(path, FLASH_SIZE, FLASH_FILL, &fileCrc);

    if (res < 0) return; // No CRC to check against

    FILE *blackListFp = fopen(STM_FIRMWARE_BLACKLIST, "r");
    if (blackListFp == NULL) return; // Huh?

    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    while ((read = getline(&line, &len, blackListFp)) != -1) {
        if (len > 0 && line[0] == '#') continue;    // Skip comments

        res = sscanf(line, "%" SCNx32, &blCrc);
        if (res == EOF || res != 1) continue;

        if (fileCrc == blCrc) {
            LOGINFO("Deleting blacklist firmware: %08x %s\n", blCrc, path);
            res = unlink(path);
            goto EXIT;
        }
    }

EXIT:
    fclose(blackListFp);
    if (line) free(line);
}

/** Extract the firmware version from the filesystem binary containing the firmware.
 *
 * @return A string containing the version, or an empty string.
 * */
string stm_getFwVersionFromFile() {
    char path[STM_MAX_PATH];
    if (int res = stm_getFwFile(path) < 0) {
        LOGERROR("stm_getFwVersionFromFile: getFwFile = %i\n", res);
        return string();
    }
    FILE *f = fopen(path, "r");
    if (!f) return string();

    // The build system appends a 32 bit address as ASCII HEX (8 bytes),
    // followed by \n, at the end of the file. This is the address of the
    // version string in flash, so it's something like "080177d4".
    const int OffsetLen = 9;
    int seek = fseek(f, -OffsetLen, SEEK_END);
    if (seek == -1) {
        fclose(f);
        LOGERROR("stm_getFwVersionFromFile: SEEK_END failed\n");
        return string();
    }

    char offsetStr[OffsetLen];
    size_t sz = fread(offsetStr, 1, OffsetLen - 1, f);
    if (sz != OffsetLen - 1) {
        fclose(f);
        LOGERROR("stm_getFwVersionFromFile: offset read failed\n");
        return string();
    }

    unsigned char offsetHex[OffsetLen / 2];
    stm_convertAsciiToHex(offsetStr, offsetHex, OffsetLen - 1);
    uint32_t offset = 0;
    // We ignore the MSB byte because that's the flash offset (typically
    // 0x08000000) and we want instead the file offset which is relative to the
    // start of the flash.
    //offset += ((uint32_t)offsetHex[0]) << 24;
    offset += ((uint32_t)offsetHex[1]) << 16;
    offset += ((uint32_t)offsetHex[2]) <<  8;
    offset += ((uint32_t)offsetHex[3]);

    seek = fseek(f, offset, SEEK_SET);
    if (seek == -1) {
        fclose(f);
        LOGERROR("stm_getFwVersionFromFile: seek failed\n");
        return string();
    }

    // Read the null-terminated string
    char fwVersionStr[FW_VERSION_STR_MAX_LEN];
    int c;
    unsigned i = 0;
    fwVersionStr[0] = '\0'; // In case we don't read anything.
    while ((c = fgetc(f)) != '\0' && c != EOF && i < FW_VERSION_STR_MAX_LEN) {
        fwVersionStr[i++] = c;
    }

    fclose(f);

    if (c == EOF || i >= FW_VERSION_STR_MAX_LEN - 1) return string();

    fwVersionStr[i] = '\0'; // Since we didn't copy the null
    return fwVersionStr;
}

/** Computes the pseudo-CRC of a firmware binary file. This is not a simplistic
 * CRC over the file contents. Instead this function will compute the CRC that
 * would be computed by the SensorHub if the firmware binary file were to be
 * loaded in flash on the SensorHub and the CRC computed over the whole flash.
 *
 * @param crc The computed CRC.
 * @return Returns 0 on success or a negative value on error.
 */
int stm_calcFwFileCrc(uint32_t &crc) {
    int res;
    char path[STM_MAX_PATH];
    if ((res = stm_getFwFile(path)) < 0) {
        LOGERROR("Error: getFwFile = %i\n", res);
        return -1;
    }

    res = calculateFileCrc32(path, FLASH_SIZE, FLASH_FILL, &crc);
    return res;
}

/**
 * Compares the version string and CRC of the firmware in the AP file system
 * and in the SensorHub hardware.
 *
 * Given the CRC check, the version string check is redundant, but is being
 * done for informative purposes. It is possible for different firmwares to
 * be built with the same version string (for example, different developer
 * iterations that all have the "-dirty" flag), but if the contents are trully
 * different the CRC will not match.
 *
 * @return One of STM_VERSION_MISMATCH (which could indicate an actual mismatch
 * or an error in obtaining one or more pieces of data required for the check)
 * or STM_VERSION_MATCH which indicates the version string and CRC matches.
 */
int stm_versionCheck() {
    int res;
    uint32_t fileCrc = 0, hwCrc = 0;

    string fileVersion = stm_getFwVersionFromFile();
    if (fileVersion.empty()) {
        LOGERROR("getFwVersionFromFile failed\n");
        return STM_VERSION_MATCH; // Corrupt file?
    }

    res = stm_calcFwFileCrc(fileCrc);
    if (res < 0) {
        LOGERROR("calcFwFileCrc returned %i\n", res);
        return STM_VERSION_MATCH; // Corrupt file?
    }

    string hwVersion = sensorHub.getVersionStr();
    if (hwVersion.empty()) {
        LOGERROR("Unable to retrieve the FW version from the HW\n");
        return STM_VERSION_MISMATCH; // Corrupt SH?
    }

    hwCrc = sensorHub.getFlashCrc();
    if (!hwCrc) {
        LOGERROR("Could not get FW CRC from HW\n");
        return STM_VERSION_MISMATCH; // Corrupt SH?
    }

    LOGINFO("Version info: in filesystem = %s\n", fileVersion.c_str());
    LOGINFO("Version info: in hardware   = %s\n", hwVersion.c_str());
    LOGINFO("FW CRC value: in filesystem = 0x%08X\n", fileCrc);
    LOGINFO("FW CRC value: in hardware   = 0x%08X\n", hwCrc);

    if ((hwVersion == fileVersion) && (fileCrc == hwCrc)) {
        return STM_VERSION_MATCH;
    } else {
        return STM_VERSION_MISMATCH;
    }
}

int stm_convertAsciiToHex(char * input, unsigned char * output, int inlen)
{
    int i=0,outlen=0,x,result;

    if (input != NULL && output != NULL) {
        while(i < inlen) {
            result = sscanf(input+i,"%02x",&x);
            if (result != 1) {
                break;
            }
            output[outlen++] = (unsigned char)x;
            i= i+2;
        }
    }
    return outlen;
}

int stm_getpacket( FILE ** filepointer, unsigned char * databuff)
{
    FILE * fp = *filepointer;
    int len = 0;
    unsigned char c;

    while(1){
        c = fgetc(fp);
        if (feof(fp))
            break;
        databuff[len] = c;
        len++;
        if( len >= STM_MAX_PACKET_LENGTH) {
            break;
        }
    }

    /* packet size needs to be a multiple of 8 bytes (64 bits) */
    while(len < STM_MAX_PACKET_LENGTH &&
          len % 8 > 0){
        databuff[len] = FLASH_FILL;
        len++;
    }
    return len;

}

int stm_downloadFirmware(FILE *filep)
{

    unsigned int address;
    int ret = STM_SUCCESS;
    int packetlength;
#ifdef _DEBUG
    int packetno = 0;
#endif
    unsigned char packet[STM_MAX_PACKET_LENGTH];
    int temp = 100; // this is only a dummy variable for the 3rd parameter of ioctl call

    LOGDEBUG("Ioctl call to switch to bootloader mode\n");
    ret = motosh_ioctl(devFd, MOTOSH_IOCTL_BOOTLOADERMODE, &temp);
    CHECK_RETURN_VALUE(ret,"Failed to switch STM to bootloader mode\n");

    LOGDEBUG("Ioctl call to erase flash on STM\n");
    ret = motosh_ioctl(devFd, MOTOSH_IOCTL_MASSERASE, &temp);
    CHECK_RETURN_VALUE(ret,"Failed to erase STM \n");

    address = FLASH_START_ADDRESS;
    ret = motosh_ioctl(devFd, MOTOSH_IOCTL_SETSTARTADDR, &address);
    CHECK_RETURN_VALUE(ret,"Failed to set address\n");

    LOGDEBUG("Start sending firmware packets to the driver\n");
    // Move the file pointer to the beginning of the file in case this is a re-try
    fseek(filep, 0, SEEK_SET);
    do {
        packetlength = stm_getpacket (&filep, packet);
        if( packetlength == 0)
            break;
#ifdef _DEBUG
        LOGDEBUG("Sending packet %d  of length %d:\n", packetno++, packetlength);
        int i;
        for( i=0; i<packetlength; i++)
            LOGDEBUG("%02x ",packet[i]);
#endif
        printf(".");
        fflush(stdout);

        ret = write(devFd, packet, packetlength);
        CHECK_RETURN_VALUE(ret,"Packet download failed\n");
    } while(packetlength != 0);

EXIT:
    return ret;
}

/*!
 * \brief Print help info
 *
 * \param[in] terminate 1 if you also want to \c exit(), 0 otherwise
 */
void help(int terminate)
{
    printf("\n");
    printf("motosh - Moto sensorhub debug and control\n");
    printf("USAGE:  ./motosh <command> <command-options>\n");
    printf("  command:\n");
    printf("    help - print this message\n");
    printf("    boot - download new firmware to hub\n");
    printf("      options:\n");
    printf("        -f disables version check\n");
    printf("    normal - reset hub into normal mode\n");
    printf("    tboot - send hub into bootloader mode\n");
    printf("    tread - read a hub register\n");
    printf("      options:\n");
    printf("    twrite - write a hub register\n");
    printf("    tmread\n");
    printf("      options: <register> <nbytes>\n");
    printf("        register - register number\n");
    printf("        nbytes   - number of bytes to read\n");
    printf("    tmwrite\n");
    printf("    debug - turn on/off kernel dynamic debug\n");
    printf("      options: <state>\n");
    printf("        state - 1 for on, 0 for off\n");
    printf("    factory - send hub into factory mode\n");
    printf("    getversion - display firmware version in hub and filesystem\n");
    printf("    readwrite\n");
    printf("      options: <type> <address> <size> <data>\n");
    printf("        type    - 1 byte -- 00 for read, 01 for write\n");
    printf("        address - 2 bytes\n");
    printf("        size    - 2 bytes size of read/write\n");
    printf("        data    - bytes to write\n");
    printf("      ex. -- read version\n");
    printf("        ./motosh readwrite 00 00 01 00 01\n");
    printf("      ex. -- write 2 bytes\n");
    printf("        ./motosh readwrite 01 00 0D 00 02 CC DD\n");
    printf("    passthrough\n");
    printf("      options: <bus> <I2C addr> <reg addr> <r/w> <size> <data>\n");
    printf("        bus      - 1 byte I2C bus number\n");
    printf("        I2C addr - 1 byte I2C address of the sensor\n");
    printf("        reg addr - 2 byte sensor register address\n");
    printf("        r/w      - 1 byte read/write flag, 00 = read, 01 = write\n");
    printf("        size     - 1 byte read or write size\n");
    printf("        data     - bytes to be written (only used for write commmands)\n");
    printf("      ex. -- read 6 bytes of accel data\n");
    printf("        ./motosh passthrough 00 d0 00 12 00 06\n");
    printf("      ex. -- write 1 byte config register\n");
    printf("        ./motosh passthrough 00 d0 00 42 01 01 28\n");
    printf("    lowpower - enable/disable low power mode\n");
    printf("      options: <state>\n");
    printf("        state - 1 for enable, 0 for disable\n");
    printf("    masserase - erase the firmware\n");
    printf("\n");
    if( terminate )
        exit(0);
}

int  main(int argc, char *argv[])
{

    int tries, ret = STM_SUCCESS;
    FILE * filep = NULL;
    eStm_Mode emode = INVALID;
    int temp = 100; // this is only a dummy variable for the 3rd parameter of ioctl call
    unsigned char hexinput[250];
    int count, i;
    short delay = 0;
    int enabledints = 0;
    bool forceBoot = false;
    char ver_string[FW_VERSION_SIZE];
    char fw_file_name[STM_MAX_PATH];

    LOGDEBUG("Start MOTOSH  Version-1 service\n");

    /*parse command line arguements */
    if( argc < 2 || !strcmp(argv[1], "help") )
        help(1);
    else if(!strcmp(argv[1], "boot"))
        emode = BOOTLOADER;
    else if( !strcmp(argv[1], "normal"))
        emode = NORMAL;
    else if(!strcmp(argv[1], "tboot"))
        emode = TBOOT;
    else if(!strcmp(argv[1], "tread"))
        emode = TREAD;
    else if(!strcmp(argv[1], "twrite"))
        emode = TWRITE;
    else if(!strcmp(argv[1], "tmread"))
        emode = TMWRRD;
    else if(!strcmp(argv[1], "tmwrite"))
        emode = TMWRITE;
    else if(!strcmp(argv[1], "debug"))
        emode = DEBUG;
    else if(!strcmp(argv[1], "factory"))
        emode = FACTORY;
    else if(!strcmp(argv[1], "getversion"))
        emode = VERSION;
    else if(!strcmp(argv[1], "readwrite"))
        emode = READWRITE;
    else if(!strcmp(argv[1], "passthrough"))
        emode = PASSTHROUGH;
    else if(!strcmp(argv[1], "lowpower"))
        emode = LOWPOWER_MODE;
    else if(!strcmp(argv[1], "masserase"))
        emode = MASS_ERASE_PART;

    /* check if its a force download */
    if (emode == BOOTLOADER && (argc == 3)) {
        if (!strcmp(argv[2], "-f")) {
            forceBoot = true;
        }
    }

    /* open the device */
    devFd = open(STM_DRIVER,O_RDONLY|O_WRONLY);
    if (devFd < 0) {
        LOGERROR("Unable to open motosh driver (are you root?): %s\n", strerror(errno))
        ret = STM_FAILURE;
        goto EXIT;
    }


    if (emode == BOOTLOADER) {

        #ifdef MODULE_motosh
            /* trigger capsense check and flash if this is a normal
               boot up check (no -f option applied) */
            if (!forceBoot)
                flash_capsense();
        #endif

        stm_processBlackList();

        // Verify we have FW before taking the part out of reset/boot mode
        ret = stm_getFwFile(fw_file_name);

        CHECK_RETURN_VALUE(ret, "STM valid firmware not found");

        // Take the part out of reset
        ret = motosh_ioctl(devFd, MOTOSH_IOCTL_NORMALMODE, &temp);
        CHECK_RETURN_VALUE(ret, "STM boot -> normal mode failed");

        // Wait until the SensorHub boots
        tries = 0;
        ret = STM_FAILURE;
        while (tries++ < 20) {
            uint32_t hwCrc = sensorHub.getFlashCrc();
            if (hwCrc) {
                ret = STM_SUCCESS;
                break;
            }
            usleep(100000); // Wait for a total of 2s
        }
        if (ret != STM_SUCCESS) {
            LOGINFO("STM getFlashCrc() after normal mode failed. Blank part? (%d)", ret);
        }

        /* check if new firmware available for download */
        if (forceBoot || stm_versionCheck() == STM_VERSION_MISMATCH) {
            filep = fopen(fw_file_name, "r");
            CHECK_RETURN_VALUE(ret = filep ? 0 : -1, "STM could not open FW file");

            tries = 0;
            while((tries < STM_DOWNLOADRETRIES )) {
                if( (stm_downloadFirmware(filep)) >= STM_SUCCESS) {
                    fclose(filep);
                    filep = NULL;
                    /* reset STM */
                    ret = motosh_ioctl(devFd, MOTOSH_IOCTL_NORMALMODE, &temp);
                    printf("\n");
                    // IOCTLS will be briefly blocked during part reset
                    usleep(1000000);
                    if (stm_versionCheck() != STM_VERSION_MATCH) {
                        /* try once more */
                        usleep(2000000);
                        if (stm_versionCheck() == STM_VERSION_MATCH)
                            LOGINFO("Firmware download completed successfully\n")
                        else
                            LOGERROR("Firmware download error\n")
                    } else
                        LOGINFO("Firmware download completed successfully\n")

                    break;
                }
                tries++;
                usleep(250000);
            }

            CHECK_RETURN_VALUE(ret = (tries >= STM_DOWNLOADRETRIES ? -1 : 0),
                    "Firmware download failed")
        } else {
            LOGDEBUG("No new firmware to download \n");
        }

        #ifdef MODULE_motosh
        if (!forceBoot)
            configure_capsense();
        #endif

        property_set("hw.motosh.booted", "1");
    }
    if(emode == NORMAL) {
        LOGDEBUG("Ioctl call to reset STM\n");
        ret = motosh_ioctl(devFd,MOTOSH_IOCTL_NORMALMODE, &temp);
        CHECK_RETURN_VALUE(ret, "STM reset failed");
    }
    if( emode == TBOOT) {
        LOGDEBUG("Ioctl call to send STM to boot mode\n");
                ret = motosh_ioctl(devFd,MOTOSH_IOCTL_TEST_BOOTMODE, &temp);
                CHECK_RETURN_VALUE(ret, "STM not in bootloader mode");
    }
    if( emode == TREAD) {
        if( argc < 4 )
            help(1);
        LOGDEBUG("Test read\n");
        // get the register to read from
                stm_convertAsciiToHex(argv[2],hexinput,strlen(argv[2]));
        LOGDEBUG( "%02x: ", hexinput[0]);
                ret = motosh_ioctl(devFd,MOTOSH_IOCTL_TEST_WRITE,hexinput);

        // get the number of bytes to be read
        stm_convertAsciiToHex(argv[3],hexinput,strlen(argv[3]));
        LOGDEBUG( "count = %02x: \n ", hexinput[0]);

        for( i= 0; i< hexinput[0]; i++) {
            ret = motosh_ioctl(devFd,MOTOSH_IOCTL_TEST_READ, &temp);
            LOGDEBUG( "%02x ", ret);
        }
    }
    if( emode == TWRITE) {
        LOGDEBUG(" Test write\n");
        for( i=0; i< (argc-2); i++) {
            stm_convertAsciiToHex(argv[i+2],hexinput,strlen(argv[i+2]));
            ret = motosh_ioctl(devFd,MOTOSH_IOCTL_TEST_WRITE,hexinput);
            if (ret >= 0) {
                LOGDEBUG( "%02x", hexinput[0]);
            } else {
                LOGDEBUG( "TWrite Error %02x\n", ret);
            }
        }
    }
    if( emode == TMWRITE) {
        count = argc-2;
        LOGDEBUG(" Writing data: ");
        for( i=0; i< count; i++) {
            stm_convertAsciiToHex(argv[i+2],hexinput+i,strlen(argv[i+2]));
            LOGDEBUG(" %02x",hexinput[i]);
        }
        LOGDEBUG("\n");
        ret = write(devFd,hexinput,count);
        if( ret != count) {
            LOGDEBUG("Write FAILED\n");
        }
    }
    if( emode == TMWRRD) {
        if( argc < 4 )
            help(1);
        LOGDEBUG( " Read from address ");
        stm_convertAsciiToHex(argv[2],hexinput,strlen(argv[2]));
        LOGDEBUG (" %02x, ",hexinput[0]);
        stm_convertAsciiToHex(argv[3],hexinput+1,strlen(argv[3]));
        LOGDEBUG (" %02x bytes: \n",hexinput[1]);
        ret = motosh_ioctl(devFd,MOTOSH_IOCTL_TEST_WRITE_READ,hexinput);
    }
    if( emode == VERSION) {
        stm_versionCheck();
    }
    if( emode == DEBUG ) {
        if( argc < 3 )
            help(1);
        LOGDEBUG( " Set debug to ");
        stm_convertAsciiToHex(argv[2],hexinput,strlen(argv[2]));
        delay = hexinput[0];
        LOGDEBUG(" %d\n", delay);
        ret = motosh_ioctl(devFd,MOTOSH_IOCTL_SET_DEBUG,&delay);

        const char * const files[] = {
            #ifdef MODULE_stml0xx
                "stml0xx",          // the prefix for each file
                "spi", "led",       // stml0xx specific files
            #else
                "motosh",           // the prefix for each file
                "time", "antcap",   // motosh specific files
            #endif
            "core", "flash", "ioctl", "irq", "queue", "reset", "wake_irq", "display"
        };

        char cmd[255];
        for (unsigned int i = 1; i < ARRAY_SIZE(files); ++i) {
            snprintf(cmd, sizeof(cmd), "echo 'file %s_%s.c %sp' > /sys/kernel/debug/dynamic_debug/control",
                files[0], files[i], delay ? "+" : "-");
            printf("Executing: %s\n", cmd);
            system(cmd);
        }
    }
    if( emode == FACTORY ) {
        LOGDEBUG( "Switching to factory mode\n");
        ret = motosh_ioctl(devFd,MOTOSH_IOCTL_SET_FACTORY_MODE, &temp);
    }
    if( emode == INVALID ) {
        LOGERROR("Invalid arguements passed: %d, %s\n",argc,argv[1])
        ret = STM_FAILURE;
    }
    if (emode == READWRITE) {
        //                    1B      2B       2B    ...
        // motosh readwrite [type] [address] [size] [data]
        //
        // read version example: motosh readwrite 00 00 01 00 01
        // write example:        motosh readwrite 01 00 0D 00 02 CC DD

        unsigned int arg_index = STM_MAX_GENERIC_COMMAND_LEN;
        unsigned char data_header[STM_MAX_GENERIC_HEADER];
        unsigned char *data_ptr;
        int result;

        if (argc < 7)
            help(1);

        // read in the header 2 bytes address, 2 bytes data_size
        LOGDEBUG(" Header Input: ");
        for( i=0; i < STM_MAX_GENERIC_HEADER; i++) {
            result = stm_convertAsciiToHex(argv[arg_index],data_header+i,
                    strlen(argv[arg_index]));
            if (result != 1) {
                printf("Header Input: stm_convertAsciiToHex failure\n");
                goto EXIT;
            }
            LOGDEBUG(" %02x",data_header[i]);
            arg_index++;
        }

        // read_write, 0 = read, 1 = write
        unsigned int read_write = atoi(argv[2]);
        int addr = (data_header[0] << 8) | data_header[1];
        int data_size = (data_header[2] << 8) | data_header[3];

        if (data_size > STM_MAX_GENERIC_DATA - 1) {
            printf("Data size too large, must be <= %d\n", STM_MAX_GENERIC_DATA - 1);
            goto EXIT;
        } else if (data_size <= 0) {
            printf("Data size invalid,\n");
            goto EXIT;
        } else if (read_write && data_size != (argc - STM_MAX_GENERIC_COMMAND_LEN
                 - STM_MAX_GENERIC_HEADER)) {
            printf("Not enough data provided,\n");
            goto EXIT;
        }

        // allocate data_ptr with read/write size + header
        data_ptr = (unsigned char *)malloc(data_size + STM_MAX_GENERIC_HEADER);
        memset(data_ptr, 0, data_size + STM_MAX_GENERIC_HEADER);

        // copy header into data_ptr
        int data_index = STM_MAX_GENERIC_HEADER;
        memcpy(data_ptr, data_header, STM_MAX_GENERIC_HEADER);

        // if writing, read in the data
        if (read_write) {
            LOGDEBUG(" READWRITE Data Input:");
            for( i=0; i < data_size; i++) {
                result = stm_convertAsciiToHex(argv[arg_index],
                    data_ptr + data_index,
                    strlen(argv[arg_index]));
                if (result != 1) {
                    printf("Data Input: stm_convertAsciiToHex failure\n");
                    free(data_ptr);
                    goto EXIT;
                }
                arg_index++;
                data_index++;
                LOGDEBUG(" %02x",data_ptr[i]);
            }
        }

        if (read_write) {
            ret = motosh_ioctl(devFd,MOTOSH_IOCTL_WRITE_REG,data_ptr);
            LOGDEBUG ("Writing data returned: %d", ret);
            printf ("Writing data returned: %d", ret);
        } else {
            ret = motosh_ioctl(devFd,MOTOSH_IOCTL_READ_REG,data_ptr);
            LOGDEBUG ("Read data (%d):", ret);
            printf ("Read data (%d):", ret);
            for ( i = 0; i < data_size; i++) {
                LOGDEBUG (" %02x", data_ptr[i]);
                printf (" %02x", data_ptr[i]);
            }
        }
        printf("\n");

        free(data_ptr);
    }
    if (emode == PASSTHROUGH) {
        // Passthrough command:
        //                     1B      1B         2B       1B     1B    ...
        // motosh passthrough [bus] [I2C addr] [reg addr] [r/w] [size] [data]
        //
        // read example:   motosh passthrough 00 d0 00 12 00 06
        // write example:  motosh passthrough 00 d0 00 42 01 01 28

        int result;

        if (argc < 6)
            help(1);

        // Allocate space for the command and response data
        unsigned char *pt_data;
        pt_data = (unsigned char *)malloc(MOTOSH_PASSTHROUGH_SIZE);
        if (pt_data == NULL) {
            printf("Passthrough memory allocation failure\n");
            goto EXIT;
        }

        // Convert the user input from ASCII to hex
        unsigned char *data_ptr;
        data_ptr = pt_data;
        LOGDEBUG("Passthrough Input: ");
        for( i=2; i<argc; i++) {
            result = stm_convertAsciiToHex(argv[i], data_ptr,
                strlen(argv[i]));
            LOGDEBUG("%02x", *data_ptr);
            data_ptr++;
            if (result != 1) {
                printf("Passthrough Input: stm_convertAsciiToHex failure\n");
                free(pt_data);
                goto EXIT;
            }
        }

        // read_write, 0 = read, 1 = write
        unsigned int read_write = pt_data[4];
        unsigned int data_size = pt_data[5];

        if (read_write == 0) {
            if (data_size > MOTOSH_PASSTHROUGH_SIZE - 1) {
                printf("Passthrough data size too large\n");
                goto EXIT;
            }
        } else {
            if (data_size > MOTOSH_PASSTHROUGH_SIZE - 6) {
                printf("Passthrough data size too large\n");
                goto EXIT;
            }
        }

        // Pass the command to the hub
        ret = motosh_ioctl(devFd, MOTOSH_IOCTL_PASSTHROUGH, pt_data);
        if (ret != 0) {
            LOGDEBUG ("Command failed: %d", ret);
            printf ("Command failed: %d\n", ret);
            free(pt_data);
            goto EXIT;
        }

        LOGDEBUG("Passthrough Output: ");
        if (read_write == 0) {
            for (unsigned i = 0; i < data_size; i++) {
                if (i != 0) {
                    printf (" ");
                }
                LOGDEBUG ("%02x", pt_data[i]);
                printf ("%02x", pt_data[i]);
            }
            printf("\n");
        }

        free(pt_data);
    }
    if(emode == LOWPOWER_MODE) {
        if( argc < 3 )
            help(1);
        unsigned int setting = atoi(argv[2]);
        if (setting == 0 || setting == 1) {
            LOGINFO(" lowpower mode set to: %d\n", setting);
            ret = motosh_ioctl(devFd, MOTOSH_IOCTL_SET_LOWPOWER_MODE, &setting);
        } else {
            LOGERROR(" lowpower mode incorrect setting\n");
            ret = STM_FAILURE;
        }
    }
    if(emode == MASS_ERASE_PART) {
        LOGDEBUG("Ioctl call to switch to bootloader mode\n");
        ret = motosh_ioctl(devFd, MOTOSH_IOCTL_BOOTLOADERMODE, &temp);
        CHECK_RETURN_VALUE(ret,"Failed to switch STM to bootloader mode\n");

        LOGDEBUG("Ioctl call to erase flash on STM\n");
        ret = motosh_ioctl(devFd, MOTOSH_IOCTL_MASSERASE, &temp);
        CHECK_RETURN_VALUE(ret,"Failed to erase STM \n");
        LOGINFO("Erased.\n");

    }

EXIT:
    if( ret < STM_SUCCESS)
        LOGERROR("Command execution error\n")
    close(devFd);
    if( filep != NULL)
        fclose(filep);
    return ret;
}

