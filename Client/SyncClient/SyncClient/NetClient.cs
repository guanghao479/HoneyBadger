using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net.Sockets;
using System.Threading;
using System.IO;

namespace SyncClient
{
    class NetClient
    {
        private string remoteIP = "127.0.0.1";
        private int remotePort = 3432;
        private TcpClient tcpclient;
        private Thread recvThread;
        private NetworkStream dataStream;
        private StreamReader streamR;
        private bool running = true;
        public NetClient(string ip, int port)
        {
            remoteIP = ip;
            remotePort = port;
        }
        public string start()
        {
            try
            {
                tcpclient = new TcpClient(remoteIP, remotePort);
            }
            catch (Exception e)
            {
                return "connect to server failed,error:" + e.Message;
            }
            dataStream = tcpclient.GetStream();
            streamR = new StreamReader(dataStream);
            recvThread = new Thread(OnReceiveMsg);
            return "";
        }
        public void stop()
        {
            running = false;
            streamR.Close();
            if (tcpclient != null)
            {
                tcpclient.Close();
            }
        }
        public void write(Byte[] data)
        {
            dataStream.Write(data, 0, data.Length);
        }
        private void OnReceiveMsg()
        {
            while (running)
            {
                string strdata = streamR.ReadToEnd();
                byte[] byteArray = System.Text.Encoding.Default.GetBytes(strdata);
                object msg = XmlClass.deserializeXml(byteArray);
            }
        }
    }
}
