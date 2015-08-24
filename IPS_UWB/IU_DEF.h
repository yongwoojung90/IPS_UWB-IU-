#include <Windows.h>

//Error Code
#define IU_NO_ERROR 1
#define IU_ERROR_SOCKET 2
#define IU_ERROR_CONNECTION 3


#define CP_PACKET_LENGTH			25		// length of the Cicada Project Protocol's packet :  *00.0000,00.0000,00.0000=
#define CP_RECV_BUF_LENGTH			CP_PACKET_LENGTH*2 // 50bytes-bluetooth receive buffer length
#define CP_BT_ADDR_LEN				17+1	//Cicada Project Bluetooth Address Length, 6 two-digit hex values plus 5 colons and null charactor '\0'


