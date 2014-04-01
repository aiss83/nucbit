#include "mzflasher.h"

#include "mzSettings.h"
#include "flash_proto.h"

#include <qstring.h>
#include <windows.h>
#include <strsafe.h>

#include <qevent.h>

#include "qdebug.h"

	/* CRC16 implementation acording to CCITT standards */

static const unsigned short crc16tab[256]= {
	0x0000,0x1021,0x2042,0x3063,0x4084,0x50a5,0x60c6,0x70e7,
	0x8108,0x9129,0xa14a,0xb16b,0xc18c,0xd1ad,0xe1ce,0xf1ef,
	0x1231,0x0210,0x3273,0x2252,0x52b5,0x4294,0x72f7,0x62d6,
	0x9339,0x8318,0xb37b,0xa35a,0xd3bd,0xc39c,0xf3ff,0xe3de,
	0x2462,0x3443,0x0420,0x1401,0x64e6,0x74c7,0x44a4,0x5485,
	0xa56a,0xb54b,0x8528,0x9509,0xe5ee,0xf5cf,0xc5ac,0xd58d,
	0x3653,0x2672,0x1611,0x0630,0x76d7,0x66f6,0x5695,0x46b4,
	0xb75b,0xa77a,0x9719,0x8738,0xf7df,0xe7fe,0xd79d,0xc7bc,
	0x48c4,0x58e5,0x6886,0x78a7,0x0840,0x1861,0x2802,0x3823,
	0xc9cc,0xd9ed,0xe98e,0xf9af,0x8948,0x9969,0xa90a,0xb92b,
	0x5af5,0x4ad4,0x7ab7,0x6a96,0x1a71,0x0a50,0x3a33,0x2a12,
	0xdbfd,0xcbdc,0xfbbf,0xeb9e,0x9b79,0x8b58,0xbb3b,0xab1a,
	0x6ca6,0x7c87,0x4ce4,0x5cc5,0x2c22,0x3c03,0x0c60,0x1c41,
	0xedae,0xfd8f,0xcdec,0xddcd,0xad2a,0xbd0b,0x8d68,0x9d49,
	0x7e97,0x6eb6,0x5ed5,0x4ef4,0x3e13,0x2e32,0x1e51,0x0e70,
	0xff9f,0xefbe,0xdfdd,0xcffc,0xbf1b,0xaf3a,0x9f59,0x8f78,
	0x9188,0x81a9,0xb1ca,0xa1eb,0xd10c,0xc12d,0xf14e,0xe16f,
	0x1080,0x00a1,0x30c2,0x20e3,0x5004,0x4025,0x7046,0x6067,
	0x83b9,0x9398,0xa3fb,0xb3da,0xc33d,0xd31c,0xe37f,0xf35e,
	0x02b1,0x1290,0x22f3,0x32d2,0x4235,0x5214,0x6277,0x7256,
	0xb5ea,0xa5cb,0x95a8,0x8589,0xf56e,0xe54f,0xd52c,0xc50d,
	0x34e2,0x24c3,0x14a0,0x0481,0x7466,0x6447,0x5424,0x4405,
	0xa7db,0xb7fa,0x8799,0x97b8,0xe75f,0xf77e,0xc71d,0xd73c,
	0x26d3,0x36f2,0x0691,0x16b0,0x6657,0x7676,0x4615,0x5634,
	0xd94c,0xc96d,0xf90e,0xe92f,0x99c8,0x89e9,0xb98a,0xa9ab,
	0x5844,0x4865,0x7806,0x6827,0x18c0,0x08e1,0x3882,0x28a3,
	0xcb7d,0xdb5c,0xeb3f,0xfb1e,0x8bf9,0x9bd8,0xabbb,0xbb9a,
	0x4a75,0x5a54,0x6a37,0x7a16,0x0af1,0x1ad0,0x2ab3,0x3a92,
	0xfd2e,0xed0f,0xdd6c,0xcd4d,0xbdaa,0xad8b,0x9de8,0x8dc9,
	0x7c26,0x6c07,0x5c64,0x4c45,0x3ca2,0x2c83,0x1ce0,0x0cc1,
	0xef1f,0xff3e,0xcf5d,0xdf7c,0xaf9b,0xbfba,0x8fd9,0x9ff8,
	0x6e17,0x7e36,0x4e55,0x5e74,0x2e93,0x3eb2,0x0ed1,0x1ef0
};

