/*
 * volume.h
 *
 * Created: 29/03/2022 11:20:48 AM
 *  Author: Middy Khong
 */ 


#ifndef VOLUME_H_
#define VOLUME_H_

// Parameters of FS2012 for sampling
#define INTERVAL_MEAS		15			// measurement interval (seconds)
#define INTERVAL_OVLP		2			// overlap interval (seconds)
#define N_SAMPLES_MEAS		15			// no. of samples in measurement interval
#define FLOWRATE_MIN		1000		// minimum flow rate (10ml/min)

struct vol {
	uint8_t i_meas;
	uint8_t i_ovlp;	
	uint8_t n_meas;									
	uint8_t n_ovlp;	
	uint16_t flowrates[N_SAMPLES_MEAS];
	uint16_t flowrates_ol[10];
	uint16_t sample_interval;
	uint16_t flr_mean;	
	float volume_instant;
	float volume_mean;
	uint8_t i1;	
	uint8_t i2;	
	uint8_t i3;	
	uint8_t i5;
};

uint16_t flowrate_mean(uint16_t flowrates[], uint8_t n);
float flowrate_to_volume(uint16_t mean, uint8_t i, uint8_t n);
void set_sample_volume(uint8_t loLimit, uint16_t hiLimit);
struct vol volume_instants(struct vol v);
struct vol volume_means(struct vol v);
struct vol volume_init(struct vol v);

#endif /* VOLUME_H_ */