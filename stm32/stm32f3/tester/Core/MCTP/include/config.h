#ifndef CONFIG_H
#define CONFIG_H

/* Define STM32 family here */
#define STM32F3

#if defined(STM32F1)
#include "stm32f1xx_hal.h"
#elif defined(STM32F3)
#include "stm32f3xx_hal.h"
#elif defined(STM32F4)
#include "stm32f4xx_hal.h"
/* TODO: Add more families */
#else
#error "Unsupported STM32 family. Provide a valid macro from supported familes."
#endif

#endif