static unsigned short crc16_ccitt(const unsigned char *buf, int len)
{
	int counter;
	unsigned short crc = 0;
	for( counter = 0; counter < len; counter++)
    {
		crc = (crc<<8) ^ crc16tab[((crc>>8) ^ *(char *) buf++) & 0x00FF];
    }

	return crc;
}

QString getLastErrorMsg(int err) {
    LPWSTR bufPtr = NULL;
   // DWORD err = port.getLastError();
    FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                   FORMAT_MESSAGE_FROM_SYSTEM |
                   FORMAT_MESSAGE_IGNORE_INSERTS,
                   NULL, err, 0, (LPWSTR)&bufPtr, 0, NULL);
    const QString result = 
        (bufPtr) ? QString::fromUtf16((const ushort*)bufPtr).trimmed() :
                   QString("Unknown Error %1").arg(err);
    LocalFree(bufPtr);
    return result;
}


#define send_char(x) tmp.append(x);
/* SEND_PACKET: sends a packet of length "len", starting at
 * location "p".
 */
void mzFlasher::send_packet(QByteArray *pkt)
	 {
		 QByteArray tmp;
		 char *p = pkt->data();

		 unsigned int len = pkt->size();
	/* send an initial END character to flush out any data that may
	 * have accumulated in the receiver due to line noise
	 */
	send_char(END);

	/* for each byte in the packet, send the appropriate character
	 * sequence
	 */
	while (len--) {
		switch (*p) {
		/* if it's the same code as an END character, we send a
		 * special two character code so as not to make the
		 * receiver think we sent an END
		 */
		case END:
			send_char(ESC);
			send_char(ESC_END);
			break;

			/* if it's the same code as an ESC character,
			 * we send a special two character code so as not
			 * to make the receiver think we sent an ESC
			 */
		case ESC:
			send_char(ESC);
			send_char(ESC_ESC);
			break;
			/* otherwise, we just send the character
			 */
		default:
			send_char(*p);
		}

		p++;
	}

	/* tell the receiver that we're done sending the packet
	 */
	send_char(END);

	/*
	tmp.clear();
	tmp.append(QString("test"));
	*/
	signed int ret = (signed int)port.Write(&tmp);
	qDebug() <<"Out" << tmp.toHex();
	qDebug() <<"Sent: "<< ret;
	
	if(ret < 0){
	 void  *lpMsgBuf;
		qDebug() << getLastErrorMsg(port.getLastError());

	}
}


mzFlasher::mzFlasher(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags)
{
	ui.setupUi(this);

	staffCk= false;
	ui.cmbPorts->addItems( port.EnumeratePorts() );
	

	QObject::connect(ui.btnPort, SIGNAL(clicked()), this, SLOT(portClick()));

	QObject::connect(ui.btnNodeAdd, SIGNAL(clicked()), this, SLOT(addNode()));
	QObject::connect(ui.btnNodeDel, SIGNAL(clicked()), this, SLOT(delNode()));
	QObject::connect(ui.btnAddDev, SIGNAL(clicked()), this, SLOT(addDev()));
	QObject::connect(ui.btnDelDev, SIGNAL(clicked()), this, SLOT(delDev()));

	QObject::connect(ui.btnWrite, SIGNAL(clicked()), this, SLOT(save()));
	QObject::connect(ui.btnRead, SIGNAL(clicked()), this, SLOT(load()));
		
	QObject::connect(ui.btnSrch, SIGNAL(clicked()), this, SLOT(srchClick()));

	QObject::connect(&port, SIGNAL(DataReceived(quint64)), this, SLOT(DataIn(quint64)));
	
	qDebug() << "start";

	
	ui.grpCfg->setEnabled(false);
}

mzFlasher::~mzFlasher()
{
	port.CancelListening();
	port.Close();

}

