#ifndef MZFLASHER_H
#define MZFLASHER_H

#include <QtGui/QMainWindow>
#include <QMessageBox>
#include <qbytearray.h>
#include "ui_mzflasher.h"
#include "serialport.h"

#define MAXNODES 50
#define MAXDEVS 32

/* SLIP special character codes
 */
#define END             0300    /* indicates end of packet */
#define ESC             0333    /* indicates byte stuffing */
#define ESC_END         0334    /* ESC ESC_END means END data byte */
#define ESC_ESC         0335    /* ESC ESC_ESC means ESC data byte */

class mzFlasher : public QMainWindow
{
	Q_OBJECT

public:
	mzFlasher(QWidget *parent = 0, Qt::WFlags flags = 0);
	~mzFlasher();

private:
	Ui::mzFlasherClass ui;
	SerialPort port;

	QString ieee2str(QByteArray *d);
	QByteArray str2ieee(QString *str);
	void ParsePacket(unsigned char *p);
	bool sanityCheck();
	
	void mzFlasher::send_packet(QByteArray *pkt);
	//slip part
	QByteArray packet;
	bool staffCk;

	void InPacket();
	void getMac();
	void getConf();
	void ping();
public slots:
	void addNode();
	void delNode();
	void addDev();
	void delDev();

	void save();
	void load();

	void portClick();

	void srchClick();

	QByteArray buidPackage();
	void DataIn(quint64 n);
	
	

signals:
	

};

#endif // MZFLASHER_H
