/**
  * Serial Port access class. Allow user to find out available serial ports
  * and work with any serial port.
  *
  * Provides basic functions to read, write and change settings of port.
  * Allow Qt users to use SIGNALS and SLOTS to access data.
  *
  * @author Alexander Shaykhrazeev
  * @date 23/05/2012
  *
  * Copyright© AISS Ltd, 2012
  */

#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <QObject>
#include <QList>
#include <QString>
#include <QByteArray>
#include <exception>
#include <QThread>
#include <QMutexLocker>
#include <qendian.h>

typedef size_t ssize_t;

/**
  * Platform dependent Serial port descriptor.
  */
struct SerialPortDescriptor;

class SerialPortException : public std::exception
{
public:
    SerialPortException() throw() {}
    ~SerialPortException()throw() {}
};

/**
  * Serial Port use to have a control under serial interface on
  * PC and other POSIX/Windows capatible devices.
  */
class SerialPort : public QThread
{
    Q_OBJECT

public:

    /**
      * Serial port Baudrate
      */
    enum Baudrate
    {
        BaudUnknown = -1,     ///< Unknown
        Baud110     = 110,    ///< 110 bits/sec
        Baud300     = 300,    ///< 300 bits/sec
        Baud600     = 600,    ///< 600 bits/sec
        Baud1200    = 1200,   ///< 1200 bits/sec
        Baud2400    = 2400,   ///< 2400 bits/sec
        Baud4800    = 4800,   ///< 4800 bits/sec
        Baud9600    = 9600,   ///< 9600 bits/sec
        Baud14400   = 14400,  ///< 14400 bits/sec
        Baud19200   = 19200,  ///< 19200 bits/sec (default)
        Baud38400   = 38400,  ///< 38400 bits/sec
        Baud56000   = 56000,  ///< 56000 bits/sec
        Baud57600   = 57600,  ///< 57600 bits/sec
        Baud115200  = 115200, ///< 115200 bits/sec
        Baud128000  = 128000, ///< 128000 bits/sec
        Baud256000  = 256000, ///< 256000 bits/sec
		Baud460800	= 460800, ///< 460800 bits/sec
		Baud921600	= 921600, ///< 921600 bits/sec
		Baud1250000 = 1250000 ///< 1250000 bits/sec
    };

    /**
      * Serial port data bits
      */
    enum DataBits
    {
        DataUnknown = -1, ///< Unknown
        Data5       = 5,  ///< 5 bits per byte
        Data6       = 6,  ///< 6 bits per byte
        Data7       = 7,  ///< 7 bits per byte
        Data8       = 8	  ///< 8 bits per byte (default)
    };

    /**
      * Serial port parity check mode
      */
    enum Parity
    {
        ParUnknown = -1,  ///< Unknown
        ParNone    = 0,   ///< No parity (default)
        ParOdd     = 1,   ///< Odd parity
        ParEven    = 2,   ///< Even parity
        ParMark    = 3,   ///< Mark parity
        ParSpace   = 4    ///< Space parity
    };

    /**
      * Serial port handshake mode
      */
    enum Handshake
    {
        HandshakeUnknown  = -1, ///< Unknown
        HandshakeOff      = 0,  ///< No handshaking
        HandshakeHardware = 1,  ///< Hardware handshaking (RTS/CTS)
        HandshakeSoftware = 2	///< Software handshaking (XON/XOFF)
    };

    /**
      * Serial port stop bits mode
      */
    enum StopBits
    {
        StopUnknown = -1,	///< Unknown
        Stop1       = 0,	///< 1 stopbit (default)
        Stop1_5     = 1,    ///< 1.5 stopbit
        Stop2       = 2     ///< 2 stopbits
    };

    /**
      * Serial port timeout on read mode
      */
    enum ReadTimeout
    {
        ReadTimeoutUnknown     = -1,///< Unknown
        ReadTimeoutNonblocking = 0, ///< Always return immediately
        ReadTimeoutBlocking    = 1	///< Block until everything is retrieved
    };

    /**
      * Serial port errors
      */
    enum Error
    {
        ErrorUnknown = 0, ///< Unknown
        ErrorBreak,       ///< Break condition detected
        ErrorFrame,       ///< Framing error
        ErrorIOE,         ///< I/O device error
        ErrorMode,        ///< Unsupported mode
        ErrorOverrun,     ///< Character buffer overrun, next byte is lost
        ErrorRxOver,      ///< Input buffer overflow, byte lost
        ErrorParity,      ///< Input parity error
        ErrorTxFull       ///< Output buffer full
    };

    /**
      * Serial port states
      */
    enum Port
    {
        PortUnknownError = -1,///< Unknown error occurred
        PortAvailable    = 0, ///< Port is available
        PortNotAvailable = 1, ///< Port is not present
        PortInUse        = 2  ///< Port is in use
    };

	/**
	  * Contains serial port settings
	  */
	struct SerialPortSettings
	{
		Baudrate	baudrate;
		DataBits	dataBits;
		Parity		parity;
		Handshake	handshake;
		StopBits	stopBits;
		QString		portName;

