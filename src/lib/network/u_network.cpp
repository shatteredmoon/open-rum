#include <network/u_network.h>

#include <network/u_connection.h>
#include <network/u_packet.h>

#include <u_assert.h>
#include <u_events.h>
#include <u_log.h>
#include <u_rum.h>
#include <u_utility.h>

#include <fstream>
#include <mutex>

//#undef NETWORK_DEBUG
//#define NETWORK_DEBUG 1


namespace rumNetwork
{
  // A generic buffer for global reuse
  constexpr int32_t g_uiBufferSize{ 1024 * 1024 };
  char g_cBuffer[g_uiBufferSize];

  // The main connection socket
  SOCKET g_iSocket{ INVALID_SOCKET };

  // The global packet queues, all access must be protected by a mutex
  PacketQueue g_cPacketQueueArray[NUM_PACKET_QUEUES];
  std::mutex g_cMutexArray[NUM_PACKET_QUEUES];

  DownloadQueue g_cDownloadQueue;
  std::mutex g_cDownloadGuard;

  DownloadVector g_cDownloadVector;
  constexpr uint32_t g_iNumConcurrentDownloads{ 4 };

  ConnectionStatus g_eConnectionStatus{ ConnectionStatus::Disconnected };

  // Function Declarations
  SOCKET ConnectTCP( const std::string& i_strAddress, int32_t i_iPort );


  void ClearDownloadQueue()
  {
    std::lock_guard<std::mutex> cLockGuard( g_cDownloadGuard );

    while( !g_cDownloadQueue.empty() )
    {
      g_cDownloadQueue.pop();
    }
  }


  void ClearPacketQueue()
  {
    rumPacket* pcPacket{ nullptr };

    for( int32_t i = 0; i < NUM_PACKET_QUEUES; ++i )
    {
      // Lock the appropriate mutex
      std::lock_guard<std::mutex> cLockGuard( g_cMutexArray[i] );

      while( !g_cPacketQueueArray[i].empty() )
      {
        pcPacket = g_cPacketQueueArray[i].front();
        g_cPacketQueueArray[i].pop();
        delete pcPacket;
      }
    }
  }


  SOCKET Connect( const std::string& i_strAddr, int32_t i_iPort )
  {
    if( ConnectionStatus::Connected == g_eConnectionStatus )
    {
      return g_iSocket;
    }

    g_eConnectionStatus = ConnectionStatus::Connecting;
    g_iSocket = ConnectTCP( i_strAddr, i_iPort );
    if( g_iSocket != INVALID_SOCKET )
    {
      g_eConnectionStatus = ConnectionStatus::Connected;
    }
    else
    {
      g_eConnectionStatus = ConnectionStatus::Failed;
    }

    return g_iSocket;
  }


