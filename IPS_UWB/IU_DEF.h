#include <Windows.h>

//Error Code
#define IU_NO_ERROR 1
#define IU_ERROR_SOCKET 2
#define IU_ERROR_CONNECTION 3


#define IU_READ_DATA_LENGTH			25		// length of the received data
#define IU_MIDDLE_BUFFER_LENGTH		IU_READ_DATA_LENGTH*2
#define IU_BT_ADDR_LEN				17+1	// 6 two-digit hex values plus 5 colons and null charactor '\0'


