
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_dma.h"
#include "stm32f1xx_hal_tim.h"
#include <stdbool.h>



// Datasheet page 281
// TIM2_UP --> DMA Channel 2

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



		RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; // enable timer2 clock
		RCC->AHBENR |= RCC_AHBENR_DMA1EN; // enable dma1 clock

	pins_init();



	DMA1_Channel2->CPAR = &(TIM2->DMAR); // DMA 1 Channel 2 to TIM2

	DMA1_Channel2->CCR  = 0x00;
	DMA1_Channel2->CCR  |= 	(0x01 << DMA_CCR_PSIZE_Pos); // Peripheral size 16 bit
	DMA1_Channel2->CCR  |= 	(0x00 << DMA_CCR_MSIZE_Pos); // Memory size 8 bit
	DMA1_Channel2->CCR  |=  (0x1 << DMA_CCR_DIR_Pos);   // Memory to Peripheral
	DMA1_Channel2->CCR  |=  (0x1 << DMA_CCR_MINC_Pos);   // Memory increasement
	DMA1_Channel2->CCR  |=  (0x0 << DMA_CCR_PINC_Pos);   // Peripheral increasement
	DMA1_Channel2->CCR  |=  (0x0 << DMA_CCR_CIRC_Pos);   // Circular mode
	DMA1_Channel2->CCR |= DMA_CCR_TCIE; // Enable transfer complete interrupt

	DMA1_Channel2->CCR |= DMA_CCR_TEIE; // Enable transfer error interrupt



	TIM2->DCR = 0;

	TIM2->DCR |= (( 13 ) << TIM_DCR_DBA_Pos); // DMA Transfer Base address CCR1
	TIM2->DCR |= (( 3 ) << TIM_DCR_DBL_Pos); // 4 Transfer at a time (CCR1 to CCR4)



	TIM2->ARR = 8 ; // Reload Value
	TIM2->PSC = 9 ; // Prescaler

	TIM2->DIER = 0;
	TIM2->DIER |= TIM_DIER_UDE; // Update DMA Request Enable
	//TIM2->DIER |= TIM_DIER_CC1DE; // Update DMA Request Enable

	TIM2->CCMR1 = 0;
	TIM2->CCMR1 |= (0b110 << TIM_CCMR1_OC1M_Pos); // pwm mode 1
	TIM2->CCMR1 |= (0b110 << TIM_CCMR1_OC2M_Pos); // pwm mode 1


	TIM2->CCMR2 = 0;
	TIM2->CCMR2 |= (0b110 << TIM_CCMR2_OC3M_Pos); // pwm mode 1
	TIM2->CCMR2 |= (0b110 << TIM_CCMR2_OC4M_Pos); // pwm mode 1


	TIM2->CCMR1 |= (1 << TIM_CCMR1_OC1PE_Pos); // output compare preload enable
	TIM2->CCMR1 |= (1 << TIM_CCMR1_OC2PE_Pos); // output compare preload enable

	TIM2->CCMR2 |= (1 << TIM_CCMR2_OC3PE_Pos); // output compare preload enable
	TIM2->CCMR2 |= (1 << TIM_CCMR2_OC4PE_Pos); // output compare preload enable

	// This I forgot
	TIM2->CCER = TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E | TIM_CCER_CC4E;



	TIM2->CR1 |= 1 << 7; // auto reload enable
	TIM2->CR1 &= ~(0b1110000); // Edge aglined, upcounting
	TIM2->CR1 |= 0b100; // Event source, only over/underflow
	TIM2->CR1 |= 0x0001; // enable


//	 TIM2->CCR1 = 3; // output val <-- maybe add some val to test
	//TIM2->CR1 = 1;
	//TIM2->CR1 |=  TIM_CR1_URS;//;(1 << TIM_CR1_URS_Pos); // Only generate event on dma
}


void start_dma_transer(char* memory, size_t size) {

	NVIC_ClearPendingIRQ(DMA1_Channel2_IRQn);
	NVIC_EnableIRQ(DMA1_Channel2_IRQn);
	NVIC_EnableIRQ(TIM2_IRQn);

	DMA1_Channel2->CNDTR = size;
	DMA1_Channel2->CMAR = memory;


	TIM2->CCMR1 |= 1; // enable timer
	DMA1_Channel2->CCR |= 1; // Enable DMA

	TIM2->EGR = TIM_EGR_UG;
}


void TIM2_IRQHandler(void) {
	int i = 1;
}

void DMA1_Channel2_IRQHandler(void) {

	// Transfer complete

	// Clear interrupt // the crude way, clear everythinh
	// Should be changed once we support multiple channels.
	DMA1->IFCR = DMA1->ISR;

	//TIM2->CCMR1 &= ~1; // disable timer
	DMA1_Channel2->CCR &= ~1; // Disable DMA

}

bool is_busy() {
	return DMA1_Channel2->CCR & 1;
}
