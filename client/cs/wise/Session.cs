using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net;
using System.Net.Sockets;
using System.IO;

namespace wise
{
    public class Session : IDisposable
    {
        private long sid = 0;
        private byte[] recvBuf;
        private const int bufferSize = 32 * 1024;

        private object syncRoot;
        private object syncSend;

        protected List<Packet> packetsSending;
        protected List<Packet> packetsAccumulated;
        private MemoryStream sendBuffer;

        private bool sending = false;
        private bool disposed = false;
        private EndPoint localEndPoint;
        private EndPoint remoteEndPoint;
        private Detail.Protocol protocol;

        public long Id
        {
            get { return sid;  }
        }

        public bool Accepted
        {
            get;
            private set;
        }

        private Socket socket;

        public Session(Socket socket, bool accepted)
        {
            syncRoot = new object();
            syncSend = new object();

            this.socket = socket;

            localEndPoint = socket.LocalEndPoint;
            remoteEndPoint = socket.RemoteEndPoint;

            Accepted = accepted;
            recvBuf = new byte[bufferSize];
            protocol = new Detail.Protocol();

            sendBuffer = new MemoryStream();
        }

        public Result BeginRecv()
        {
            try
            {
                socket.BeginReceive(recvBuf, 0, bufferSize, SocketFlags.None, OnRecvCompleted, null);
                return Result.Success();
            } 
            catch (ObjectDisposedException) {
                return Result.Fail(Result.Code.FailSocketDisposed);
            }
            catch (Exception e)
            {
                Network.Logger.Warn(string.Format("{0} recv error {1}", this, e));
                return Result.Fail(Result.Code.FailSessionException);
            }
        }

        public Result Send(Packet p)
        {
            lock (syncRoot)
            {
                if (disposed)
                {
                    return Result.Fail(Result.Code.FailSocketDisposed);
                }
            }

            lock (syncSend)
            { 
                packetsAccumulated.Add(p);

                if (sending)
                {
                    return Result.Success();
                }
            }

            BeginSend();

            return Result.Success();
        }

        public void Disconnect()
        {
            lock (syncRoot)
            {
                if (socket.Connected)
                {
                    socket.Shutdown(SocketShutdown.Both);
                }

                // 여기서 Socket.Close()를 하면 BeginReceive 콜백으로 통지 되지 않는다. 
                // 따라서, 일관된 처리를 위해 여기서 Close하지 않고 Dispose()에서만 닫는다.                 
            }
        }

        public void Dispose()
        {
            lock (syncRoot)
            {
                Dispose(true);
            }

            GC.SuppressFinalize(this);
        }

        public override string ToString()
        {
            return string.Format(
                "Accepted: {0}, Local: {1}, Remote: {2}", 
                Accepted, localEndPoint, remoteEndPoint );
        }

        private void BeginSend()
        { 
            lock (syncSend)
            {
                if ( sending )
                {
                    return; 
                }
                else
                {
                    if (packetsAccumulated.Count == 0)
                    {
                        return;
                    }

                    sending = true;

                    // Swap the event buffers.
                    if (packetsSending.Count != 0)
                    {
                        packetsSending.Clear();
                    }

                    List<Packet> temp = packetsSending;
                    packetsSending = packetsAccumulated;
                    packetsAccumulated = temp;
                    temp = null;
                }
            }

            // rewind
            sendBuffer.Position = 0;

            for ( int i = 0; i<packetsSending.Count; ++i)
            {
                // sendBuffer로 패킷을 처리하여 모은다. 
                var result = protocol.OnSend(packetsSending[i], sendBuffer);

                if ( !result )
                {
                    // recv 쪽에서 에러 처리
                    Disconnect();
                    return;
                }
            }

            try
            {
                // 버퍼가 변경되지 않는다. 소켓은 연결이 끊어지거나 Dispose 될 수 있다. 

                socket.BeginSend(sendBuffer.GetBuffer(), 0, (int)sendBuffer.Position, SocketFlags.None, OnSendCompleted, null);
            }
            catch (ObjectDisposedException)
            {
                // 특별한 처리가 필요없다. 세션 자체 (소켓)이 사라진 상태.
            }
            catch (Exception e)
            {
                Network.Logger.Warn(string.Format("{0} send exception : {1}", this, e));
                Network.Instance.OnSessionError(this);
            }
        }

        private void Dispose(bool disposing)
        {
            // Dispose 호출은 한번만 실행한다. 

            if (disposed) return;

            disposed = true;

            if (socket != null)
            {
                try
                {
                    if (socket.Connected)
                    {
                        socket.Shutdown(SocketShutdown.Both);
                    }
                    socket.Close();
                }
                catch (Exception e)
                {
                    Network.Logger.Warn(
                        string.Format("{0} close exception : {1}", this, e));
                }
            }
        }

        // Asynchronous callback for BeginReceive
        private void OnRecvCompleted(IAsyncResult asyncResult)
        {
            try
            {
                int bytesTransferred = socket.EndReceive(asyncResult);

                if (bytesTransferred > 0)
                {
                    // Disconnect 호출과 BeginReceive간의 동기화를 위해 사용
                    // BeginReceive 호출 전에 연결이 끊어지면 다시 OnRecvCompleted가 호출 되도록 
                    // 확실하게 하도록 순서를 제어하기 위해 락을 사용
                    lock (syncRoot)
                    {
                        var result = protocol.OnRecv(recvBuf, bytesTransferred);

                        if ( !result )
                        {
                            Network.Instance.OnSessionError(this);
                            return;
                        }

                        socket.BeginReceive(recvBuf, 0, bufferSize, SocketFlags.None, OnRecvCompleted, null);
                    }

                    return;
                }

                // (bytesTransferred == 0) implies a graceful shutdown
                Network.Logger.Info(string.Format("Session {0} disconnected", this));
            }
            catch (ObjectDisposedException)
            {
                // 특별한 처리가 필요없다. 세션 자체 (소켓)이 사라진 상태.
                return;
            }
            catch (Exception e)
            {
                var se = e as SocketException;

                if (se != null && se.SocketErrorCode == SocketError.OperationAborted)
                {
                    return;
                }

                Network.Logger.Warn(string.Format("Session {0} recv error {1}", this, e));
            }

            Network.Instance.OnSessionError(this);
        }

        // Asynchronous callback for BeginSend
        private void OnSendCompleted(IAsyncResult asyncResult)
        {
            try
            {
                int bytesTransferred = socket.EndSend(asyncResult);

                lock (syncSend)
                {
                    sending = false;
                }

                BeginSend();
            }
            catch (ObjectDisposedException)
            {
                // 특별한 처리가 필요없다. 세션 자체 (소켓)이 사라진 상태.
            }
            catch (Exception e)
            {
                var se = e as SocketException;
                if (se != null &&
                    se.SocketErrorCode == SocketError.OperationAborted)
                {
                    return;
                }

                Network.Instance.OnSessionError(this);
            }
        }
    }
}