/*
Too much to make delegate
*/
/*
bool mzFlasher::eventFilter(QObject *object, QEvent *event)
{ 
    if (event->type() == QEvent::KeyPress)
    {
      QKeyEvent *e = reinterpret_cast<QKeyEvent *>(event);
      if ( e->key() == Qt::Key_Tab    ||
           e->key() == Qt::Key_Return ||
           e->key() == Qt::Key_Enter )
      {
         // make the necessary validation
         if ( sanityCheck() )
         {
            event->accept();
            return true;
         }
      }
 
	}
}
*/
void mzFlasher::portClick(){
	if(!port.IsOpen()){
		//opening
		port.Open(ui.cmbPorts->currentText());
		if (port.IsOpen()){

            port.SetBaudrate(SerialPort::Baud115200);
            port.SetDataBits(SerialPort::Data8);
            port.SetParity(SerialPort::ParNone);
            port.SetHandshake(SerialPort::HandshakeOff);
            port.SetStopBits(SerialPort::Stop1);
            port.SetReadTimeout(5);
			
            port.SetListen(true);


		
			ui.cmbPorts->setEnabled(false);
			ui.btnRead->setEnabled(true);
			ui.btnWrite->setEnabled(true);
			
			//ui.gdpCfg->setEnabled(false);
			ping();
			ui.btnPort->setText("Close");
		}else{
			ui.statusBar->showMessage("Port opening error");
		}

	}else{
		//closing
		port.CancelListening();
		port.Close();
		ui.cmbPorts->setEnabled(true);
		ui.btnRead->setEnabled(false);
		ui.btnWrite->setEnabled(false);
		ui.grpCfg->setEnabled(false);

		ui.lblRime->clear();
		ui.btnPort->setText("Open");

	}

}


void mzFlasher::addNode(){
	if (ui.treeWidget->topLevelItemCount() < MAXNODES){
		QTreeWidgetItem *n = new QTreeWidgetItem;

		#if (RIMEADDR_SIZE==8)
	n->setText(0,"0.0.0.0.0.0.0.0");
#else if (RIMEADDR_SIZE==2)
		n->setText(0,"0.0");
#endif

		

		n->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);
		ui.treeWidget->addTopLevelItem(n);
		
	}else{
		ui.statusBar->showMessage("Maximum number of nodes reached");
		ui.btnNodeAdd->setEnabled(false);
	}
}

void mzFlasher::delNode(){
		if(ui.treeWidget->selectedItems().size() > 0){		
			QTreeWidgetItem *n = ui.treeWidget->selectedItems()[0]->parent();
			if (n == 0){
				n=ui.treeWidget->selectedItems()[0];
			}

			if (QMessageBox(QMessageBox::Question,"","Are you sure want to delete this node?",QMessageBox::Yes| QMessageBox::No,this).exec()== QMessageBox::Yes){
				//ui.treeWidget->removeItemWidget(n,0);
				delete n;
			}
	}
}

void mzFlasher::addDev(){
	//ui.treeWidget->indexOfTopLevelItem(sender);

	if(ui.treeWidget->selectedItems().size() > 0){
		QTreeWidgetItem *p = ui.treeWidget->selectedItems()[0]->parent();

		//if we don't have parent - selected item is top level and adding to it, othewise addint to parent widget
		if (p == NULL)
			p = ui.treeWidget->selectedItems()[0];

		if( p->childCount() < MAXDEVS){
		QTreeWidgetItem *n = new QTreeWidgetItem;

		n->setText(0,"255");
		n->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);

		p->addChild(n);
		p->setExpanded(true);
		}else{
				ui.statusBar->showMessage("Maximum number of devices per node reached");
		}
	}

}

void mzFlasher::delDev(){
	//ui.treeWidget->items
		if(ui.treeWidget->selectedItems().size() > 0){
		
			QTreeWidgetItem *p = ui.treeWidget->selectedItems()[0]->parent();
			if (p > 0){
			//p->setText(0,"t");

			p->removeChild(ui.treeWidget->selectedItems()[0]);
			
			}

	}
}

