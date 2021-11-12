#ifndef _DEFINES_H_
#define _DEFINES_H_

#define PI acos(-1)

#define TYPE_ACK 0
#define TYPE_DATA 1

#define NO_HEADER -1
#define SYNC 0
#define DATA_PROCESS 1
#define DATA_RECEIVED 2

#define Tx_DONE 1

#define num_bits_per_frame 100
#define num_samples_per_bit 48

#define DEFAULT_RWS 4
enum STATE {
	FrameDetection = 1, Tx, Rx, TxACK
};


#endif // !_DEFINES_H_
