#ifndef _DEFINES_H_
#define _DEFINES_H_

#define PI acos(-1)

#define TYPE_ACK 0
#define TYPE_DATA 1
#define TYPE_MACPING_REQUEST 2
#define TYPE_MACPING_REPLY 3
#define TYPE_ICMP_REQUEST 4
#define TYPE_ICMP_REPLY 5
#define TYPE_FTP_COMMAND 6
#define TYPE_FILE_END 7

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

#define FRAME_OFFSET 40 // type(8) + id(8) + dst address(8) + src address(8) + length_field(8) 
#define CRC_LEN 8
#define IP_PORT_LEN 48

#define MAX_BYTE_NUM 1200
#define DEAFULT_WINDOW_SIZE 63

#define DEFAULT_RECEIVE_NUM 30
// address
#define YHD 0x01
#define ZYB 0x10

#define NODE1_ADDR 0xc0a80102
#define NODE1_PORT 23334

#define MODE_UDP_NODE2_SEND 0x0a
#define MODE_UDP_NODE1_RECEIVE 0x0b
#define MODE_UDP_NODE1_SEND 0x0c
#define MODE_UDP_NODE2_RECEIVE 0x0d
#define MODE_ICMP_NODE1 0x0e
#define MODE_ICMP_NODE2 0x0f
#define MODE_FTP_NODE1 0x10
#define MODE_FTP_NODE2 0x11

#define DEFAULT_HOST "ftp.ncnu.edu.tw"
constexpr size_t QUEUE_SIZE = 256;
enum STATE {
	FrameDetection = 1, Tx, Rx, TxACK
};

enum Status {
	Status_Waiting,
	Status_Sent,
	Status_Acked
};

enum COMMMAND {
	CONT,
	USER,
	PASS,
	PWD,
	CWD,
	PASV,
	LIST,
	RETR,
	QUIT,
	RESP, // response
	WRNG // wrong command
};

#endif // !_DEFINES_H_
