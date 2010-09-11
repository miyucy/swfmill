#include "b64/cdecode.h"
#include "b64/cencode.h"

long
base64_decode(char *to, char *from, unsigned int len)
{
    base64_decodestate state;
    base64_init_decodestate(&state);
    return base64_decode_block(from, len, to, &state);
}

long
base64_encode(char *to, char *from, unsigned int len)
{
    base64_encodestate state;
    base64_init_encodestate(&state);
    return base64_encode_block(from, len, to, &state) + base64_encode_blockend(to, &state);
}
