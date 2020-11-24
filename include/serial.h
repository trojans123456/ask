#ifndef __SERIAL_H
#define __SERIAL_H

#include "default.h"

#ifdef __cplusplus
extern "C" {
#endif


declear_handler(hSerial);


typedef enum
{
    COMM_STOPBIT_1 = 1,  // 1位停止位
    COMM_STOPBITS_1_5,    // 1.5位停止位 for win
    COMM_STOPBITS_2,	  // 2位停止位
}CommStopBit;

typedef enum
{
    COMM_PARITY_NONE,		//无校验位
    COMM_PARITY_ODD,		//奇数校验
    COMM_PARITY_EVEN,	// 偶数校验
    COMM_PARITY_MARK,			// 校验位总为1
    COMM_PARITY_SPACE,			// 校验位总为0
}CommParityType;

/// 串口属性结构，128字节
typedef struct
{
    int         baudrate;		///< 实际的波特率值。
    char	dataBits;	///< 实际的数据位 取值为5,6,7,8
    char	parity;		///< 奇偶校验选项，取CommParityType类型的枚举值。
    char	stopBits;	///< 停止位数，取CommStopBit类型的枚举值。
    char	flag;		///< 特殊串口标志，取CommSpecialFlag类型的枚举值。
} CommAttribute;


//打开串口
//param:[IN]tty_dev:串口设备文件
hSerial serial_open(const char* tty_dev);

//关闭串口
int serial_close(hSerial h);

//设置串口属性
int serial_setAttr(hSerial h, const CommAttribute *attr);

//设置串口数据回调函数
typedef int (*cb_com_rxdata_func)(hSerial h, char *pdata, int len, void *ctx);
int serial_setRxDataCallBack(hSerial h, cb_com_rxdata_func cbRxData, void *ctx);


//启动comm异步监听
//time: 允许等待时间，毫秒单位，0: 永久等待
int serial_start(hSerial h, int time);

//关闭comm异步监听
int serial_stop(hSerial h);


int serial_purge(hSerial h, int dw_flags);


int serial_write(hSerial h, void *pdata, int len);








#ifdef __cplusplus
}
#endif

#endif