		QString getDescription()
		{
			QString strPar;
			QString strStop;

			switch(parity)
			{
			case(ParNone):	strPar = "Нет"; break;
			case(ParOdd):	strPar = "Нечет"; break;
			case(ParEven):	strPar = "Чет"; break;
			case(ParMark):	strPar = "Маркер"; break;
			case(ParSpace): strPar = "Пробел"; break;
			}

			switch (stopBits)
			{
			case(Stop1):	strStop = "1"; break;
			case(Stop1_5):	strStop = "1.5"; break;
			case(Stop2):	strStop = "2"; break;
			}

			return QString("Имя: %1; Скорость: %2; Длина слова: %3; Стоп-биты: %4; Контроль четности: %5;")
				.arg(portName, QString::number(baudrate), QString::number(dataBits), strStop, strPar);
		}
	};

    /**
      * Default value for read operation timeout timer
      */
    static const int DefaultReadTimeout;

    /**
      * Default value for write operation timeout timer
      */
    static const int DefaultWriteTimeout;

    /**
      * Create new instance of serial port
      *
      * @param name Name of serial port to open
      * @param insize Size of input buffer
      * @param outsize Size of output buffer
      */
    SerialPort(int insize = 1200, int outsize = 1200);

    ~SerialPort();

    /**
      * Open port and lock resources
      */
    void Open(QString name);

    /**
      * Close current port and free resources
      */
    void Close();

    /**
      * Read specified amount of bytes
      *
      * @param buffer Bytes array to store read data
      * @param len Length of data to read
      *
      * @return Amount of actualy read bytes, -1 on error
      */
    ssize_t Read(QByteArray *buffer, size_t len);

    /**
      * Write specified amount of bytes
      *
      * @param buffer Buffer to write to port
      * @param len Length of write buffer
      *
      * @remarks Operation is synchronous
      *
      * @return Amount of data actualy written
      */
    ssize_t Write(const QByteArray *buffer);

    /**
      * Check if port is been opened
      *
      * @return true if port is been opened
      */
    bool IsOpen() const;

    /**
      * Set the char used to generate event (such as SLIP END received)
      *
      * @param c Event char
      */
    void SetEventChar(char c);

    /**
      * Set serial port baudrate
      *
      * @param baud Baudrate to use
      */
    void SetBaudrate(Baudrate baud);

    /**
      * Set serial port parity mode
      *
      * @param parity Parity mode to use
      */
    void SetParity(Parity parity);

    /**
      * Set stop bits
      *
      * @param bits Stop-bits mode
      */
    void SetStopBits(StopBits bits);

    /**
      * Set data bits (добавлено 17.08)
      *
      * @param bits Data-bits
      */
    void SetDataBits(DataBits bits);

    /**
      * Set serial port handshake mode
      *
      * @param handshake Handshake mode
      */
    void SetHandshake(Handshake handshake);

    /**
      * Set serial port read timeout value. Use 0 to engage INFINITY.
      *
      * @param timeout Timeout value
      */
    void SetReadTimeout(long timeout);

    /**
      * Set serial port write timeout value. Use 0 to engage INFINITY.
      *
      * @param timeout Timeout value
      */
    void SetWriteTimeout(long timeout);

    /**
      * Start listen to incoming data on serial port
      *
      * @param flag Set listen mode active if true and disable on false.
      */
    void SetListen(bool flag);

    /**
      * Check if port is listening for a data
      *
      * @return true if port is set to listen mode
      */
    bool IsListening() const;

    /**
      * Cancels listening for port data.
      * Port become usual serial read/write port.
      */
    void CancelListening();

    /**
      * Check if port still valid during it's work.
      *
      * Because user can unplug some ports (bluetooth, usb) during work,
      * so we need a way to find out this change. If port gone or disabled we
      * will know it here.
      *
      * @return true if port still valid and false if port disabled or has gone.
      */
    bool IsValid() const;

    /**
      * Get current port name
      *
      * @return Name of current port
      */
    QString Name() const;

signals:
    /**
      * Raised when some data has arrived to serial port
      *
      * @param length Length of array ready to read
      */
    void DataReceived(quint64);

private:
    QString m_name;                 //!< Serial port name
    SerialPortDescriptor *m_spd;    //!< Serial port descriptor
    bool m_ready;                   //!< Port ready
    long m_readTimeout;             //!< Port read timeout
    long m_writeTimeout;            //!< Port write timeout
    int m_inQueue;                  //!< Size of input queue
    int m_outQueue;                 //!< Size of output queue
    volatile bool m_stopListen;     //!< Listen stop condition
    mutable QMutex m_mutex;         //!< Mutext to protect internal buffer from change
    mutable QByteArray  m_buffer;   //!< Internal buffer to store data that was read
	unsigned int lastErr;
protected:
    /**
      * Runs the wait thread to wait for incoming data. The most of time
      * thread will sleep while wait for incoming data.
      */
    void run();

    /**
      * Read specified amount of bytes
      *
      * @param buffer Bytes array to store read data
      * @param len Length of data to read
      *
      * @remarks Operation is synchronous
      *
      * @return Amount of actualy read bytes, -1 on error
      */
    ssize_t ReadPrivate(QByteArray *buffer, size_t len);

public:
    /**
      * Enumerates all available serial ports by using
      * registry records in Windows NT®.
      *
      * @return List of available ports names
      */
    static  QStringList EnumeratePorts();/*QList<QString>*/
	unsigned int getLastError();
};

#endif // SERIALPORT_H