  SOCKET ConnectTCP( const std::string& i_strAddr, int32_t i_iPort )
  {
    if( i_strAddr.empty() )
    {
      return INVALID_SOCKET;
    }

    // Try to connect to server
    in_addr cInAddr;
    inet_pton( AF_INET, i_strAddr.c_str(), &cInAddr.s_addr );

    addrinfo cHints;
    memset( &cHints, 0, sizeof addrinfo );
    cHints.ai_family = AF_INET;
    cHints.ai_socktype = SOCK_STREAM;
    cHints.ai_protocol = IPPROTO_TCP;

    bool bAddrValid{ false };

    SOCKET iSocket{ INVALID_SOCKET };

    if( !isdigit( i_strAddr[0] ) )
    {
      // Server provided as a name, not IP
      addrinfo* pcAddrs{ nullptr };

      char strPort[16];
      sprintf( strPort, "%d", i_iPort );

      if( getaddrinfo( i_strAddr.c_str(), strPort, &cHints, &pcAddrs ) == 0 )
      {
        bAddrValid = true;
      }
      else
      {
        std::string strError{ "Failed getaddrinfo for " };
        strError += i_strAddr;
#ifdef WIN32
        strError += " error ";
        strError += rumStringUtils::ToString( WSAGetLastError() );
#endif
        Logger::LogStandard( strError, Logger::LOG_ERROR );

        return INVALID_SOCKET;
      }

#if NETWORK_DEBUG
      for( addrinfo *p = pcAddrs; p != nullptr; p = p->ai_next )
      {
        void* pcAddr{ nullptr };
        char* strIPVer{ nullptr };

        // Get the pointer to the address itself
        if( p->ai_family == AF_INET )
        {
          // IPv4
          struct sockaddr_in* ipv4{ (struct sockaddr_in*)p->ai_addr };
          pcAddr = &( ipv4->sin_addr );
          strIPVer = "IPv4";
        }
        else
        {
          // IPv6
          struct sockaddr_in6* ipv6{ (struct sockaddr_in6*)p->ai_addr };
          pcAddr = &( ipv6->sin6_addr );
          strIPVer = "IPv6";
        }

        // Convert the IP to a string and print it
        char ipstr[INET6_ADDRSTRLEN];
        inet_ntop( p->ai_family, pcAddr, ipstr, sizeof ipstr );
        RUM_COUT( "  " << strIPVer << ": " << ipstr << '\n' );
      }
#endif // NETWORK_DEBUG

      // Create a socket
      iSocket = socket( pcAddrs->ai_family, pcAddrs->ai_socktype, pcAddrs->ai_protocol );
      if( INVALID_SOCKET == iSocket )
      {
        Logger::LogStandard( "Error: Could not create socket" );
        return INVALID_SOCKET;
      }

      // Connect to the server
      const int32_t eResult{ connect( iSocket, pcAddrs->ai_addr, (int32_t)pcAddrs->ai_addrlen ) };
      if( SOCKET_ERROR == eResult )
      {
        std::string strErr{ "Error: Could not connect to server " };
        strErr += i_strAddr;
        strErr += ":";
        strErr += rumStringUtils::ToString( i_iPort );
        Logger::LogStandard( strErr );

        return INVALID_SOCKET;
      }

      freeaddrinfo( pcAddrs );
    }
    else
    {
      // Fill in address structure
      sockaddr_in cAddrIn;
      cAddrIn.sin_port = htons( i_iPort );
      cAddrIn.sin_family = AF_INET;
      cAddrIn.sin_addr.s_addr = cInAddr.s_addr;

      char strHostName[NI_MAXHOST];

      // The server provided is an ip address, try to resolve it first
      if( getnameinfo( (const SOCKADDR*)&cAddrIn, sizeof( sockaddr_in ), strHostName, NI_MAXHOST, 0, 0, 0 ) == 0 )
      {
        bAddrValid = true;
        cHints.ai_addr = (SOCKADDR*)&cAddrIn;
        cHints.ai_addrlen = sizeof( cAddrIn );
      }
      else
      {
        std::string strError{ "Address " };
        strError += i_strAddr;
        strError += " is invalid or unavailable";
#ifdef WIN32
        strError += ", error ";
        strError += rumStringUtils::ToString( WSAGetLastError() );
#endif
        Logger::LogStandard( strError, Logger::LOG_ERROR );
      }

      if( !bAddrValid )
      {
        return INVALID_SOCKET;
      }

      // Create a socket
      iSocket = socket( cHints.ai_family, cHints.ai_socktype, cHints.ai_protocol );
      if( INVALID_SOCKET == iSocket )
      {
        Logger::LogStandard( "Error: Could not create socket" );
        return INVALID_SOCKET;
      }

      // Connect to the server
      const int32_t eResult{ connect( iSocket, cHints.ai_addr, (int32_t)cHints.ai_addrlen ) };
      if( SOCKET_ERROR == eResult )
      {
        std::string strErr{ "Error: Could not connect to server " };
        strErr += i_strAddr;
        strErr += ":";
        strErr += rumStringUtils::ToString( i_iPort );
        Logger::LogStandard( strErr );

        return INVALID_SOCKET;
      }
    }

    return iSocket;
  }


