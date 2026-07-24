#include "py/runtime.h"
#include "py/objarray.h"

#include <stdint.h>

static mp_obj_t cobs_encode(mp_obj_t src_in, mp_obj_t dst_in)
{
	mp_buffer_info_t src_buf;
	mp_buffer_info_t dst_buf;
    
	// Get raw buffer access
	mp_get_buffer_raise(src_in, &src_buf, MP_BUFFER_READ);
	mp_get_buffer_raise(dst_in, &dst_buf, MP_BUFFER_WRITE);   
    
	// Check sizes
	size_t required = src_buf.len + (src_buf.len / 254) + 2;

	if (dst_buf.len < required)
	{
    		mp_raise_ValueError(MP_ERROR_TEXT("destination too small"));
	}
    
	size_t length = src_buf.len;

	uint8_t* data   = (uint8_t*)src_buf.buf;
	uint8_t* buffer = (uint8_t*)dst_buf.buf;
    
    
	uint8_t *encode = buffer; // Encoded byte pointer
	uint8_t *codep = encode++; // Output code pointer
	uint8_t code = 1; // Code value

	for (const uint8_t *byte = (const uint8_t *)data; length--; ++byte)
	{
		if (*byte) // Byte not zero, write it
			*encode++ = *byte, ++code;

		if (!*byte || code == 0xff) // Input is zero or block completed, restart
		{
			*codep = code, code = 1, codep = encode;
			if (!*byte || length)
				++encode;
		}
	}
	*codep = code; // Write final code value

	return mp_obj_new_int_from_uint((mp_uint_t)(encode-buffer));
}



static mp_obj_t cobs_decode(mp_obj_t src_in, mp_obj_t src_len, mp_obj_t dst_in)
{
	mp_buffer_info_t src_buf;
	mp_buffer_info_t dst_buf;

	// Get raw buffer access
	mp_get_buffer_raise(src_in, &src_buf, MP_BUFFER_READ);
	mp_get_buffer_raise(dst_in, &dst_buf, MP_BUFFER_WRITE);
	mp_int_t src_buf_length = mp_obj_get_int(src_len);

	// Check sizes
	if (src_buf.len < src_buf_length)
	{
		mp_raise_ValueError(MP_ERROR_TEXT("src_len is greater than source buffer size"));
	}
	
	
	size_t length = src_buf_length;
	
	if (dst_buf.len < src_buf_length)
	{
		mp_raise_ValueError(MP_ERROR_TEXT("destination too small"));
	}

	
	
	uint8_t* buffer = (uint8_t*)src_buf.buf;
	uint8_t* data   = (uint8_t*)dst_buf.buf;

	uint8_t *byte = buffer; // Encoded input byte pointer
	uint8_t *decode = (uint8_t *)data; // Decoded output byte pointer

	for (uint8_t code = 0xff, block = 0; byte < buffer + length; --block)
	{
		if (block) // Decode block byte
			*decode++ = *byte++;
		else
		{
			block = *byte++;             // Fetch the next block length
			if (block && (code != 0xff)) // Encoded zero, write it unless it's delimiter.
				*decode++ = 0;
			code = block;
			if (!code) // Delimiter code found
				break;
		}
	}

	return mp_obj_new_int_from_uint((mp_uint_t)(decode - data));
}







static MP_DEFINE_CONST_FUN_OBJ_2(cobs_encode_obj, cobs_encode);
static MP_DEFINE_CONST_FUN_OBJ_3(cobs_decode_obj, cobs_decode);

static const mp_rom_map_elem_t cobs_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_cobs) },
    { MP_ROM_QSTR(MP_QSTR_encode), MP_ROM_PTR(&cobs_encode_obj) },
    { MP_ROM_QSTR(MP_QSTR_decode), MP_ROM_PTR(&cobs_decode_obj) },
};

static MP_DEFINE_CONST_DICT(
    cobs_globals,
    cobs_globals_table
);

const mp_obj_module_t cobs_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&cobs_globals,
};

MP_REGISTER_MODULE(MP_QSTR_cobs, cobs_user_cmodule);

