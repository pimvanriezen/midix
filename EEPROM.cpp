#include "EEPROM.h"

// ==========================================================================
// CLASS EEPROMHandler
// ==========================================================================
EEPROMHandler::EEPROMHandler (void) {
    i2cid = 0x50;
}

// --------------------------------------------------------------------------
EEPROMHandler::~EEPROMHandler (void) {
}

// --------------------------------------------------------------------------
void EEPROMHandler::setAddress (uint8_t i2caddr) {
    i2cid = i2caddr;
}

// --------------------------------------------------------------------------
uint8_t EEPROMHandler::valid (uint32_t appid, uint8_t vmajor, 
                              uint8_t vminor) {
    EEPROMPage *pg = load (0);
    uint32_t *aptr = (uint32_t *) pg->data;
    if (! pg) return EEPROM_INVALID;
    if (*aptr != appid) {
        delete pg;
        return EEPROM_INVALID;
    }
    maxpage = (pg->data[6] << 8) | pg->data[7];
    if ((pg->data[4] != vmajor) || (pg->data[5] != vminor)) {
        delete pg;
        return EEPROM_DIFFVERSION;
    }
    delete pg;
    return EEPROM_VALID;
}

// --------------------------------------------------------------------------
void EEPROMHandler::init (uint32_t appid, uint8_t vmajor, uint8_t vminor,
                          uint16_t numpages) {
    EEPROMPage p;
    uint32_t *aptr = (uint32_t *) p.data;
    *aptr = appid;
    p.data[4] = vmajor;
    p.data[5] = vminor;
    p.data[6] = (numpages & 0xff) >> 8;
    p.data[7] = numpages & 0xff;
    write (0, p.data, 64);
}

// --------------------------------------------------------------------------
EEPROMPage *EEPROMHandler::load (uint16_t page) {
    if (page > maxpage) return NULL;
    EEPROMPage *res = new EEPROMPage;
    if (! I2C.getBytes (i2cid, page * 64, res->data, 64)) {
        delete res;
        return NULL;
    }
    return res;
}

// --------------------------------------------------------------------------
void EEPROMHandler::write (uint16_t page, void *vdata, uint8_t sz) {
    if (page > maxpage) return;
    if (sz > 64) return;
    uint8_t *data = (uint8_t *) vdata;
    I2C.beginWrite (i2cid, page * 64);
    for (uint8_t i=0; i<sz; ++i) {
        I2C.write (data[i]);
    }
    I2C.endWrite();
}

// --------------------------------------------------------------------------
EEPROMHandler EEPROM;