  rumPacket* DequeuePacket( PacketQueueType i_eQueueType )
  {
    rumPacket* pcPacket{ nullptr };

    // Avoid the mutex lock if there is nothing in the queue
    if( g_cPacketQueueArray[i_eQueueType].size() > 0 )
    {
      std::lock_guard<std::mutex> cLockGuard( g_cMutexArray[i_eQueueType] );

      pcPacket = g_cPacketQueueArray[i_eQueueType].front();
      g_cPacketQueueArray[i_eQueueType].pop();
    }

    return pcPacket;
  }


  // Copy the global packet queue to a provided packet queue
  void DequeuePackets( PacketQueueType i_eQueueType, PacketQueue& io_rcPacketContainer )
  {
    // Avoid the mutex lock if there is nothing in the queue
    if( g_cPacketQueueArray[i_eQueueType].size() > 0 )
    {
      std::lock_guard<std::mutex> cLockGuard( g_cMutexArray[i_eQueueType] );

      do
      {
        // Copy the packet to the provided queue
        io_rcPacketContainer.push( g_cPacketQueueArray[i_eQueueType].front() );
        g_cPacketQueueArray[i_eQueueType].pop();

      } while( g_cPacketQueueArray[i_eQueueType].size() > 0 );
    }
  }


  void Disconnect( SOCKET i_iSocket )
  {
    if( i_iSocket != INVALID_SOCKET )
    {
      ::shutdown( i_iSocket, SD_BOTH );
      closesocket( i_iSocket );
    }

    if( i_iSocket == g_iSocket )
    {
      g_iSocket = INVALID_SOCKET;
      g_eConnectionStatus = ConnectionStatus::Disconnected;
    }
  }


