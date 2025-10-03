#ifndef RFFE_APP_H
#define RFFE_APP_H

#define RX1_RF_SWITCH_ADDR  0xD4
#define TRX1_RF_SWITCH_ADDR 0xD4
#define TRX1_ANT_RF_SWITCH_ADDR 0xD5
#define RX2_RF_SWITCH_ADDR  0xD6
#define TRX2_RF_SWITCH_ADDR 0xD6
#define TRX2_ANT_RF_SWITCH_ADDR 0xD7


int rffe_set_band(unsigned int channel, unsigned int band);
int rffe_set_lms_tx(unsigned int channel);
int rffe_set_lms_rx(unsigned int channel);
int rffe_pa_off(unsigned int channel);
int rffe_pa_on(unsigned int channel);
int rffe_lna_off(unsigned int channel);
int rffe_lna_on(unsigned int channel);
int rffe_rxatt(unsigned int channel, unsigned int att);
int rffe_rflb(unsigned int onoff);
int rffe_tdd(unsigned int lh);
int rffe_rd_dac(unsigned int channel, unsigned int *value);
int rffe_wr_dac(unsigned int channel, unsigned int value);

#endif // RFFE_APP_H
