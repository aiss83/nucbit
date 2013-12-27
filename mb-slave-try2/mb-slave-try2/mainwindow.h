#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <qtimer.h>
#include "ui_mainwindow.h"
#include "serialport.h"

#define MB_SER_PDU_SIZE_MIN     4       /*!< Minimum size of a Modbus RTU frame. */
#define MB_SER_PDU_SIZE_MAX     256     /*!< Maximum size of a Modbus RTU frame. */
#define MB_SER_PDU_SIZE_CRC     2       /*!< Size of CRC field in PDU. */
#define MB_SER_PDU_ADDR_OFF     0       /*!< Offset of slave address in Ser-PDU. */
#define MB_SER_PDU_PDU_OFF      1       /*!< Offset of Modbus-PDU in Ser-PDU. */
#define MB_ADDRESS_BROADCAST    0

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = 0, Qt::WFlags flags = 0);
	~MainWindow();

private:
	Ui::MainWindowClass ui;
	SerialPort port;
	QTimer timer;
	unsigned long cntPkt, cntErr,cntWtf;
	QByteArray buf,out;
	char nodeAddr;
	unsigned short MBCRC16( QByteArray * bFrame, int Len );

	typedef union {
		unsigned short ush;
		unsigned char  uch[2];
	} mbShrt_t;


public slots:	
		void openPort();
		void closePort();
		void nodeUpdateSettings();
		void DataInput(quint64);
		void timeout();
		void processFrame();

};

#endif // MAINWINDOW_H
