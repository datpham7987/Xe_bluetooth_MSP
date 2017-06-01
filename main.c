#include <msp430.h>
   // Port_init();

 /*
Timer A0 CCR0: P1.1 or P1.5
Timer A0 CCR1: P1.2 or P1.6
Timer A0 CCR2: P3.0
Timer A1 CCR0: P2.0, P2.3, P3.1 or P3.4
Timer A1 CCR1: P2.1, P2.2, P3.2 or P3.5
Timer A1 CCR2: P2.4, P2.5, P3.3 or P3.6
 */
int mode; // quay thuận
int toi;
int lui;
void config_IO(void)
{
	// DONG CO 2
  P2DIR |= (BIT2+BIT3+BIT4+BIT5+BIT6); // P1.6:PWM1; P2.3:IN1;P2.5:IN2;(DC 1)
  P2OUT |= (BIT2);	  	  	  	  	    // P2.2:PWM2; P2.4:IN1;P2.6:IN2;(DC 2)
  P2SEL |= BIT2;
  P2SEL &=~(BIT3+BIT4+BIT5+BIT6);
  P2SEL2&=~(BIT3+BIT4+BIT5+BIT6);
  P2OUT &=~(BIT3+BIT4+BIT5+BIT6);
  // DONG CO 1
  P1DIR |= BIT6;
  P1OUT |= BIT6;
  P1SEL |= BIT6;
}
void DC_1_CW()
{
	  P1OUT |= BIT6;
	  P2OUT |= BIT3;
	  P2OUT &=~(BIT5);
}
void DC_2_CW()
{
	  P2OUT |= (BIT2+BIT4);
	  P2OUT &=~(BIT6);
}
void DC_1_CWC()
{
	  P1OUT |= BIT6;
	  P2OUT |= BIT5;
	  P2OUT &=~(BIT3);
}
void DC_2_CWC()
{
	  P2OUT |= (BIT2+BIT6);
	  P2OUT &=~(BIT4);
}

void DC_1_Short_Brake()
{
	P2OUT |= (BIT3+BIT5);
}
void DC_2_Short_Brake()
{
	P2OUT |= (BIT4+BIT6);
}

void DC_1_stop()
{
	P1OUT |= BIT6;
	P2OUT &=~(BIT3+BIT5);
}

void DC_2_stop()
{
	P2OUT &=~(BIT4+BIT6);
	P2OUT |= BIT2;
}

void config_PWM(void)
{
  TA1CTL = TASSEL_2 + MC_1; // SMCLK(1Mhz) // up mode
  TA1CCR0 =3000;
  TA1CCR1 = 2000;                              //modul timer
  TA1CCTL1 = OUTMOD_7; // chỉnh mode cho PWM //PWM reset/set */

  TA0CTL = TASSEL_2 + MC_1;
  TA0CCR0 = 3000;
  TA0CCR1 = 3000;
  TA0CCTL1 = OUTMOD_7;
}
void Config_Clocks(void)
{
	if(CALBC1_1MHZ ==0xFF || CALDCO_1MHZ == 0xFF) // cấp xung dao động nội(clock source)
	{
		while(1);
	}
		DCOCTL = 0;
		BCSCTL1 = CALBC1_1MHZ; // set range
		DCOCTL = CALDCO_1MHZ; //// Set DCO step + modulationm
}
void config_UART()
{
    //config P1.2, P1.1 UART
	P1SEL = BIT1+BIT2;
	P1SEL2 = BIT1+BIT2;

	UCA0CTL1 |= UCSSEL_2; // CHỌN CLOCK CHO UART

	// THIẾT LẬP BAUD RATE = 9600
	UCA0BR0 = 104;
	UCA0BR1 = 0;
	UCA0MCTL=UCBRS1; //Modulation Select 2  // Modulation Select 0
	UCA0CTL1&=~UCSWRST; //Reset  UART
	IE2|= UCA0RXIE; // ngắt Nhận

}
void main(void)
  {
       WDTCTL = WDTPW | WDTHOLD;    // Stop watchdog timer
        Config_Clocks();
        config_UART();
    	config_IO();
        config_PWM();


    	__bis_SR_register(GIE);
        while(1)
        {
        	if(mode == 1) // quay thẳng
        	{
        		DC_1_CW();
        		DC_2_CW();
        	}
        	if(mode == 2) // quay ngược
        	{
        		DC_1_CWC();
        		DC_2_CWC();
        	}
        	if(mode == 3 && toi == 1) //  tới quẹo trái
        	{
        		toi = 0;
        		DC_2_Short_Brake();
        		DC_1_CW();
        	}
        	if(mode == 4 && toi ==1) // tới quẹo phải
        	{
        		toi = 0;
        		DC_1_Short_Brake();
        		DC_2_CW();
        	}
        	if(mode == 3 && lui == 1) //  lui quẹo trái
        	{
        		lui = 0;
        		DC_2_Short_Brake();
        		DC_1_CWC();
        	}
        	if(mode == 4 && lui ==1) // lui quẹo phải
        	{
        		lui = 0;
        		DC_1_Short_Brake();
        		DC_2_CWC();
        	}
        	if(mode == 5) //dừng
        	{
        		DC_1_stop();
        		DC_2_stop();
        	}
        }

}
#pragma vector=USCIAB0RX_VECTOR
__interrupt void  ten(void)
{
	while(!(IFG2 & UCA0RXBUF)); //nếu nhận đc kí tự thì truyền ngược kí tự đó lên
	if(UCA0RXBUF =='w') // tới
	{
		UCA0TXBUF = UCA0RXBUF;
		mode = 1;
		toi = 1;
		lui = 0;
	}
	if(UCA0RXBUF =='s') // lùi
	{
		UCA0TXBUF = UCA0RXBUF;
		mode = 2;
		lui = 1;
		toi = 0;
	}
	if(UCA0RXBUF =='a')
	{
		UCA0TXBUF = UCA0RXBUF;
		mode = 3;//quẹo trái

	}
	if(UCA0RXBUF =='d')
	{
		UCA0TXBUF = UCA0RXBUF;
		mode = 4;// quẹo phải

	}
	if(UCA0RXBUF =='k')
	{
		UCA0TXBUF = UCA0RXBUF;
		mode = 5;//dừng

	}
}
