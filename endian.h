#ifndef __ENDIAN_H__
#define __ENDIAN_H__

#ifdef __ARMEB__
#define cpu_to_be32(x) (x)
#define cpu_to_be16(x) (x)
#define be16_to_cpu(x) (x)
#define be32_to_cpu(x) (x)
#else

#define cpu_to_be32(x) ((uint32_t)(          \
	(((uint32_t)(x) & (uint32_t)0x000000ffUL) << 24) |  \
	(((uint32_t)(x) & (uint32_t)0x0000ff00UL) <<  8) |  \
	(((uint32_t)(x) & (uint32_t)0x00ff0000UL) >>  8) |  \
	(((uint32_t)(x) & (uint32_t)0xff000000UL) >> 24)))
 
#define cpu_to_be16(x) ((uint16_t)(          \
	(((uint16_t)(x) & (uint16_t)0x00ffU) << 8) |  \
	(((uint16_t)(x) & (uint16_t)0xff00U) >> 8)))

#define be32_to_cpu(x) ((uint32_t)(          \
	(((uint32_t)(x) & (uint32_t)0x000000ffUL) << 24) |  \
	(((uint32_t)(x) & (uint32_t)0x0000ff00UL) <<  8) |  \
	(((uint32_t)(x) & (uint32_t)0x00ff0000UL) >>  8) |  \
	(((uint32_t)(x) & (uint32_t)0xff000000UL) >> 24)))
 
#define be16_to_cpu(x) ((uint16_t)(          \
	(((uint16_t)(x) & (uint16_t)0x00ffU) << 8) |  \
	(((uint16_t)(x) & (uint16_t)0xff00U) >> 8)))

#endif
#endif