  void DownloadFiles()
  {
    if( !g_cDownloadQueue.empty() && g_cDownloadVector.size() < g_iNumConcurrentDownloads )
    {
      std::lock_guard<std::mutex> cLockGuard( g_cDownloadGuard );

      do
      {
        // Transfer the download info from the queue to the active download vector
        g_cDownloadVector.push_back( std::move( g_cDownloadQueue.front() ) );
        g_cDownloadQueue.pop();
      } while( !g_cDownloadQueue.empty() && g_cDownloadVector.size() < g_iNumConcurrentDownloads );
    }

    for( auto& rcDownloadInfo : g_cDownloadVector )
    {
      if( INVALID_SOCKET == rcDownloadInfo.m_iSocket )
      {
        if( rcDownloadInfo.m_strServerPath.empty() || rcDownloadInfo.m_strServerPath[0] != '/' )
        {
          rcDownloadInfo.m_strServerPath.insert( rcDownloadInfo.m_strServerPath.begin(), '/' );
        }

        // Create a connection for this download
        std::string strInfo{ "Downloading file: " };
        strInfo += rcDownloadInfo.m_strServer;
        strInfo += ":";
        strInfo += rumStringUtils::ToString( rcDownloadInfo.m_iPort );
        strInfo += rcDownloadInfo.m_strServerPath;
        Logger::LogStandard( strInfo );

        const SOCKET iSocket{ ConnectTCP( rcDownloadInfo.m_strServer, rcDownloadInfo.m_iPort ) };
        if( iSocket != INVALID_SOCKET )
        {
          rcDownloadInfo.m_iSocket = iSocket;

          // Make sure the underlying folder structure exists
          const std::filesystem::path fsClientPath( rcDownloadInfo.m_strClientPath );
          std::filesystem::create_directories( fsClientPath.parent_path() );

          std::ofstream* pcOutfile{ new std::ofstream };
          pcOutfile->open( fsClientPath.c_str(), std::ofstream::out | std::ofstream::binary );
          if( !pcOutfile->is_open() )
          {
            std::string strError{ "Failed to create or open: " };
            strError += fsClientPath.string();
            Logger::LogStandard( strError, Logger::LOG_ERROR );

            rcDownloadInfo.m_bFailed = true;
            continue;
          }

          std::string strConnect = "GET ";
          strConnect += rcDownloadInfo.m_strServerPath;
          strConnect += " HTTP/1.1\r\nHost: ";
          strConnect += rcDownloadInfo.m_strServer;
          strConnect += "\r\nConnection: close\r\nUser-Agent: RUM Client\r\n\r\n";

          int32_t eStatus{ send( iSocket, strConnect.c_str(), (int32_t)strConnect.size(), 0 ) };
          if( SOCKET_ERROR == eStatus )
          {
            std::string strError{ "Download file failed connection: " };
            strError += strConnect;
            Logger::LogStandard( strError, Logger::LOG_ERROR );

            rcDownloadInfo.m_bFailed = true;
          }

          rcDownloadInfo.m_pcOutfile = pcOutfile;
        }
        else
        {
          rcDownloadInfo.m_bFailed = true;
        }
      }
      else if( rcDownloadInfo.m_pcOutfile && rcDownloadInfo.m_pcOutfile->is_open() )
      {
        // Continue downloading
        const int32_t eStatus{ recv( rcDownloadInfo.m_iSocket, g_cBuffer, g_uiBufferSize, 0 ) };
        if( SOCKET_ERROR == eStatus )
        {
          Logger::LogStandard( "Download file network failure", Logger::LOG_ERROR );
          rcDownloadInfo.m_bFailed = true;
        }
        else if( 0 == rcDownloadInfo.m_uiDownloadedBytes )
        {
          // Handle first packet
          if( !strstr( g_cBuffer, "HTTP/1.1 200 OK" ) )
          {
            std::string strError{ "Download file request failed: " };
            char* pcOffset{ strstr( g_cBuffer, "\r\n" ) };
            if( nullptr == pcOffset )
            {
              *pcOffset = '\0';
            }
            strError += pcOffset;

            Logger::LogStandard( strError, Logger::LOG_ERROR );
            rcDownloadInfo.m_bFailed = true;
          }
        }

        if( !rcDownloadInfo.m_bFailed )
        {
          if( eStatus > 0 )
          {
            // Are we still in header?
            if( rcDownloadInfo.m_bInHeader )
            {
              char* pcOffset{ strstr( g_cBuffer, "Content-Length: " ) };
              if( pcOffset )
              {
                pcOffset += strlen( "Content-Length: " );

                uint64_t uiLength{ 0 };
                while( isdigit( *pcOffset ) )
                {
                  uiLength = uiLength * 10 + ( *pcOffset - '0' );
                  ++pcOffset;
                }

                rcDownloadInfo.m_uiExpectedBytes = uiLength;

                RUM_COUT_IFDEF( NETWORK_DEBUG, "  " << uiLength << " bytes\n" );
              }

              pcOffset = strstr( g_cBuffer, "\r\n\r\n" );
              if( pcOffset )
              {
                const int32_t iOffset{ (int32_t)( pcOffset + 4 - g_cBuffer ) };
                rcDownloadInfo.m_pcOutfile->write( pcOffset + 4, eStatus - iOffset );
                rcDownloadInfo.m_uiDownloadedBytes += eStatus - iOffset;
                rcDownloadInfo.m_bInHeader = false;
              }
            }
            else
            {
              rcDownloadInfo.m_pcOutfile->write( g_cBuffer, eStatus );
              rcDownloadInfo.m_uiDownloadedBytes += eStatus;
            }

#if NETWORK_DEBUG
            double fProgressPercent{ rcDownloadInfo.m_uiDownloadedBytes / (double)rcDownloadInfo.m_uiExpectedBytes };
            int32_t iProgressPercent{ static_cast<int32_t>( fProgressPercent * 100.0 ) };

            RUM_COUT( "  " << rcDownloadInfo.m_strClientPath << " " << iProgressPercent << "%\n" );
#endif // NETWORK_DEBUG
          }
          else
          {
            rumAssert( 0 == eStatus );

            // File download finished
            RUM_COUT_IFDEF( NETWORK_DEBUG,
                            "File downloaded: " << rcDownloadInfo.m_strClientPath <<
                            " Bytes: " << rcDownloadInfo.m_uiDownloadedBytes << '\n' );

            rcDownloadInfo.m_pcOutfile->close();
            rcDownloadInfo.m_bComplete = true;
          }
        }
      }

      if( rcDownloadInfo.m_bFailed )
      {
        if( rcDownloadInfo.m_pcOutfile != nullptr )
        {
          rcDownloadInfo.m_pcOutfile->close();
          SAFE_DELETE( rcDownloadInfo.m_pcOutfile );
        }

        closesocket( rcDownloadInfo.m_iSocket );
      }

      rumEvent::s_cFileDownloadedEvent.emit( rcDownloadInfo );
    }

    // Remove complete and failed files from the download vector
    g_cDownloadVector.erase( std::remove_if( g_cDownloadVector.begin(), g_cDownloadVector.end(),
                                             []( const auto& i_rcDownloadInfo )
                                             {
                                               // Return true if the files match
                                               return i_rcDownloadInfo.m_bFailed || i_rcDownloadInfo.m_bComplete;
                                             } ),
                                             g_cDownloadVector.end() );
  }


