#include "Endian.hpp"

const Endian::TestUnion Endian::test = { 0x01020408 };
const bool Endian::bigEndian = Endian::test.v[0] == 1;

