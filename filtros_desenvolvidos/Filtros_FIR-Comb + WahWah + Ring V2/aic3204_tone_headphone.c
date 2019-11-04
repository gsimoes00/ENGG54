//////////////////////////////////////////////////////////////////////////////
// * File name: aic3204_tone_headphone.c

//////////////////////////////////////////////////////////////////////////////

//#include "stdio.h"
#include "ezdsp5502.h"
#include "ezdsp5502_mcbsp.h"
#include "csl_mcbsp.h"
#include "math.h"
//
#include "ezdsp5502_i2cgpio.h"
#include "ezdsp5502_i2c.h"
//
#include "csl_gpio.h" //Usar botoes
//TEM QUE USAR PRAGMA E "const" NA VARIÁVEL (NESSE CASO!)
//#pragma DATA_SECTION(signal,".infoD")
#pragma DATA_SECTION(filtro,".infoD")
//#pragma CODE_SECTION(buttonInterrupt, "interruptI2C")
//#pragma INTERRUPT(buttonInterrupt)
//#pragma INTR_FUNC ( buttonInterrupt , 24 );



/*interrupt void c_int24()
{
    //if(!EZDSP5502_I2CGPIO_readLine(SW1)){ // O ideal seria uma interrupção aqui
    (filtro >= 3) ? (filtro = 0) : (filtro = filtro + 1);
    //}
}*/
extern Int16 AIC3204_rset(Uint16 regnum, Uint16 regval);

/*
 *
 *  AIC3204 Tone
 *      Outputs a 1KHz tone on STEREO OUT
 *
 */
