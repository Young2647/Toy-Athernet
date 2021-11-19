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

#define BITS_PERFRAME 128
#define SAMPLES_PERBIT 48

#define DEFAULT_RWS 4
#define MAX_WAITING_TIME 500
#define MAX_RESEND_TIME 5

#define FRAME_OFFSET 16
#define CRC_LEN 8

constexpr size_t QUEUE_SIZE = 256;
enum STATE {
	FrameDetection = 1, Tx, Rx, TxACK
};

enum Status {
	Status_Waiting,
	Status_Sent,
	Status_Acked
};



#endif // !_DEFINES_H_