  void EnqueueFile( rumDownloadInfo&& io_rcDownloadInfo )
  {
    std::lock_guard<std::mutex> cLockGuard( g_cDownloadGuard );

    g_cDownloadQueue.push( std::move( io_rcDownloadInfo ) );
  }


  void EnqueueFiles( std::vector<rumDownloadInfo>& io_rcDownloadInfoVector )
  {
    {
      std::lock_guard<std::mutex> cLockGuard( g_cDownloadGuard );

      for( auto& rcDownloadInfo : io_rcDownloadInfoVector )
      {
        g_cDownloadQueue.push( std::move( rcDownloadInfo ) );
      }
    }

    io_rcDownloadInfoVector.clear();
  }


  void EnqueuePacket( PacketQueueType i_eQueueType, rumPacket* io_pcPacket )
  {
    rumAssertMsg( io_pcPacket != nullptr, "pPacket NULL" );
    if( io_pcPacket != nullptr )
    {
      std::lock_guard<std::mutex> cLockGuard( g_cMutexArray[i_eQueueType] );

      g_cPacketQueueArray[i_eQueueType].push( io_pcPacket );

      RUM_COUT_IFDEF( NETWORK_DEBUG,
                      "Enqueued global " << ( i_eQueueType == PacketQueueType::PACKET_QUEUE_RECV ? "in" : "out" ) <<
                      "packet, queue size: " << g_cPacketQueueArray[i_eQueueType].size() << '\n' );
    }
  }


  // Copy a local packet queue to the global packet queue
  void EnqueuePackets( PacketQueueType i_eQueueType, PacketQueue& io_rcPacketContainer )
  {
    // Avoid the mutex lock if there is nothing in the queue
    if( !io_rcPacketContainer.empty() )
    {
      std::lock_guard<std::mutex> cLockGuard( g_cMutexArray[i_eQueueType] );

      do
      {
        // Copy the packet to the provided queue
        g_cPacketQueueArray[i_eQueueType].push( io_rcPacketContainer.front() );
        io_rcPacketContainer.pop();

      } while( !io_rcPacketContainer.empty() );
    }
  }


  ConnectionStatus GetConnectionStatus()
  {
    return g_eConnectionStatus;
  }


  SOCKET GetNetworkSocket()
  {
    return g_iSocket;
  }


