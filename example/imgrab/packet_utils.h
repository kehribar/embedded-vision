// ----------------------------------------------------------------------------
// 
// 
// ----------------------------------------------------------------------------
#include <stdint.h>

// ----------------------------------------------------------------------------
static uint32_t make32b(uint8_t* buff, uint32_t base)
{
  uint32_t rv;

  rv  = (uint32_t)(buff[base + 3]) <<  0;
  rv += (uint32_t)(buff[base + 2]) <<  8;
  rv += (uint32_t)(buff[base + 1]) << 16;
  rv += (uint32_t)(buff[base + 0]) << 24;

  return rv;
}

// ----------------------------------------------------------------------------
static uint16_t make16b(uint8_t* buff, uint32_t base)
{
  uint16_t rv;

  rv  = (uint16_t)(buff[base + 1]) <<  0;
  rv += (uint16_t)(buff[base + 0]) <<  8;

  return rv;
}

// ----------------------------------------------------------------------------
static void put32b(uint8_t* buff, uint32_t base, uint32_t data)
{
  buff[base + 3] = (data & 0x000000FF) >> 0;
  buff[base + 2] = (data & 0x0000FF00) >> 8;
  buff[base + 1] = (data & 0x00FF0000) >> 16;
  buff[base + 0] = (data & 0xFF000000) >> 24;
}
