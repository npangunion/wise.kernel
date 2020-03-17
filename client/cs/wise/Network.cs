using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net;
using System.Net.Sockets;
using System.Threading;

namespace wise
{
    public class Network
    {
        public static ILogger Logger
        {
            get;
            private set;
        }

        public static Network Instance
        {
            get;
            private set;
        }

        private static Detail.Channel Channel
        {
            get;
            set;
        }

        /// <summary>
        /// setup static properites
        /// </summary>
        static Network()
        {
            Instance = new Network();
            Logger = CreateLogger();
            Channel = new Detail.Channel();
        }

        /// <summary>
        /// Internal use to parse ip:port string
        /// </summary>
        private class Address
        {
            public string ip;
            public ushort port;    
        }

        private Dictionary<Detail.Acceptor, bool> acceptors;
        private Dictionary<Detail.Connector, bool> connectors;
        private Dictionary<Session, bool> sessions;
        private ReaderWriterLockSlim rwLock; 


        /// <summary>
        /// Hide default constructor
        /// </summary>
        private Network()
        {
            acceptors = new Dictionary<Detail.Acceptor, bool>();
            connectors = new Dictionary<Detail.Connector, bool>();
            sessions = new Dictionary<Session, bool>();
            rwLock = new ReaderWriterLockSlim();
        }

        static ILogger CreateLogger()
        {
            // make it optional for platforms
            return new Detail.NLogBridge();
        }

        /// <summary>
        /// Connect with ip:port string
        /// </summary>
        public Result Connect(string address)
        {
            Address addr = new Address();
            ParseAddress(address, addr);

            return Connect(addr.ip, addr.port);
        }

        public Result Connect(string ip, ushort port)
        {
            var ipaddress = IPAddress.Parse(ip);
            return Connect(ipaddress, port);
        }

        public Result Connect(IPAddress ip, ushort port)
        {
            var connector = new Detail.Connector(ip, port);

            var result = connector.BeginConnect();

            if ( result )
            {
                AddConnector(connector);
            }

            return result;
        }

        /// <summary>
        /// Listen with ip:port string
        /// </summary>
        public Result Listen(string address)
        {
            Address addr = new Address();
            ParseAddress(address, addr);

            return Listen(addr.ip, addr.port);
        }

        public Result Listen(string address, ushort port)
        {
            var ipaddress = IPAddress.Parse(address);
            return Listen(ipaddress, port);
        }

        public Result Listen(ushort port)
        {
            return Listen(IPAddress.Any, port);
        }

        public Result Listen(IPAddress ip, ushort port)
        {
            var acceptor = new Detail.Acceptor(ip, port);

            var result = acceptor.BeginAccept();

            if ( result )
            {
                AddAcceptor(acceptor);
            }

            return result;
        }

        /**
         * Subscribe with a Message prototype and action.
         */
        public Token Subscribe<T>(T m, Action<T> action) where T : Message
        {
            return Channel.Subscribe<T>(m, action);
        }

        /** 
         * Unsubscribe from Network Channel
         */
        public bool Unsubscribe(Token token)
        {
            return Channel.Unsubscribe(token);
        }

        /// <summary>
        /// Internal use. Called from Session
        /// </summary>
        public int Post(Message m)
        {
            return Channel.Post(m);
        }

        internal void OnAccepted(Detail.Acceptor acceptor, Socket socket)
        {
            OnNewSocket(socket, true);
        }

        internal void OnConnected(Detail.Connector connector, Socket socket)
        {
            OnNewSocket(socket, false);

            RemoveConnector(connector);
        }

        internal void OnConnectFailed(Detail.Connector connector)
        {
            RemoveConnector(connector);

            var msg = new SysConnectFailed();
            msg.Address = connector.Address.ToString();
        }

        internal void OnSessionError(Session session)
        {
            RemoveSession(session);

            session.Dispose();

            Logger.Info(string.Format("Session error. {0}", session));

            var msg = new SysSessionClosed();
            msg.Session = session;

            Post(msg);
        }

        private void OnNewSocket(Socket socket, bool accepted)
        {
            var session = new Session(socket, accepted);
            var result = session.BeginRecv();
            
            if ( result )
            {
                AddSession(session);

                Logger.Info(string.Format("NewSocket: {0}", session));

                var msg = new SysSessionReady();
                msg.Session = session;

                Post(msg);
            }
        }

        private void AddAcceptor(Detail.Acceptor acceptor)
        {
            rwLock.EnterWriteLock();
            acceptors.Add(acceptor, true);
            rwLock.ExitWriteLock();
        } 

        private void RemoveAcceptor(Detail.Acceptor acceptor)
        {
            rwLock.EnterWriteLock();
            acceptors.Remove(acceptor);
            rwLock.ExitWriteLock();
        } 

        private void AddConnector(Detail.Connector connector)
        {
            rwLock.EnterWriteLock();
            connectors.Add(connector, true);
            rwLock.ExitWriteLock();
        } 

        private void RemoveConnector(Detail.Connector connector)
        {
            rwLock.EnterWriteLock();
            connectors.Remove(connector);
            rwLock.ExitWriteLock();
        } 

        private void AddSession(Session session)
        {
            rwLock.EnterWriteLock();
            sessions.Add(session, true); 
            rwLock.ExitWriteLock();
        }

        private void RemoveSession(Session session)
        {
            rwLock.EnterWriteLock();
            sessions.Remove(session);
            rwLock.ExitWriteLock();
        }

        private void ParseAddress(string address, Address addr)
        {
            int port = 0;
            int index = address.LastIndexOf(':');

            if (index >= 0 && address.Length > 15)
            {
                // Might be an IPv6 strig without a port specification.
                if (address.Split(':').Length > 6 &&
                    address.LastIndexOf("]:") < 0)
                {
                    index = -1;
                }
            }

            if (0 < index && index < (address.Length - 1))
            {
                string portString = address.Substring(index + 1).Trim();
                if (Int32.TryParse(portString, out port))
                {
                    address = address.Substring(0, index).Trim();
                }
            }

            addr.port = (ushort)port;
            addr.ip = address;
        }
    }
}
