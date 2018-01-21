
#include "SensorHub.hpp"

using namespace std;

namespace mot {
#if defined(MOTOSH) || defined(MODULE_motosh)
    #define VMM_ENTRY(reg, name, writable, addr, size) {#name, reg},
    const map<string, uint16_t> SensorHub::Vmm = {
        #include "linux/motosh_vmm.h"
    };
    #undef VMM_ENTRY
#elif defined(STML0XX) || defined(MODULE_stml0xx)
    #define VMM_ENTRY(reg, name, writable, addr, size) {#name, reg},
    const map<string, uint16_t> SensorHub::Vmm = {
        #include "linux/stml0xx_vmm.h"
    };
    #undef VMM_ENTRY
#endif

int SensorHub::retryIoctl (int fd, int ioctl_number, ...) {
    va_list ap;
    void * arg;
    int status = 0;
    int error = 0;

    if (fd < 0) return fd;

    va_start(ap, ioctl_number);
    arg = va_arg(ap, void *);
    va_end(ap);

    do {
        status = ioctl(fd, ioctl_number, arg);
        error = errno;
    } while ((status != 0) && (error == -EINTR));
    return status;
}

int16_t SensorHub::getRegisterNumber(const string regName) {
    auto regEntry = Vmm.find(regName);
    if (regEntry == Vmm.end()) {
        return -1;
    }
    return static_cast<int16_t>(regEntry->second);
}


unique_ptr<uint8_t[]> SensorHub::readReg(VmmID vmmId, uint16_t size) {
    if (fd < 0 || size > getMaxRx()) return nullptr;

    unique_ptr<uint8_t[]> res(new uint8_t[ max<uint16_t>(size, SENSORHUB_CMD_LENGTH) ]);

    uint16_t regNr = htons(static_cast<uint16_t>(vmmId));
    uint16_t dataSize = htons(size);
    memcpy(res.get(), &regNr, 2);
    memcpy(res.get() + 2, &dataSize, 2);

    int ret = retryIoctl(fd, SH_IOCTL_READ_REG, res.get());
    if (ret < 0) {
        return nullptr;
    } else {
        return res;
    }
}


unique_ptr<uint8_t[]> SensorHub::readReg(const string regName, uint16_t size) {
    auto regEntry = Vmm.find(regName);
    if (regEntry == Vmm.end()) {
        return nullptr;
    }

    return readReg(static_cast<VmmID>(regEntry->second), size);
}

bool SensorHub::writeReg(VmmID vmmId, uint16_t size,
        const uint8_t * const data) {

    if (data == nullptr || size == 0 || size > getMaxTx()) return false;

    uint16_t regNr = htons(static_cast<uint16_t>(vmmId));
    uint16_t bytesLength = htons(size);

    char msg[SENSORHUB_CMD_LENGTH + size];
    memcpy(msg, &regNr, 2);
    memcpy(msg + 2, &bytesLength, 2);
    memcpy(msg + 4, data, size);

    int res = retryIoctl(fd, SH_IOCTL_WRITE_REG, msg);
    return res >= 0;
}

bool SensorHub::writeReg(string regName, uint16_t size,
        const uint8_t * const data) {
    int16_t regNr = getRegisterNumber(regName);
    if (regNr < 0) return false;

    return writeReg(static_cast<VmmID>(regNr), size, data);
}

string SensorHub::getVariant(void) {
    char variantStr[FW_VERSION_SIZE] = {0};
    int res = retryIoctl(fd, SH_IOCTL_GET_VERNAME, variantStr);
    return res < 0 ? string() : variantStr;
}

string SensorHub::getVersionStr(void) {
    unique_ptr<uint8_t[]> verStr;

    unique_ptr<uint8_t[]> buff = readReg(VmmID::FW_VERSION_LEN, 1);
    if (!buff) return string();

    // Make sure we don't read more than the physical layer packet size.
    uint8_t strLen = min<size_t>(buff[0], getMaxRx());
    if (!strLen) return string();

    verStr = readReg(VmmID::FW_VERSION_STR, strLen);
    // verStr has no \0 terminator, so we must specify the string length.
    return verStr ? string((char *)verStr.get(), strLen) : string();
}

uint32_t SensorHub::getFlashCrc(void) {
    unique_ptr<uint8_t[]> buff = readReg(VmmID::FW_CRC, 4);
    if (!buff) return 0;

    uint32_t hwCrc = Endian::extract<uint32_t>(&buff[0]);
    if (! isBigEndian) {
        // Extraction was done assuming data was BE, so we must swap.
        hwCrc = Endian::swap(hwCrc);
    }

    return hwCrc;
}

bool SensorHub::triggerProxRecal(void) {
    static const uint8_t prox_recal_command[1] = {0xB1};
    return writeReg(VmmID::BYPASS_MODE, sizeof(prox_recal_command),
            prox_recal_command);
}

} // namespace mot

