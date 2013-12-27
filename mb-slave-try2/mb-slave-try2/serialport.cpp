#include "serialport.h"
#include <windows.h>
#include <QDebug>

/**
  *  хглемемн 01.08.2012
  */

/**
  * Windows adopted descriptors structure
  */
struct SerialPortDescriptor {
    HANDLE      dev;        //!< Device descriptor
    HANDLE      ioEvent;    //!< IO Event
    OVERLAPPED  readOL;     //!< Overlapped read operation
    OVERLAPPED  writeOL;    //!< Overlapped write operation
    DCB         comm;       //!< Communication properties
};


// Both timeouts are 200 ms
const int SerialPort::DefaultReadTimeout  = 200;
const int SerialPort::DefaultWriteTimeout = 200;

//SerialPort::SerialPort(const QString &name, int insize, int outsize) :
//    m_name(name), m_ready(false), m_readTimeout(DefaultReadTimeout),
//    m_writeTimeout(DefaultWriteTimeout), m_inQueue(insize), m_outQueue(outsize)
//{
//    m_spd = new SerialPortDescriptor();
//    memset(m_spd, 0, sizeof(SerialPortDescriptor));

//    m_spd->ioEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
//    ResetEvent(m_spd->ioEvent);

//    m_spd->readOL.hEvent = m_spd->ioEvent;
//    m_spd->comm.DCBlength = sizeof(DCB);
//}

SerialPort::SerialPort(int insize, int outsize) :
    m_ready(false), m_readTimeout(DefaultReadTimeout),
    m_writeTimeout(DefaultWriteTimeout), m_inQueue(insize), m_outQueue(outsize)
{
    m_spd = new SerialPortDescriptor();
    memset(m_spd, 0, sizeof(SerialPortDescriptor));

/*    m_spd->ioEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    ResetEvent(m_spd->ioEvent);

    m_spd->readOL.hEvent = m_spd->ioEvent;
    m_spd->comm.DCBlength = sizeof(DCB);
	*/
}

SerialPort::~SerialPort()
{
    if (IsOpen() && IsValid())
        Close();

    // Free port descriptors
    delete m_spd;
}

/*void SerialPort::Open()
{
    QString path = m_name;  // name still canonical name without prefixes
    path.insert(0, QString("\\\\.\\"));
    m_spd->dev = CreateFileA(path.toAscii().data(),
                             GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
                             FILE_FLAG_OVERLAPPED, 0);
    if (m_spd->dev != INVALID_HANDLE_VALUE)
        m_ready = true;
    else
        throw SerialPortException();

    SetupComm(m_spd->dev, m_inQueue, m_outQueue);

    GetCommState(m_spd->dev, &m_spd->comm);
    m_spd->comm.fBinary = 1;
    m_spd->comm.ByteSize = 8;
    SetCommState(m_spd->dev, &m_spd->comm);
}
*/

void SerialPort::Open(QString name)
{
	m_spd->ioEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    ResetEvent(m_spd->ioEvent);

    m_spd->readOL.hEvent = m_spd->ioEvent;
    m_spd->comm.DCBlength = sizeof(DCB);

    m_name = name;
    QString path = m_name;  // name still canonical name without prefixes
    path.insert(0, QString("\\\\.\\"));
    m_spd->dev = CreateFileA(path.toAscii().data(),
                             GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
                             FILE_FLAG_OVERLAPPED, 0);
    if (m_spd->dev != INVALID_HANDLE_VALUE)
        m_ready = true;
    else
        throw SerialPortException();

    SetupComm(m_spd->dev, m_inQueue, m_outQueue);

    GetCommState(m_spd->dev, &m_spd->comm);
    m_spd->comm.fBinary = 1;
    m_spd->comm.ByteSize = 8;
    SetCommState(m_spd->dev, &m_spd->comm);
}

void SerialPort::Close()
{
    if (IsListening())
        CancelListening();

    CloseHandle(m_spd->ioEvent);
    CloseHandle(m_spd->dev);
    m_spd->dev = 0;
    m_ready = false;
}

void SerialPort::CancelListening()
{
    m_stopListen = true;
    /**
     * @todo wait until thread finish (or better cancel all pending operations) to
     *       optimize termination time
     */
    QThread::currentThread()->wait(ULONG_MAX); // block calling thread until complete
}

bool SerialPort::IsValid() const
{
    /// @todo check MSDN to find out how to find out that port became unavailable
    return true;
}

QString SerialPort::Name() const
{
    return m_name;
}