QByteArray mzFlasher::buidPackage(){
QByteArray p;

	//mz_settings_t s;
	qDebug() << p.toHex();
	//mode
	if (ui.cmbMode->currentIndex() == 0){
		//s.mode = MZ_MASTER;
		p.append((char)MZ_MASTER);
	}else{
		//s.mode = MZ_SLAVE;
		p.append((char)MZ_SLAVE);
	}

		qDebug() << p.toHex();
	//speed
	//s.speed = (mz_line_speed_t)ui.cmdSpeed->currentIndex();
	p.append((char)ui.cmdSpeed->currentIndex());
	//unsigned int spd = mz_speed_val[s.speed];
		qDebug() << p.toHex();

	//nodes packing
	//s.recNum = ui.treeWidget->topLevelItemCount();
		p.append((char)ui.treeWidget->topLevelItemCount());
		qDebug() << p.toHex();

	int i,j;
	for(i = 0; i < ui.treeWidget->topLevelItemCount(); i++){
		mz_node_t n;
		
		//n.rnode = ui.treeWidget->topLevelItem(i)->text(0);
		p.append(str2ieee(&ui.treeWidget->topLevelItem(i)->text(0)));
			qDebug() << p.toHex();
		//n.num = ui.treeWidget->topLevelItem(i)->childCount();
		p.append((char)ui.treeWidget->topLevelItem(i)->childCount());
			qDebug() << p.toHex();
		for (j = 0; j < ui.treeWidget->topLevelItem(i)->childCount(); j++){

			p.append((unsigned char)ui.treeWidget->topLevelItem(i)->child(j)->text(0).toInt());
				qDebug() << p.toHex();
		}

	}


	qDebug()<< "Total: " << p.size();
	qDebug()<< p.toHex(); ;

	return p;
}

