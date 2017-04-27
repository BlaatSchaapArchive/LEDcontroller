#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_dma.h"
#include "stm32f1xx_hal_tim.h"
#include <stdbool.h>

// Datasheet page 281
// TIM2_UP --> DMA Channel 2

void pins_init() {
	GPIO_InitTypeDef GPIO_InitStruct;

	// Enable Timer 2 Clock
	__HAL_RCC_TIM2_CLK_ENABLE()
	;

	// Enable GPIO Port A Clock
	__HAL_RCC_GPIOA_CLK_ENABLE()
	;

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

void pwm_init2(DMA_Channel_TypeDef *dma,
		TIM_TypeDef *tim){
	dma->CPAR = &(tim->DMAR); // DMA 1 Channel 2 to tim

		dma->CCR = 0x00;
		dma->CCR |= (0x01 << DMA_CCR_PSIZE_Pos); // Peripheral size 16 bit
		dma->CCR |= (0x00 << DMA_CCR_MSIZE_Pos); // Memory size 8 bit
		dma->CCR |= (0x1 << DMA_CCR_DIR_Pos);   // Memory to Peripheral
		dma->CCR |= (0x1 << DMA_CCR_MINC_Pos);   // Memory increasement
		dma->CCR |= (0x0 << DMA_CCR_PINC_Pos);  // Peripheral increasement
		dma->CCR |= (0x0 << DMA_CCR_CIRC_Pos);   // Circular mode
		dma->CCR |= DMA_CCR_TCIE; // Enable transfer complete interrupt

		dma->CCR |= DMA_CCR_TEIE; // Enable transfer error interrupt

		tim->DCR = 0;

		tim->DCR |= ((13) << TIM_DCR_DBA_Pos); // DMA Transfer Base address CCR1
		tim->DCR |= ((3) << TIM_DCR_DBL_Pos); // 4 Transfer at a time (CCR1 to CCR4)

		tim->ARR = 8; // Reload Value
		tim->PSC = 9; // Prescaler

		tim->DIER = 0;
		tim->DIER |= TIM_DIER_UDE; // Update DMA Request Enable

		tim->CCMR1 = 0;
		tim->CCMR1 |= (0b110 << TIM_CCMR1_OC1M_Pos); // pwm mode 1
		tim->CCMR1 |= (0b110 << TIM_CCMR1_OC2M_Pos); // pwm mode 1

		tim->CCMR2 = 0;
		tim->CCMR2 |= (0b110 << TIM_CCMR2_OC3M_Pos); // pwm mode 1
		tim->CCMR2 |= (0b110 << TIM_CCMR2_OC4M_Pos); // pwm mode 1

		tim->CCMR1 |= (1 << TIM_CCMR1_OC1PE_Pos); // output compare preload enable
		tim->CCMR1 |= (1 << TIM_CCMR1_OC2PE_Pos); // output compare preload enable

		tim->CCMR2 |= (1 << TIM_CCMR2_OC3PE_Pos); // output compare preload enable
		tim->CCMR2 |= (1 << TIM_CCMR2_OC4PE_Pos); // output compare preload enable

		// This I forgot
		tim->CCER = TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E | TIM_CCER_CC4E;

		tim->CR1 |= 1 << 7; // auto reload enable
		tim->CR1 &= ~(0b1110000); // Edge aglined, upcounting
		tim->CR1 |= 0b100; // Event source, only over/underflow
		tim->CR1 |= 0x0001; // enable

}

void pwm_init() {

	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; // enable timer2 clock
	RCC->AHBENR |= RCC_AHBENR_DMA1EN; // enable dma1 clock

	pins_init();

	pwm_init2(DMA1_Channel2,TIM2);

	NVIC_ClearPendingIRQ(DMA1_Channel2_IRQn);
	NVIC_EnableIRQ(DMA1_Channel2_IRQn);


}


void start_dma_transer2(char* memory, size_t size, DMA_Channel_TypeDef *dma,
		TIM_TypeDef *tim) {
	dma->CNDTR = size;
	dma->CMAR = memory;
	tim->CCMR1 |= 1; // enable timer
	dma->CCR |= 1; // Enable DMA
	tim->EGR = TIM_EGR_UG; // Trigger update event, starting the transfer
}
void start_dma_transer(char* memory, size_t size) {
	start_dma_transer2(memory,size,DMA1_Channel2,TIM2);
}


bool is_busy2(DMA_Channel_TypeDef *dma) {
	return dma->CCR & 1;
}

bool is_busy() {
	return is_busy2(DMA1_Channel2);
	//return DMA1_Channel2->CCR & 1;
}

void DMA1_Channel2_IRQHandler(void) {
	DMA1->IFCR = DMA1->ISR;
	DMA1_Channel2->CCR &= ~1; // Disable DMA
}