ssize_t SerialPort::ReadPrivate(QByteArray *buffer, size_t len)
{
    DWORD read; // actualy read bytes
    ssize_t result = 0;

    char *buff = new char[len];

//    qDebug() << "Read start";
    if (!ReadFile(m_spd->dev, buff, len, &read, &m_spd->readOL))
    {
        if (WaitForSingleObject(m_spd->readOL.hEvent, INFINITE) == WAIT_OBJECT_0)
        {
            // Let other pending operations to complete
            ResetEvent(m_spd->readOL.hEvent);

            if (GetOverlappedResult(m_spd->dev, &m_spd->readOL, &read, FALSE))
            {
                buffer->append(buff, read);
                result = read;

//                qDebug() << "Buffer read " << read << " bytes";

            }else {
                CancelIo(m_spd->readOL.hEvent);
                result = (-1);
//                qDebug() << "Read failed";
            }
        }
    }
    else
    {
        // Operation can complete immediately
        buffer->append(buff, read);
        result = read;
//        qDebug() << "Buffer read sync " << read << " bytes";
    }

    delete[] buff;
//    qDebug() << "Read end";

    return result;
}

void SerialPort::SetListen(bool flag)
{
    if (flag)
    {
        // Start receiver thread
        m_stopListen = false;
        start();
    }else
    {
        // Stop receiver thread
        m_stopListen = true;
        wait();
    }
}

void SerialPort::run()
{
    COMSTAT stat;
    DWORD mask = 0;
    DWORD read;
    size_t length = 0;
    OVERLAPPED waitOL;

    memset(&waitOL, 0, sizeof(OVERLAPPED));

    waitOL.hEvent = CreateEventA(NULL, TRUE, FALSE, NULL);

    // Wait for any char has come or event char
    SetCommMask(m_spd->dev, (m_spd->comm.EvtChar ? EV_RXFLAG : EV_RXCHAR));

    QByteArray arr;

    while(!m_stopListen)
    {
        arr.clear();
        if (WaitCommEvent(m_spd->dev, &mask, &waitOL))
        {
            if ((mask & EV_RXCHAR) || (mask & EV_RXFLAG)){
                // Get comm status
                if (ClearCommError(m_spd->dev, NULL, &stat))
                {
                    length = stat.cbInQue;
//                    qDebug() << "COM:" << length << "bytes ready";

                    ReadPrivate(&arr, length);

                    m_mutex.lock();
                    m_buffer.append(arr);
                    m_mutex.unlock();

                    if (length > 0)
                        emit DataReceived(length);
                }
            }
        }
        else
        {
            int res = GetLastError();

            switch(res)
            {
                case ERROR_IO_PENDING:
                    res = WaitForSingleObject(waitOL.hEvent, m_readTimeout);

                    ResetEvent(waitOL.hEvent);

                    if (res == WAIT_OBJECT_0)
                    {
                        if (GetOverlappedResult(m_spd->dev, &waitOL, &read, FALSE))
                        {
                            if ((mask & EV_RXCHAR) || (mask & EV_RXFLAG))
                            {
                                if (ClearCommError(m_spd->dev, NULL, &stat))
                                {
                                    length = stat.cbInQue;
//                                    qDebug() << "COM:" << length << "bytes ready";
//                                    qDebug() << "Thread:" << QThread::currentThreadId();

                                    ReadPrivate(&arr, length);

                                    m_mutex.lock();
                                    m_buffer.append(arr);
                                    m_mutex.unlock();

                                    if (length)
                                         emit DataReceived(length);
                                }
                            }
                        }
                    }
                    else if (res == WAIT_TIMEOUT)
                    {
                        CancelIo(m_spd->dev);
                    }
                    break;
                case ERROR_TIMEOUT:
                    CancelIo(m_spd->dev);
                    break;
            }
        }

        mask = 0;
    }

    CloseHandle(waitOL.hEvent);
}

ssize_t SerialPort::Read(QByteArray *buffer, size_t len)
{
    int copyLen = 0;

//    if (!m_stopListen)
    {
        // we are using internal buffer
        m_mutex.lock();
        copyLen = (len <= m_buffer.size()) ? len : m_buffer.size();

        buffer->append(m_buffer.left(copyLen));
        m_buffer.remove(0, copyLen);
        m_mutex.unlock();
    }
//    else
    {
//        copyLen = ReadPrivate(buffer, len);
    }
    return copyLen;
}

ssize_t SerialPort::Write(const QByteArray *buffer)
{
    DWORD written = 0;
    ssize_t result = 0;

    //OVERLAPPED IOSyncOL;
 	    if (!WriteFile(m_spd->dev, buffer->constData(), buffer->size(),
                   &written, &m_spd->writeOL))
    {
        if (WaitForSingleObject(m_spd->writeOL.hEvent, INFINITE) == WAIT_OBJECT_0)
        {
            ResetEvent(m_spd->writeOL.hEvent);

            if (GetOverlappedResult(m_spd->dev, &m_spd->writeOL, &written, FALSE))
            {
                result = written;
//                qDebug() << "Written " << written << " bytes";
            }else
            {
                CancelIo(m_spd->writeOL.hEvent);
                result = (-1);
//                qDebug() << "Write failed";
            }
        }
        else
        {
            CancelIo(m_spd->writeOL.hEvent);
            result = (-1);
        }
    }
	
    CloseHandle(m_spd->writeOL.hEvent);

    // Return successfully
    return result;
}

