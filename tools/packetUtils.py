#------------------------------------------------------------------------------
# 
# 
#------------------------------------------------------------------------------
import ctypes

#------------------------------------------------------------------------------
def make48b(buf):
  buf = bytearray(buf)
  outp  = (buf[0] << 40)
  outp += (buf[1] << 32)
  outp += (buf[2] << 24)
  outp += (buf[3] << 16)
  outp += (buf[4] <<  8)
  outp += (buf[5] <<  0)
  return outp

#------------------------------------------------------------------------------
def put48b(inp):
  buf = [0] * 6
  buf[0] = (inp >> 40) & 0xFF
  buf[1] = (inp >> 32) & 0xFF
  buf[2] = (inp >> 24) & 0xFF
  buf[3] = (inp >> 16) & 0xFF
  buf[4] = (inp >>  8) & 0xFF
  buf[5] = (inp >>  0) & 0xFF
  return buf

#------------------------------------------------------------------------------
def convertToSigned_48b(inp):
  inp = inp << 16
  rv = ctypes.c_int64(inp).value
  return (rv >> 16)

#------------------------------------------------------------------------------
def convertToSigned_32b(inp):
  rv = ctypes.c_int32(inp).value
  return rv

#------------------------------------------------------------------------------
def make32b(buf):
  buf = bytearray(buf)
  outp  = (buf[0] << 24)
  outp += (buf[1] << 16)
  outp += (buf[2] <<  8)
  outp += (buf[3] <<  0)
  return outp

#------------------------------------------------------------------------------
def put32b(inp):
  buf = [0] * 4
  buf[0] = (inp >> 24) & 0xFF
  buf[1] = (inp >> 16) & 0xFF
  buf[2] = (inp >>  8) & 0xFF
  buf[3] = (inp >>  0) & 0xFF
  return buf

#------------------------------------------------------------------------------
def convertToSigned_32b(inp): 
  rv = ctypes.c_int32(inp).value
  return rv

#------------------------------------------------------------------------------
def make16b(buf):
  buf = bytearray(buf)
  outp  = (buf[0] << 8)
  outp += (buf[1] << 0)
  return outp

#------------------------------------------------------------------------------
def put16b(inp):
  buf = [0] * 2
  buf[0] = (inp >> 8) & 0xFF
  buf[1] = (inp >> 0) & 0xFF
  return buf

#------------------------------------------------------------------------------
def convertToSigned_16b(inp): 
  rv = ctypes.c_int16(inp).value
  return rv

#------------------------------------------------------------------------------
if __name__ == '__main__':
  test_val = 281474976710650

  print(test_val)
  
  rv = put48b(test_val)
  print(rv)

  rv = make48b(rv)
  print(rv)

  rv = convertToSigned_48b(rv)
  print(rv)

  test_val = -32700

  print(test_val)
  
  rv = put16b(test_val)
  print(rv)

  rv = make16b(rv)
  print(rv)

  rv = convertToSigned_16b(rv)
  print(rv)
