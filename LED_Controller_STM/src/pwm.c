

#include "stm32f1xx.h"
#include "stm32f1xx_hal.h"


extern  uint8_t data_c0[4][3072]; // 4 Clockless channels of either 96 RGBW or 128 RGB leds
extern  uint8_t data_c1[512];  // 1 Clocked channels of 96 LEDS

uint32_t bytecount = 0;

void NextBit() {

	if (bytecount < 72) {
		// Set output
		TIM2->CCR1 = data_c0[0][bytecount];

		// More channels for later, for now, 2 channels seem ok, 4 is too much
		// We need DMA after all
		// So compressed data on usb might be doable after all
		//TIM2->CCR2 = data_c0[1][bytecount];
		//TIM2->CCR3 = data_c0[2][bytecount];
		//TIM2->CCR4 = data_c0[3][bytecount];

		//trace_printf("%d", TIM2->CCR1 > 3);
	} else {
		bytecount = 0;
		//TIM2->CR1 &=~0x0001; // disable
	}
	bytecount++;
}


// http://electronics.stackexchange.com/questions/116429/can-not-jump-to-timer-interrupt-function-in-stm32f4-discoverty

void TIM2_IRQHandler (void) {

	if (TIM2->SR &0b01) {
		NextBit();
	}
	TIM2->SR &=~TIM2->SR;

}


void pins_init() {
  GPIO_InitTypeDef   GPIO_InitStruct;


  // Enable Timer 2 Clock
  __HAL_RCC_TIM2_CLK_ENABLE();

  // Enable GPIO Port A Clock
  __HAL_RCC_GPIOA_CLK_ENABLE();


  // Common configuration for all channels
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

  // Apply pin configuration to PA0
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  // Apply pin configuration to PA1
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  // Apply pin configuration to PA2
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  // Apply pin configuration to PA3
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}


void pwm_init() {

	pins_init();

	NVIC->ISER[0] |= 0x10000000;
	RCC->APB1ENR |= 1;
	//TIM2->CR1 = 0x0010;
	//TIM2->ARR = 0x8000;
	//TIM2->CR2 = 0;
	//TIM2->SMCR = 0;
	//TIM2->DIER = 0x0001;
	//TIM2->CR1 |= 0x0001;
	//

	//TIM2->DIER = 0x0010; // interrupt enable cc1
	TIM2->DIER = 0x0001; // interrupt enable c

	RCC->APB1ENR |= 1;

/*
	TIM2->ARR = 10 ; // Reload Value
	TIM2->PSC =  9 ; // Prescaler
*/

	TIM2->ARR = 8 ; // Reload Value
	TIM2->PSC = 15 ; // Prescaler


	TIM2->CCMR1 = ( TIM2->CCMR1  & ~(0b11110000) ) | (0b1101 << 3);  // Set Channel 1 to PWM mode 1 and enabling reload


	TIM2->CR1 |= 1 << 7; // auto reload enable

	TIM2->EGR |= 1; // Set UG bit

	TIM2->CR1 &= ~(0b1110000); // Edge aglined, upcounting
	TIM2->CR1 |= 0b100; // Event source, only over/underflow


	TIM2->CCER |= 0b1;  // output enable and polarity

	TIM2->CCR1 = 0; // output val

	TIM2->CR1 |= 0x0001; // enable




	  NVIC_ClearPendingIRQ(TIM2_IRQn);
	  NVIC_EnableIRQ(TIM2_IRQn);

}
