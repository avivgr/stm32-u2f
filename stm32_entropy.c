
#include "stm32f4xx_hal.h"

int mbedtls_hardware_poll( void *data,
	unsigned char *output,
	size_t len,
	size_t *olen)

{
	int i;
	HAL_StatusTypeDef ret;
	//printf("%s\n", __func__);
	
	*olen = 0;
	
	for (i = 0; i < len / 4; i++, output+=4) {
		ret = HAL_RNG_GenerateRandomNumber(&hrng, (uint32_t *)output);
		if (ret != HAL_OK)
			return 0;
	}
	
	if (len % 4 != 0) {
		uint32_t last;
		ret = HAL_RNG_GenerateRandomNumber(&hrng, &last);
		if (ret != HAL_OK)
			return 0;
		for (i = 0; i < len % 4; i++, output += 1) {
			*output = (unsigned char)(last & 0xff);
			last = last >> 8;
		}
	}
		
	*olen = len;
	
	return 0;
}