bool SerialPort::IsOpen() const
{
    return m_ready;
}

void SerialPort::SetBaudrate(Baudrate baud)
{
    GetCommState(m_spd->dev, &m_spd->comm);
    m_spd->comm.BaudRate = (DWORD) baud; //482000;
    SetCommState(m_spd->dev, &m_spd->comm);
}

void SerialPort::SetEventChar(char c)
{
    GetCommState(m_spd->dev, &m_spd->comm);
    m_spd->comm.EvtChar = c;
    SetCommState(m_spd->dev, &m_spd->comm);
}

void SerialPort::SetParity(Parity parity)
{
    GetCommState(m_spd->dev, &m_spd->comm);
    m_spd->comm.Parity = (BYTE) parity;
    m_spd->comm.fParity = (ParNone != parity);
    SetCommState(m_spd->dev, &m_spd->comm);
}

void SerialPort::SetHandshake(Handshake handshake)
{
    GetCommState(m_spd->dev, &m_spd->comm);

    switch(handshake) {

    case HandshakeOff:
        m_spd->comm.fDtrControl     = DTR_CONTROL_DISABLE;
        m_spd->comm.fRtsControl     = RTS_CONTROL_DISABLE;
        m_spd->comm.fOutxCtsFlow    = false;
        m_spd->comm.fOutxDsrFlow    = false;
        m_spd->comm.fInX            = false;
        m_spd->comm.fOutX           = false;
        break;

    case HandshakeSoftware:
        m_spd->comm.fDtrControl     = DTR_CONTROL_ENABLE;
        m_spd->comm.fRtsControl     = RTS_CONTROL_ENABLE;
        m_spd->comm.fOutxCtsFlow    = true;
        m_spd->comm.fOutxDsrFlow    = true;
        m_spd->comm.fInX            = true;
        m_spd->comm.fOutX           = true;
        break;

    case HandshakeHardware: // Enable full hardware handshake
        m_spd->comm.fDtrControl     = DTR_CONTROL_HANDSHAKE;
        m_spd->comm.fRtsControl     = RTS_CONTROL_HANDSHAKE;
        m_spd->comm.fOutxCtsFlow    = true;
        m_spd->comm.fOutxDsrFlow    = true;
        m_spd->comm.fOutX           = false;
        m_spd->comm.fInX            = false;
        break;

    case HandshakeUnknown:
        break;
    }

    SetCommState(m_spd->dev, &m_spd->comm);
}

void SerialPort::SetDataBits(DataBits bits)
{
    GetCommState(m_spd->dev, &m_spd->comm);
    m_spd->comm.ByteSize = (BYTE) bits;
    SetCommState(m_spd->dev, &m_spd->comm);
}

void SerialPort::SetStopBits(StopBits bits)
{
    GetCommState(m_spd->dev, &m_spd->comm);
//    m_spd->comm.ByteSize = (BYTE) Data8;
    m_spd->comm.StopBits = (BYTE) bits;
    SetCommState(m_spd->dev, &m_spd->comm);
}

void SerialPort::SetReadTimeout(long timeout)
{
    m_readTimeout = timeout;
}

void SerialPort::SetWriteTimeout(long timeout)
{
    m_writeTimeout = timeout;
}

bool SerialPort::IsListening() const
{
    return !m_stopListen;
}

// Windows specific serial ports examination function
QList<QString> SerialPort::EnumeratePorts()
{
    QList<QString> ports;
    HKEY key;
    long res = 0;

    // we do belive that microsoft will never change intrface to unicode-only
    // implementation and this will work long time.
    // (unfortunately Qt lacks support of registry because of portability purposes)
    res = RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                        "HARDWARE\\DEVICEMAP\\SERIALCOMM",
                        0, KEY_READ, &key);
    if (res == ERROR_SUCCESS)
    {
        DWORD i = 0;
        CHAR regValName[255];
        BYTE regValue[255];
        DWORD regNameLen = 255;
        DWORD regValueLen = 255;
        DWORD type = 0;

//        qDebug() << "Enumerating values";
        while( (res = RegEnumValueA(key, i, regValName, &regNameLen, 0,
                                   &type, regValue, &regValueLen )) == ERROR_SUCCESS)
        {
            char str[255] = {0,};
            memcpy(str, regValue, regValueLen);
//            qDebug() << "Value index: " << i << " value: " << str;
            ports.append(QString(str));

            // Read next value
            regValueLen = 255;
            regNameLen = 255;
            ++i;
        }
    }   

    return ports;
}
