#ifndef _DEFINES_H_
#define _DEFINES_H_

#define PI acos(-1)

#define TYPE_ACK 0
#define TYPE_DATA 1
#define TYPE_MACPING_REQUEST 2
#define TYPE_MACPING_REPLY 3
#define TYPE_ICMP 4

#define NO_HEADER -1
#define SYNC 0
#define DATA_PROCESS 1
#define DATA_RECEIVED 2

#define Tx_DONE 1

#define BITS_PERFRAME 128
#define SAMPLES_PERBIT 48

#define DEFAULT_RWS 4
#define MAX_WAITING_TIME 1500
#define MAX_RESEND_TIME 10

#define FRAME_OFFSET 32
#define CRC_LEN 8

#define MAX_BYTE_NUM 5000
#define DEAFULT_WINDOW_SIZE 63

#define DEFAULT_RECEIVE_NUM 50
// address
#define YHD 0x01
#define ZYB 0x10
#define NODE1_ADDR 0xc0a80102
#define NODE1_PORt 23334
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