//interrupt void buttonInterrupt(){
//   if(!EZDSP5502_I2CGPIO_readLine(SW0)){ // O ideal seria uma interrupção aqui
//            (filtro >= 3)? (filtro=0) : (filtro = filtro +1);
//  }
Int16 aic3204_tone_headphone()
{


    // Int16 filtro = 2;


    //debug de assembly inline
    /*
     // asm(" NOP");
     asm(" MOV #20, T3"); //T3 ARMAZENA SP
     asm(" PSH T3");
     asm(" MOV SP, T3"); //T3 ARMAZENA SP

     asm(" MOV #0 , SP");  //Zera SP (valor-base = 0)
     asm(" MOV #128 , *SP(45h)"); //coloca 128 = 0000000010000000b em SP + 45h == IER1 (habilitar o interrupt para I2C)
     //asm(" MOV T3, *(SP)");  //Retorna o valor original de SP;
     //asm(" BCLR 40h, ST1"); //ALTERA CPL PARA 0 EM ST1
     asm(" MOV T3, SP"); //DEVOLVE ORIGINAL DE SP
     asm(" POP T3"); //DESEMPILHA T3

     asm(" MOV #18, T3"); //Poe valor qualquer em T3
     asm(" PSH  T2"); // empilha t2
     asm(" MOV _filtro, *(T3)"); //Carrega valor do endereço 0x100000 em T3

     asm(" MOV SP, *(T2)");
     asm(" MOV #65024, SP");


     asm(" MOV _c_int24, *(T3)"); //ALTERA CPL PARA 0 EM ST1
     asm(" MOV #(0xFEB8), *(T3)");
     asm(" MOV T2, SP"); //retorna valor de SP
     asm(" POP T2"); //retorna valor de T2


     //asm(" BSET 40h, ST1"); //ALTERA CPL PARA 1 EM ST1
     */
    //Configurar push buttons
    EZDSP5502_I2CGPIO_configLine( SW0, IN);
    EZDSP5502_I2CGPIO_configLine( SW1, IN);

    //Configurar LEDs para debug
   // EZDSP5502_I2CGPIO_configLine( LED0, OUT);
  //  EZDSP5502_I2CGPIO_configLine( LED1, OUT);
  //  EZDSP5502_I2CGPIO_configLine( LED2, OUT);
    //Desliga os LEDs (lembrando que eles estão com lógica invertida
  //  EZDSP5502_I2CGPIO_writeLine( LED0, HIGH);
   // EZDSP5502_I2CGPIO_writeLine( LED1, HIGH);
   // EZDSP5502_I2CGPIO_writeLine( LED2, HIGH);
    /* ---------------------------------------------------------------- *
     *  Configure AIC3204                                               *
     * ---------------------------------------------------------------- */
    AIC3204_rset(0, 0);      // Select page 0
    AIC3204_rset(1, 1);      // Reset codec
    AIC3204_rset(0, 1);      // Select page 1
    AIC3204_rset(1, 8);      // Disable crude AVDD generation from DVDD
    AIC3204_rset(2, 1);      // Enable Analog Blocks, use LDO power
    AIC3204_rset(0, 0);

    /* PLL and Clocks config and Power Up  */
    //AIC3204_rset( 27, 0x0d );  // BCLK and WCLK are set as o/p; AIC3204(Master) and set as 16-bit data
    AIC3204_rset(27, 0x0d); // BCLK and WCLK are set as o/p; AIC3204(Master) and set as 16-bit data
    AIC3204_rset(28, 0x00);  // Data ofset = 0
    AIC3204_rset(4, 3);   // PLL sett2ing: PLLCLK <- MCLK, CODEC_CLKIN <-PLL CLK
    /*******************************************************************/
    //Valores originais para multiplos de 48khz
    //AIC3204_rset( 6, 7 );      // PLL setting: J=7
    //AIC3204_rset( 7, 0x06 );   // PLL setting: HI_BYTE(D=1680)
    //AIC3204_rset( 8, 0x90 );   // PLL setting: LO_BYTE(D=1680)
    //Valores para multiplos de 11025 hz
    AIC3204_rset(6, 6);      // PLL setting: J=6
    AIC3204_rset(7, 0x16);   // PLL setting: HI_BYTE(D=5856)
    AIC3204_rset(8, 0xE0);   // PLL setting: LO_BYTE(D=5856)

    /*******************************************************************/
    AIC3204_rset(30, 0x9C);  // For 32 bit clocks per frame in Master mode ONLY
                             // BCLK=DAC_CLK/N =(12288000/8) = 1.536MHz = 32*fs
    AIC3204_rset(5, 0x91);   // PLL setting: Power up PLL, P=1 and R=1

    /******************************@@@@@@@@@@@@@@@@@@@@@@@@@@@@*************************************/
    //Valores originais para 48khz
    /*
     AIC3204_rset( 13, 0x00 );     // Hi_Byte(DOSR) for DOSR = 128 decimal or 0x0080 DAC oversamppling
     AIC3204_rset( 14, 0x80 );  // Lo_Byte(DOSR) for DOSR = 128 decimal or 0x0080
     AIC3204_rset( 20, 0x80 );  // AOSR for AOSR = 128 decimal or 0x0080 for decimation filters 1 to 6
     AIC3204_rset( 11, 0x82 );  // Power up NDAC and set NDAC value to 2 //freq atual: 11025 khz
     AIC3204_rset( 12, 0x87 );  // Power up MDAC and set MDAC value to 7 ->F
     AIC3204_rset( 18, 0x87 );  // Power up NADC and set NADC value to 7 //freq atual:11025 hz
     AIC3204_rset( 19, 0x82 );  // Power up MADC and set MADC value to 2 // valor atual de 6
     */
    //Valores para 11025 hz
    AIC3204_rset(13, 0x00); // Hi_Byte(DOSR) for DOSR = 128 decimal or 0x0080 DAC oversamppling
    AIC3204_rset(14, 0x80);  // Lo_Byte(DOSR) for DOSR = 128 decimal or 0x0080
    AIC3204_rset(20, 0x80); // AOSR for AOSR = 128 decimal or 0x0080 for decimation filters 1 to 6
    AIC3204_rset(11, 0x82); // Power up NDAC and set NDAC value to 2 // valor atual de 2
    AIC3204_rset(12, 0x87); // Power up MDAC and set MDAC value to 7 //freq atual: 44100 hz
    AIC3204_rset(18, 0x87); // Power up NADC and set NADC value to 7 //freq atual: 44100 hz
    AIC3204_rset(19, 0x82); // Power up MADC and set MADC value to 2 // valor atual de 2
    /******************************@@@@@@@@@@@@@@@@@@@@@@@@@@@@*************************************/

    /* DAC ROUTING and Power Up */
    AIC3204_rset(0, 1);      // Select page 1
    AIC3204_rset(0x0c, 8);   // LDAC AFIR routed to HPL
    AIC3204_rset(0x0d, 8);   // RDAC AFIR routed to HPR
    AIC3204_rset(0, 0);      // Select page 0
    AIC3204_rset(64, 2);     // Left vol=right vol
    // AIC3204_rset( 65, 0xA0);      // Left DAC gain to -dB VOL; Right tracks Left
    AIC3204_rset(65, 0x00);      // Left DAC gain to -dB VOL; Right tracks Left
    //AIC3204_rset( 65, 0x00);      // Left DAC gain to -dB VOL; Right tracks Left
    AIC3204_rset(63, 0xd4);  // Power up left,right data paths and set channel
    AIC3204_rset(0, 1);      // Select page 1
    AIC3204_rset(9, 0x30);   // Power up HPL,HPR
    AIC3204_rset(0x10, 0x00);   // Unmute HPL , 0dB gain
    AIC3204_rset(0x11, 0x00);   // Unmute HPR , 0dB gain
    AIC3204_rset(0, 0);      // Select page 0
    EZDSP5502_waitusec(100); // wait

    /* ADC ROUTING and Power Up */
    AIC3204_rset(0, 1);      // Select page 1
    AIC3204_rset(0x34, 0x30);      // STEREO 1 Jack
                                   // IN2_L to LADC_P through 40 kohm
    AIC3204_rset(0x37, 0x30);                // IN2_R to RADC_P through 40 kohmm
    //AIC3204_rset( 0x36, 3 );   // CM_1 (common mode) to LADC_M through 40 kohm
    AIC3204_rset(0x36, 2);   // CM_1 (common mode) to LADC_M through 20 kohm
    //AIC3204_rset( 0x39, 0xc0 );// CM_1 (common mode) to RADC_M through 40 kohm
    AIC3204_rset(0x39, 0x80);    // CM_1 (common mode) to RADC_M through 20 kohm
    AIC3204_rset(0x3b, 0);   // MIC_PGA_L unmute
    AIC3204_rset(0x3c, 0);   // MIC_PGA_R unmute
    AIC3204_rset(0, 0);      // Select page 0
    AIC3204_rset(0x51, 0xc0);      // Powerup Left and Right ADC
    AIC3204_rset(0x52, 0);   // Unmute Left and Right ADC
    AIC3204_rset(0, 0);
    EZDSP5502_waitusec(200); // Wait

    /* Initialize McBSP */
    EZDSP5502_MCBSP_init();


    EZDSP5502_waitusec(10000);

    //Valores extraídos do matlab (testando filtros)
    //float gains[5]={0, 0.1532, 0.2, 0.1285 , 0.1532 };
    //float gains[5] = {0.15 , -0.15, -0.15, 0.15, 0};
    //float states1[5] = {0}, states2[5] = {0};

    //Int16 ganhos[5] = {1,-1,-1, 0, 1}; //dividir estes valores por 10000
    //Int16 estados[5] = {0}, estados2[5] = {0};
    float carrier = 0;
    float ganhos[11] = { 0.5, 0, 0, 0, 0, 0.25, 0, 0, 0, 0, 0.125 };
    Int16 estados[11] = { 0 }, estados2[11] = { 0 };

    float freq_ratio = 0.01; // razao Fc/Fs do Ring (Fc/44100);
    float pi = 3.1416;

    Int16  left_data = 0, right_data = 0;  //tratando como stereo
    Int32 y1, y2;

    Int16 j, y1_out = 0, y2_out = 0;
    //Int32 sample = 0;
    Uint16 sample = 0;
    Uint16 counter = 0;
    Uint16 filtro = 0;

    /*********Variáveis para WahWah********/

    Int32 yllow[2]= {0}, ylmed[2]={0}, ylhigh[2]={0} ;  //Considere que ylow[0] = ylow[n] e ylow[1] = ylow[n-1] e assim por diante;
                                                        //'yl' indica que é saida y lado esquerdo
    Int32 yrlow[2]={0}, yrmed[2]={0}, yrhigh[2]={0};

    float F1, damping = 0.02, q;
    Int32 Fs = 44100, Fc = 500;


    q = 2*damping;
    F1 = 2*sin(pi*Fc/Fs);

    while (1){

        if (!EZDSP5502_I2CGPIO_readLine(SW0))
        { // O ideal seria uma interrupção aqui
            (filtro >= 4)?(filtro = 0):(filtro++);
        }

        if (!EZDSP5502_I2CGPIO_readLine(SW1))
        { // O ideal seria uma interrupção aqui
            (filtro <= 0)?(filtro = 4):(filtro--);
        }






            for (counter = 0; counter < 2; counter++){
             //Repete o mesmo filtro por 10 segundos
                switch (filtro){


                case 0:
                    EZDSP5502_I2CGPIO_writeLine( LED0, LOW);
                    EZDSP5502_I2CGPIO_writeLine( LED1, HIGH);
                    EZDSP5502_I2CGPIO_writeLine( LED2, HIGH);
                    for (sample = 0; sample < 44100; sample++){

                        EZDSP5502_MCBSP_read(&left_data); //Lê canal esquerdo
                        EZDSP5502_MCBSP_read(&right_data); // direito
                        y1_out = 0;
                        y2_out = 0;
                        y1= 0;
                        y2 = 0;


                        //// MODELO FIR-COMB COM M = 10 E G = 0.5

                        //  for(j=0;j < 10; j++){

                        //Trecho com erro (?) -> y1 ta sendo acumulado (nao pode, so deve ocorrer o deslocamento dos estados
                        //y1 = y1 +  estados[j]*ganhos[j];
                        //y2 = y2 +  estados2[j]*ganhos[j];

                        // }

                        //GANHOS para mono

                        y1 =  (left_data * ganhos[0])  + (estados[5] * ganhos[5]) + (estados[10] * ganhos[10]);
                        y2 =  (right_data * ganhos[0]) + (estados[5] * ganhos[5]) + (estados[10] * ganhos[10]);

                        (y1 > 32766)? (y1_out = 32766) : (y1_out = (Int16) y1);
                        (y1 < -32766) ? (y1_out = -32766) : (y1_out = (Int16) y1);

                        (y2 > 32766)? (y2_out = 32766) : (y2_out = (Int16) y1);
                        (y2 < -32766) ? (y2_out = -32766) : (y2_out = (Int16) y1);

                      //  (y2 > 32766) ? (y2 = 32766) : (y2);
                     //   (y2 < -32766) ? (y2 = -32766) : (y2);
                        //Desloca os atrasos
                        for (j = 10; j >= 1; j--)
                        {

                            estados[j] = estados[j - 1];
                            estados2[j] = estados2[j - 1];
                        }

                        estados[0] = left_data;
                        estados2[0] = right_data;

                        EZDSP5502_MCBSP_write(y1_out);
                        EZDSP5502_MCBSP_write(y2_out);


                    }
                break;

                case 1:  //Caso sem filtro algum
                    EZDSP5502_I2CGPIO_writeLine( LED0, HIGH);
                    EZDSP5502_I2CGPIO_writeLine( LED1, LOW);
                    EZDSP5502_I2CGPIO_writeLine( LED2, HIGH);

                    for (sample = 0; sample < 44100; sample++){

                        EZDSP5502_MCBSP_read(&left_data); //Lê canal esquerdo
                        EZDSP5502_MCBSP_read(&right_data); // direito
                        EZDSP5502_MCBSP_write(left_data);
                        EZDSP5502_MCBSP_write(right_data);
                    }
                break;

                case 2:
                    EZDSP5502_I2CGPIO_writeLine( LED0, HIGH);
                    EZDSP5502_I2CGPIO_writeLine( LED1, HIGH);
                    EZDSP5502_I2CGPIO_writeLine( LED2, LOW);
                    for (sample = 0; sample < 44100; sample++){

                        EZDSP5502_MCBSP_read(&left_data); //Lê canal esquerdo
                        EZDSP5502_MCBSP_read(&right_data); // direito
                        y1_out = 0;
                        y2_out = 0;
                        y1= 0;
                        y2 = 0;

                        y1 =  (left_data * ganhos[0]
                                + estados[10] * ganhos[10]);
                        y2 =  (right_data * ganhos[0]
                                + estados[10] * ganhos[10]);

                        (y1 > 32766)? (y1_out = 32766) : (y1_out = (Int16) y1);
                        (y1 < -32766) ? (y1_out = -32766) : (y1_out = (Int16) y1);

                        (y2 > 32766)? (y2_out = 32766) : (y2_out = (Int16) y1);
                        (y2 < -32766) ? (y2_out = -32766) : (y2_out = (Int16) y1);
                        //Desloca os atrasos
                        for (j = 10; j >= 1; j--)
                        {

                            estados[j] = estados[j - 1];
                            estados2[j] = estados2[j - 1];
                        }

                        //Armazena os valores de entrada
                        estados[0] = left_data;
                        estados2[0] = right_data;

                        EZDSP5502_MCBSP_write(y1_out);
                        EZDSP5502_MCBSP_write(y2_out);
                    }
                break;

                case 3:
                    EZDSP5502_I2CGPIO_writeLine( LED0, LOW);
                    EZDSP5502_I2CGPIO_writeLine( LED1, LOW);
                    EZDSP5502_I2CGPIO_writeLine( LED2, HIGH);
                    y1_out = 0;
                    y2_out = 0;

                    for (sample = 0; sample < 44100; sample++){

                        y1 = 0;
                        y2 = 0;
                        EZDSP5502_MCBSP_read(&left_data); //Lê canal esquerdo
                        EZDSP5502_MCBSP_read(&right_data); // direito
                        y1 =  left_data * sin(2 * pi * freq_ratio * sample);
                        y2 =  right_data * sin(2 * pi * freq_ratio * sample);


                        (y1 > 32766)? (y1_out = 32766) : (y1_out = (Int16) y1);
                        (y1 < -32766) ? (y1_out = -32766) : (y1_out = (Int16) y1);

                        (y2 > 32766)? (y2_out = 32766) : (y2_out = (Int16) y1);
                        (y2 < -32766) ? (y2_out = -32766) : (y2_out = (Int16) y1);

                        EZDSP5502_MCBSP_write(y1);
                        EZDSP5502_MCBSP_write(y2);
                    }
                break;


                case 4:  //WahWah
                    EZDSP5502_I2CGPIO_writeLine( LED0, LOW);
                    EZDSP5502_I2CGPIO_writeLine( LED1, HIGH);
                    EZDSP5502_I2CGPIO_writeLine( LED2, LOW);

                    y1 = 0;
                    y2 = 0;
                    for(sample=0; sample <2; sample++){ //Limpar valores anteriores de saida
                        yllow[sample] = 0;
                        ylmed[sample]=0;
                        ylhigh[sample]=0;
                        yrlow[sample]=0;
                        yrmed[sample]=0;
                        yrhigh[sample]=0;
                    }

                    for (sample = 0; sample < 44100; sample++){

                        (sample > 22025)? (F1 = 2*sin(pi*Fc/Fs*0.00007*sample)): (F1 = 2*sin(pi*Fc/Fs*0.00007*(Fs-sample)));



                        EZDSP5502_MCBSP_read(&left_data); //Lê canal esquerdo
                        EZDSP5502_MCBSP_read(&right_data); // direito


                           for(j=1; j > 0; j--){

                               ylhigh[j] = ylhigh[j-1];
                               yrhigh[j] = yrhigh[j-1];

                               ylmed[j] = ylmed[j-1];
                               yrmed[j] = yrmed[j-1];

                               yllow[j] = yllow[j-1];
                               yrlow[j] = yrlow[j-1];

                           }



                        ylhigh[0] = left_data - yllow[1] - q*ylmed[1] ;
                            yrhigh[0] = right_data  - yrlow[1] - q*yrmed[1] ;

                            ylmed[0] = F1*ylhigh[0] + ylmed[1];
                            yrmed[0] = F1*yrhigh[0] + yrmed[1];

                            yllow[0] = F1*ylmed[0] + yllow[1];
                            yrlow[0] = F1*yrmed[0] + yrlow[1];



                            y1 =  ylmed[0];
                            y2 =  yrmed[0];

                            (y1 > 32766)? (y1_out = 32766) : (y1_out = (Int16) y1);
                            (y1 < -32766) ? (y1_out = -32766) : (y1_out = (Int16) y1);

                            (y2 > 32766)? (y2_out = 32766) : (y2_out = (Int16) y1);
                            (y2 < -32766) ? (y2_out = -32766) : (y2_out = (Int16) y1);
                              //////////////TRECHO PARA FAZER O DESLOCAMENTO DE AMOSTRAS (ATRASO)
                             /*   for(j=1; j > 0; j--){

                                    ylhigh[j] = ylhigh[j-1];
                                    yrhigh[j] = yrhigh[j-1];

                                    ylmed[j] = ylmed[j-1];
                                    yrmed[j] = yrmed[j-1];

                                    yllow[j] = yllow[j-1];
                                    yrlow[j] = yrlow[j-1];

                                }*/


                              ///////////////////////////////////////////////////////


                           EZDSP5502_MCBSP_write(y1_out);
                           EZDSP5502_MCBSP_write(y2_out);
                                       }
                  break;

                default:
                 break;
                }
            }
        }
   // }

    //    AIC3204_rset( 11, 0x82 );  // Power up NDAC and set NDAC value to 2 ->

    //EZDSP5502_MCBSP_close(); // Disable McBSP
    //AIC3204_rset( 1, 1 );    // Reset codec

    return 0;
}