  SOCKET HostTCP( int32_t i_iPort )
  {
    RUM_COUT( "Using port: " << i_iPort << '\n' );

    // The port numbers are divided into three ranges: the Well Known Ports, the Registered Ports, and the Dynamic
    // and/or Private Ports. The Well Known Ports are those from 0 through 1023. DCCP Well Known ports SHOULD NOT be
    // used without IANA registration. The Registered Ports are those from 1024 through 49151. DCCP Registered ports
    // SHOULD NOT be used without IANA registration. The Dynamic and Private Ports are those from 49152 through 65535.

    g_iSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
    if( INVALID_SOCKET == g_iSocket )
    {
      Logger::LogStandard( "Error: Failed to create a TCPIP socket" );
      return INVALID_SOCKET;
    }

    sockaddr_in cAddrIn;

    cAddrIn.sin_family = AF_INET;
    cAddrIn.sin_addr.s_addr = htonl( INADDR_ANY );

    // Check for port correctness
    cAddrIn.sin_port = htons( i_iPort );
    /*if (addr.sin_port != port || port <= 0 || port > PRIVATE_PORT_MAX)
    {
    Logger::LogStandard("Error: The network port specified is invalid. "
    "Consider using a port between the range "
    QUOTED(PRIVATE_PORT_MIN) " and " QUOTED(PRIVATE_PORT_MAX));
    return INVALID_SOCKET;
    }*/

    constexpr int32_t PRIVATE_PORT_MIN{ 49152 };
    constexpr int32_t PRIVATE_PORT_MAX{ 65535 };

    // Check for valid port number
    if( i_iPort < IPPORT_RESERVED )
    {
      Logger::LogStandard( "Warning: You have provided a port that is considered a Well Known Port. "
                           "Consider using a port between the range "
                           QUOTED( PRIVATE_PORT_MIN ) " and " QUOTED( PRIVATE_PORT_MAX ) );
    }
    else if( i_iPort < PRIVATE_PORT_MIN )
    {
      Logger::LogStandard( "Warning: You have provided a port that is considered a Registered Port. "
                           "Consider using a port between the range "
                           QUOTED( PRIVATE_PORT_MIN ) " and " QUOTED( PRIVATE_PORT_MAX ) );
    }

    // This code will make the socket cleanly reuse the port in case of crash.
    // This applies to UNIX machines, maybe not so much Windows.
#ifndef WIN32
    int32_t iOptVal;
    if( setsockopt( g_iSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&iOptVal, sizeof( int32_t ) ) == SOCKET_ERROR )
    {
      Logger::LogStandard( "Warning: Setting the socket option SO_REUSEADDR failed" );
    }
#endif

    // Bind socket - use the global namespace here since tr1 now includes a std::bind
    if( ::bind( g_iSocket, (PSOCKADDR)&cAddrIn, sizeof( sockaddr_in ) ) == SOCKET_ERROR )
    {
      Logger::LogStandard( "Error: Unable to bind a network socket" );
      return INVALID_SOCKET;
    }

    // Begin listening for connections
    if( listen( g_iSocket, SOMAXCONN ) == SOCKET_ERROR )
    {
      Logger::LogStandard( "Error: Failed to create a listening socket for connecting clients" );
      return  INVALID_SOCKET;
    }

    Logger::SetOutputColor( COLOR_SERVER );
    RUM_COUT( "Listening for connecting clients\n" );
    Logger::SetOutputColor( COLOR_STANDARD );

    return g_iSocket;
  }


  int32_t Init( uint32_t i_uiInboundPacketPoolSize, uint32_t i_uiOutboundPacketPoolSize )
  {
    int32_t eResult{ RESULT_SUCCESS };

#ifdef WIN32
    constexpr int32_t iMajor{ LOBYTE( WINSOCK_VERSION ) };
    constexpr int32_t iMinor{ HIBYTE( WINSOCK_VERSION ) };

    WSADATA cWSAData;

    Logger::SetOutputColor( COLOR_SERVER );
    std::string strInfo{ "Initializing Winsock version " };
    strInfo += rumStringUtils::ToString( iMajor );
    strInfo += ".";
    strInfo += rumStringUtils::ToString( iMinor );
    Logger::LogStandard( strInfo );
    Logger::SetOutputColor( COLOR_STANDARD );

    // Start winsock
    if( WSAStartup( WINSOCK_VERSION, &cWSAData ) != 0 )
    {
      std::string strError{ "Error: WinSock version " };
      strError += rumStringUtils::ToString( iMajor );
      strError += ".";
      strError += rumStringUtils::ToString( iMinor );
      strError += " is required";
      Logger::LogStandard( strError );

      eResult = RESULT_FAILED;
    }
#endif // WIN32

    rumInboundPacketPool::Init( i_uiInboundPacketPoolSize );
    rumOutboundPacketPool::Init( i_uiOutboundPacketPoolSize );

    return eResult;
  }


