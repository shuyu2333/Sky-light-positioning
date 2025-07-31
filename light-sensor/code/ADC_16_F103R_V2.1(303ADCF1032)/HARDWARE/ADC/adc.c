 #include "adc.h"
 #include "delay.h"
 #include "DMA.h"


/*****ͨ����Ӧ��ϵ��CH0--PA0��CH1--PA1��CH2--PA2��CH3--PA3��CH4--PA4��CH5--PA5��CH6--PA6��CH7--PA7��
                    CH8--PB0��CH9--PB1, CH10--PC0,CH11--PC1,CH12--PC2,CH13--PC3,CH14--PC4,CH15--PC5****/


__IO u16 ADC_ConvertedValue[Sample_Num][Channel_Num];       //��ά���飬�������ADCת�������Ҳ��DMA��Ŀ���ַ


void  Adc_Init(void)
{ 	
	ADC_InitTypeDef ADC_InitStructure; 
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC|RCC_APB2Periph_ADC1, ENABLE );	  //ʹ��GPIOA,GPIOB,GPIOC��ADC1ͨ��ʱ��
 

	RCC_ADCCLKConfig(RCC_PCLK2_Div6);   //����ADC��Ƶ����6 72M/6=12,ADC���ʱ�䲻�ܳ���14M

	                       
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;   //PA0-PA7 ��Ϊģ��ͨ����������  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		//ģ����������
	GPIO_Init(GPIOA, &GPIO_InitStructure);	        //��ʼ��GPIOA
	                        
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1;         //PB0-PB1 ��Ϊģ��ͨ����������  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		            //ģ����������
	GPIO_Init(GPIOB, &GPIO_InitStructure);                      //��ʼ��GPIOB
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5;   //PC0-PC5 ��Ϊģ��ͨ����������  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		//ģ����������
	GPIO_Init(GPIOC, &GPIO_InitStructure);	        //��ʼ��GPIOC
	
	
	MYDMA_Config();    //DMA1��ʼ��
	
	
	ADC_DeInit(ADC1);  //��λADC1 
	
	
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	//ADC����ģʽ:ADC1��ADC2�����ڶ���ģʽ
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;	//ģ��ת��������ɨ��ģʽ
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;	//ģ��ת������������ת��ģʽ
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//ת��������������ⲿ��������
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	//ADC�����Ҷ���
	ADC_InitStructure.ADC_NbrOfChannel =Channel_Num;	//˳����й���ת����ADCͨ������Ŀ
	ADC_Init(ADC1, &ADC_InitStructure);	//����ADC_InitStruct��ָ���Ĳ�����ʼ������ADCx�ļĴ���   
	
	
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_239Cycles5 );         //ADC1,ADC ͨ�� x,�������˳��ֵΪ y,����ʱ��Ϊ 239.5 ���ڣ���ͬ
	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 2, ADC_SampleTime_239Cycles5 );
	ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 3, ADC_SampleTime_239Cycles5 );
	ADC_RegularChannelConfig(ADC1, ADC_Channel_3, 4, ADC_SampleTime_239Cycles5 );
	ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 5, ADC_SampleTime_239Cycles5 );
	ADC_RegularChannelConfig(ADC1, ADC_Channel_5, 6, ADC_SampleTime_239Cycles5 );
	ADC_RegularChannelConfig(ADC1, ADC_Channel_6, 7, ADC_SampleTime_239Cycles5 );
	ADC_RegularChannelConfig(ADC1, ADC_Channel_7, 8, ADC_SampleTime_239Cycles5 );
	ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 9, ADC_SampleTime_239Cycles5 );
	ADC_RegularChannelConfig(ADC1, ADC_Channel_9, 10, ADC_SampleTime_239Cycles5 );
	ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 11, ADC_SampleTime_239Cycles5 );
	ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 12, ADC_SampleTime_239Cycles5 );
	ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 13, ADC_SampleTime_239Cycles5 );
	ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 14, ADC_SampleTime_239Cycles5 );
	ADC_RegularChannelConfig(ADC1, ADC_Channel_14, 15, ADC_SampleTime_239Cycles5 );
	ADC_RegularChannelConfig(ADC1, ADC_Channel_15, 16, ADC_SampleTime_239Cycles5 ); 
	
	
	
	
	ADC_DMACmd(ADC1, ENABLE);  //����ADC��DMA֧��
	
  ADC_Cmd(ADC1, ENABLE);	//ʹ��ָ����ADC1
	
	
	ADC_ResetCalibration(ADC1);	//ʹ�ܸ�λУ׼  
	 
	while(ADC_GetResetCalibrationStatus(ADC1));	//�ȴ���λУ׼����
	
	ADC_StartCalibration(ADC1);	 //����ADУ׼
 
	while(ADC_GetCalibrationStatus(ADC1)); //�ȴ�У׼����
 
  ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//ʹ��ָ����ADC1�����ת����������

}		





u16 ReadADCAverageValue(u8 Channel)            //ÿ��ͨ��������ȡ��10�Σ����ƽ��ֵ
{ 
    u8 i;
	  u32 sum=0;
    for(i=0; i<Sample_Num; i++)
    { 
     sum+=ADC_ConvertedValue[i][Channel]; 
    } 
    return (sum/Sample_Num); 
}



























