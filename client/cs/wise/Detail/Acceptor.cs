using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net;
using System.Net.Sockets;

namespace wise.Detail
{
    internal class Acceptor
    {
        private IPEndPoint endpoint;
        private Socket socket;

        public Acceptor(IPAddress ip, ushort port)
        {
            endpoint = new IPEndPoint(ip, port);
        }

        public Result BeginAccept()
        {
            try
            {
                socket = new Socket(endpoint.Address.AddressFamily, SocketType.Stream, ProtocolType.Tcp);
                socket.Bind(endpoint);
                socket.Listen(Int32.MaxValue);

                Network.Logger.Info(string.Format("Listening on {0}", endpoint));

                socket.BeginAccept(OnAccepted, null);

                return Result.Success();
            }
            catch (Exception e)
            {
                Network.Logger.Error(string.Format("{0} error listening on {1}", e, endpoint));
                return Result.Fail(Result.Code.FailListeningException);
            }
        }

        // Asynchronous callback for BeginAccept
        private void OnAccepted(IAsyncResult asyncResult)
        {
            Socket clientSocket = null;

            try
            {
                clientSocket = socket.EndAccept(asyncResult);
            }
            catch (ObjectDisposedException)
            {
                Network.Logger.Info(string.Format("{0} listening socket closed", endpoint));
                return;
            }
            catch (Exception e)
            {
                Network.Logger.Error(string.Format("{0} accept error : {1}", endpoint, e.Message));
            }

            if (!ReferenceEquals(clientSocket, null))
            {
                try
                {
                    Network.Instance.OnAccepted(this, clientSocket);
                }
                catch (ObjectDisposedException)
                {
                    // log
                }
                catch (Exception e)
                {
                    Network.Logger.Error(string.Format("{0} accept error : {1}", endpoint, e));
                }
            }

            socket.BeginAccept(OnAccepted, null);
        }
    }
}