  void ScriptBind()
  {
    HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };
    Sqrat::RootTable( pcVM ).Func( "rumGetSocket", GetNetworkSocket );
  }


  int32_t Shutdown()
  {
#pragma message("TODO - clear the queue")
    // #TODO - instead of clearing the queue like this (it's not safe), just allow all packets to finish processing???
    //ClearQueue();

    rumInboundPacketPool::Shutdown();
    rumOutboundPacketPool::Shutdown();

    int32_t eResult{ RESULT_SUCCESS };

#ifdef WIN32
    // Terminate winsock
    eResult = WSACleanup();
#endif

    return eResult;
  }


  int32_t UpdatePackets( const ConnectionHash& io_rcConnectionHash )
  {
    // The temporary PacketQueue is used here to prevent a mutex lock from occurring for each connection. All of the
    // sent and received packets are enqueued and dequeued with only one call.
    PacketQueue cPacketQueue;

    // Dequeue all pending outgoing packets (causes a mutex lock)
    DequeuePackets( PACKET_QUEUE_SEND, cPacketQueue );

#if NETWORK_DEBUG
    if( !cPacketQueue.empty() )
    {
      std::lock_guard<std::mutex> cLockGuard( g_cMutexArray[PACKET_QUEUE_SEND] );
      RUM_COUT( "Sending " << cPacketQueue.size() << " network packet(s)\n" );
    }
#endif // NETWORK_DEBUG

    // Copy packets to the destination client connection
    while( !cPacketQueue.empty() )
    {
      auto pcPacket{ cPacketQueue.front() };
      cPacketQueue.pop();

      rumAssertMsg( pcPacket, "pPacket NULL" );
      if( pcPacket )
      {
        rumOutboundPacket* pcOutboundPacket{ (rumOutboundPacket*)pcPacket };
        rumAssert( pcOutboundPacket );

        // Find the connection associated with the socket
        for( const auto& iSocket : pcOutboundPacket->GetRecipientSet() )
        {
          const ConnectionHash::const_iterator iter{ io_rcConnectionHash.find( iSocket ) };
          if( iter != io_rcConnectionHash.end() )
          {
            // Add the global packet to the connection's local send queue
            iter->second->EnqueueLocalPacket( PACKET_QUEUE_SEND, pcOutboundPacket );
          }
          else
          {
            // Recipient not found, so just decrement the ref count
            pcOutboundPacket->PopSendRef();
          }
        }
      }
    }

    // We are going to reuse the packet container, it had better be empty because we'll end up "receiving" a message
    // intended to be sent
    rumAssertMsg( cPacketQueue.empty(), "Packet queue was not empty!" );

    // Copy all received packets to the the global receive queue
    for( const auto& iter : io_rcConnectionHash )
    {
      while( iter.second->HasQueuedPackets( PACKET_QUEUE_RECV ) )
      {
        // Add the packet to the temporary queue
        auto pcPacket{ iter.second->DequeueLocalPacket( PACKET_QUEUE_RECV ) };
        cPacketQueue.push( pcPacket );
      }
    }

    // Add all of the packets to the global receive queue. Since this causes a mutex lock, we do this all in one go.
    EnqueuePackets( PACKET_QUEUE_RECV, cPacketQueue );

    return RESULT_SUCCESS;
  }
} // namespace network
