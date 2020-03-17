using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net;
using System.Net.Sockets;

namespace wise.Detail
{
    internal class Connector
    {
        private IPEndPoint endpoint;
        private Socket socket;

        public Connector(IPAddress ip, ushort port)
        {
            endpoint = new IPEndPoint(ip, port);
        }

        public IPAddress Address
        {
            get { return endpoint.Address; }
        }

        public Result BeginConnect()
        {
            try
            {
                socket = new Socket(endpoint.AddressFamily, SocketType.Stream, ProtocolType.Tcp);
                socket.BeginConnect(endpoint, OnConnected, socket);
                return Result.Success();
            }
            catch (Exception e)
            {
                Network.Logger.Error(string.Format("{0} error connecting to {1}", e, endpoint));
                return Result.Fail(Result.Code.FailConnectException);
            }
        }

        // Asynchronous callback for BeginConnect
        private void OnConnected(IAsyncResult asyncResult)
        {
            var socket = (Socket)asyncResult.AsyncState;
            try
            {
                socket.EndConnect(asyncResult);
                Network.Instance.OnConnected(this, socket);
            }
            catch (Exception e)
            {
                Network.Logger.Warn(string.Format("{0} error connecting to {1} : {2}", e, endpoint, e));
                Network.Instance.OnConnectFailed(this);
            }
        }
    }
}