/*
	TODO
*/
bool mzFlasher::sanityCheck(){
	bool ret = true;
	//validation pattern
#if (RIMEADDR_SIZE==8)
	QRegExp rx("[0-9,a-f,A-F]{1,2}.[0-9,a-f,A-F]{1,2}.[0-9,a-f,A-F]{1,2}.[0-9,a-f,A-F]{1,2}.[0-9,a-f,A-F]{1,2}.[0-9,a-f,A-F]{1,2}.[0-9,a-f,A-F]{1,2}.[0-9,a-f,A-F]{1,2}");
#else if (RIMEADDR_SIZE==2)
		QRegExp rx("[0-9,a-f,A-F]{1,2}.[0-9,a-f,A-F]{1,2}");
#endif
	int i,j;
	for(i = 0; i < ui.treeWidget->topLevelItemCount(); i++){
		
		if(!rx.exactMatch(ui.treeWidget->topLevelItem(i)->text(0))){
			//invalid entry
			ret = false;
			ui.treeWidget->topLevelItem(i)->setBackground(0,QBrush(Qt::BrushStyle::Dense6Pattern));
			ui.treeWidget->topLevelItem(i)->setBackgroundColor(0,Qt::GlobalColor::red);
		}else{
			//for unhightlighting fixed
			ui.treeWidget->topLevelItem(i)->setBackground(0,QBrush(Qt::BrushStyle::NoBrush));
			ui.treeWidget->topLevelItem(i)->setBackgroundColor(0,Qt::GlobalColor::transparent);

		}

		for (j = 0; j < ui.treeWidget->topLevelItem(i)->childCount(); j++){
			bool good;
			int val = ui.treeWidget->topLevelItem(i)->child(j)->text(0).toInt(&good);
			if (good &&	(val > 255 ||val < 0 )){
				//invalid entry
				ret = false;
				ui.treeWidget->topLevelItem(i)->child(j)->setBackground(0,QBrush(Qt::BrushStyle::Dense6Pattern));
				ui.treeWidget->topLevelItem(i)->child(j)->setBackgroundColor(0,Qt::GlobalColor::red);
			}else{
				//for unhightlighting fixed
				ui.treeWidget->topLevelItem(i)->child(j)->setBackground(0,QBrush(Qt::BrushStyle::NoBrush));
				ui.treeWidget->topLevelItem(i)->child(j)->setBackgroundColor(0,Qt::GlobalColor::transparent);
			}

		}

	}


	return ret;
}

	void mzFlasher::save(){
		QByteArray data;
		if (!sanityCheck()){
			ui.statusBar->showMessage("Config has errors");
			return;//sanity function wiil highlight errors inself
		}else{
			ui.statusBar->clearMessage();
		}

			data = buidPackage();

		if (data.size() < CIB_SIZE_B){
			unsigned short tmp;
			//insetring command word
			tmp = (unsigned short)CMD_WRITE_CNF;

			data.insert(0,(char*)&tmp, sizeof(short));
			
			//adding checksum
			tmp = crc16_ccitt((unsigned char*)data.data(), data.size());			
			data.append((char *)&tmp, sizeof(short));

			//sending
			//port.Write(&data);
			qDebug() << "packet:" << data.toHex();
			send_packet(&data);
		}else{
			ui.statusBar->showMessage("To big config");
		}
	}


	void mzFlasher::load(){
		getMac();
		//get conf will be called sequently in input thread
	}

	void mzFlasher::getConf(){
		//sending read request
		QByteArray data;
		unsigned short tmp;
		//insetring command word
		tmp = (unsigned short)CMD_READ_CNF;
		data.insert(0,(char*)&tmp, sizeof(short));
		
		//adding checksum
		tmp = crc16_ccitt((unsigned char*)data.data(), data.size());			
		data.append((char *)&tmp, sizeof(short));
		
		//sending
		send_packet(&data);
	}

	void mzFlasher::getMac(){

				//sending read request
		QByteArray data;
		unsigned short tmp;
		//insetring command word
		tmp = (unsigned short)CMD_GET_MAC;
		data.insert(0,(char*)&tmp, sizeof(short));
		
		//adding checksum
		tmp = crc16_ccitt((unsigned char*)data.data(), data.size());			
		data.append((char *)&tmp, sizeof(short));
		
		//sending
		send_packet(&data);

	}


	void mzFlasher::InPacket(){
		//checking crc
		unsigned short crcA, crcB;
		qDebug() << "Incoming packet len" << packet.size();
		qDebug() << packet.toHex();
		crcA = (packet.data()[packet.size()-2] << 8) ;
		crcA |= (0xff & (packet.data()[packet.size()-1]));
		crcB =  crc16_ccitt((unsigned char*)packet.data(), packet.size()-2);

		if (crcA == crcB){
			proto_frame_t *pp = (proto_frame_t *)packet.data();
			
			switch(pp->cmd){
			case RPL_PONG: 
					qDebug() << "got PONG!";
					ui.grpCfg->setEnabled(true);
					break;
			case RPL_DATA_CNF: 
				ParsePacket((unsigned char*)(packet.data()+sizeof(pf_cmd_e)));
					break;
			case RPL_DATA_STS:
					//succees
					if(pp->data[0]==0){
						QMessageBox(QMessageBox::Information,"","Configuration saved!",QMessageBox::Ok,this).exec();
					}else{
					QMessageBox(QMessageBox::Critical,"","Save failed!",QMessageBox::Ok,this).exec();
					}
					break;

			case RPL_MAC:				//простите меня, индусы
					ui.lblRime->setText(ieee2str(&QByteArray((char*)pp->data,(RIMEADDR_SIZE))));  
					getConf();
					break;
			default: 
					qDebug() << "Unknown packet with correct crc, id:"<< (short)(pp->cmd);
					break;
			}

		}else{
			qDebug() << "Bad resporse crc: " << crcA << " vs " << crcB;
		}
		packet.clear();
	}


    void mzFlasher::DataIn(quint64 n){
    QByteArray tmp;
	port.Read(&tmp,n);

	//qDebug() << "DataIn"<< n;

	int i;
	unsigned char c;
	for (i=0; i < tmp.size(); i++){
			
			c = tmp[i];
					if (!staffCk) {

					/* handle bytestuffing if necessary
					 */
					switch (c) {

					/* if it's an END character then we're done with
					 * the packet
					 */
					case END:
						{
						/* a minor optimization: if there is no
						 * data in the packet, ignore it. This is
						 * meant to avoid bothering IP with all
						 * the empty packets generated by the
						 * duplicate END characters which are in
						 * turn sent to try to detect line noise.
						 */
						if (packet.size())
							InPacket();
						}
						break;

						/* if it's the same code as an ESC character, wait
						 * and get another character and then figure out
						 * what to store in the packet based on that.
						 */
					case ESC:
						{
						staffCk = true;
						}
						break;
						/* here we fall into the default handler and let
						 * it store the character for us
						 */
					default:
						packet.append(c);
					}

				} else {
					/* if "c" is not one of these two, then we
					 * have a protocol violation.  The best bet
					 * seems to be to leave the byte alone and
					 * just stuff it into the packet
					 */
					switch (c) {
					case ESC_END:
						c = END;
						packet.append(c);
						break;
					case ESC_ESC:
						c = ESC;
						packet.append(c);
						break;
					default:
						packet.clear(); //Truly invalid packet - resetting
					}
					staffCk = false;
				}
	}

    }


	QByteArray mzFlasher::str2ieee(QString *str){
		QByteArray a;
		QStringList l = str->split('.');
		int i;
		for(i = l.size()-1; i >= 0 ; i--){
			a.append((char)l[i].toInt(NULL,16));
			//qDebug() << a.toHex();
		}

		

		return a;
	}

	QString mzFlasher::ieee2str(QByteArray *d){		
		std::reverse(d->begin(), d->end());
		QString t(d->toHex());

		///shit-shit-shit
		for(int i=0; i < RIMEADDR_SIZE-1;i++){
			t.insert(2+i*3,'.');
		}

			/*
		t.insert(2,'.');
		t.insert(5,'.');
		t.insert(8,'.');
		t.insert(11,'.');
		t.insert(14,'.');
		t.insert(17,'.');
		t.insert(20,'.');
		*/
		return t;
	}

	void mzFlasher::ParsePacket(unsigned char *p){

		mz_settings_t *cfg = (mz_settings_t *)p;

		bool validCfg = true;

		//mode detect
		if (cfg->mode == MZ_MASTER){
			ui.cmbMode->setCurrentIndex(0);
		}else if(cfg->mode == MZ_SLAVE){
			ui.cmbMode->setCurrentIndex(1);
		}else{
			qDebug() << "not configured";
			validCfg = false;
			ui.cmbMode->setCurrentIndex(-1);
		}

		//speed is native indeed
		ui.cmdSpeed->setCurrentIndex(cfg->speed);

		if (validCfg){
			ui.treeWidget->clear();

			//nodes parse
			int i,j;
			mz_node_t *nd = &cfg->nodeList;
			for(i=0; i < cfg->recNum; i++){
				//inserting slave
				QTreeWidgetItem *n = new QTreeWidgetItem;
				QByteArray rm((const char *)nd->rnode.u8,RIMEADDR_SIZE);
								
				n->setText(0,ieee2str(&rm));
				n->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);
				ui.treeWidget->addTopLevelItem(n);

				//inserting mb-nodes
				for (j = 0; j < nd->num; j++) {
					QTreeWidgetItem *c = new QTreeWidgetItem;
					c->setText(0,QString::number((&nd->mnodes)[j]));
				c->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);

				n->addChild(c);
				//n->setExpanded(true);	
					
				}
				//verify shift calculation
				nd =(mz_node_t*) (((char*)nd)+NODE_SHIFT(nd));

			}

		}


	}

	void mzFlasher::ping(){
			
		//sending read request
		QByteArray data;
		unsigned short tmp;
		//insetring command word
		tmp = (unsigned short)CMD_PING;
		data.insert(0,(char*)&tmp, sizeof(short));
		
		//adding checksum
		tmp = crc16_ccitt((unsigned char*)data.data(), data.size());			
		data.append((char *)&tmp, sizeof(short));
		
		//sending
		send_packet(&data);
		

	}

	void mzFlasher::srchClick(){
	
		ui.cmbPorts->clear();
		ui.cmbPorts->addItems( port.EnumeratePorts() );

	}