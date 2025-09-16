#pragma once

#include <queue>
#include <string>
#include <unordered_map>

class ofstream;

#ifdef WIN32
#include <Ws2tcpip.h>
#endif // WIN32

/* --------------------------------------------------------------------------------------------------------------------
For the most part, networking runs on a separate thread via each application's cNetworkThread. during each frame,
the network thread handles these three major functions:
1. rumNetwork::UpdatePackets() called on the complete connection hash (one entry per player)
   - Sends handled
   - Receives handled
     - For each connection, each queue is emptied and DequeueLocalPacket() is called for each entry
       - Each packet pulled from the local queue is added to a temp cPacketQueue
     - All packets in the temp cPacketQueue are copied to the global receive queue g_cPacketQueueArray
2. rumConnection::ReceivePackets() called for each connection
   - Reads bytes into the connection's recv buffer
   - When a full packet is received, heap allocate the rumInboundPacket
   - Enqueues the packet locally
3. rumConnection::SendPackets() called for each connection

Note that outbound packets are held in a packet pool so that heap allocations do not have to be made per frame. These
are able to work without mutex locks because check-ins and check-outs only occur on the application thread.
1. Application checks out a packet
2. Payload is populated
3. Recipients are either pre-populated, or a single recipient is specified on Send
4. When Send is called, a ref count is set for each recipient
5. Refs are decremented when the packet is successfully sent to the recipeint, or if the recipient can't be found
6. The application thread calls Update() on the outbound packet pool and packets are checked in if they are no longer
   sending (the ref count has returned to zero)
---------------------------------------------------------------------------------------------------------------------*/

namespace rumNetwork
{
  class rumConnection;
  class rumPacket;

  using ConnectionHash = std::unordered_map<SOCKET, rumConnection*>;
  using PacketQueue = std::queue<rumPacket*>;

  enum class ConnectionStatus
  {
    Disconnected,
    Connecting,
    Connected,
    Failed
  };

  enum PacketQueueType
  {
    PACKET_QUEUE_RECV,
    PACKET_QUEUE_SEND,
    NUM_PACKET_QUEUES
  };

  enum FDType
  {
    FD_SET_READ,
    FD_SET_WRITE,
    FD_SET_EXCEPT,
    NUM_FD_SETS
  };

  // Info about a currently downloading file
  struct rumDownloadInfo
  {
    SOCKET m_iSocket{ INVALID_SOCKET };

    std::ofstream* m_pcOutfile{ nullptr };

    uint64_t m_uiExpectedBytes{ 0UL };
    uint64_t m_uiDownloadedBytes{ 0UL };

    std::string m_strClientPath;
    std::string m_strServerPath;

    std::string m_strServer;
    int32_t m_iPort{ 0 };

    // False until fully downloaded
    bool m_bComplete{ false };

    // True until the download header has been fully received
    bool m_bInHeader{ true };

    // True if an error has occurred during download
    bool m_bFailed{ false };
  };

  using DownloadQueue = std::queue<rumDownloadInfo>;
  using DownloadVector = std::vector<rumDownloadInfo>;

  void ClearDownloadQueue();
  void ClearPacketQueue();

  SOCKET Connect( const std::string& i_strAddress, int32_t i_iPort );

  // Move packets from the global queue into individual connection queues
  void DequeuePackets( PacketQueueType i_eQueueType, PacketQueue& io_rcPacketQueue);

  void Disconnect( SOCKET i_iSocket );

  void DownloadFiles();

  // File downloads
  void EnqueueFile( rumDownloadInfo&& io_rcDownloadInfo );
  void EnqueueFiles( std::vector<rumDownloadInfo>& io_rcDownloadInfoVector );

  // Put an individual packet in the global queue
  void EnqueuePacket( PacketQueueType i_eQueueType, rumPacket* io_pcPacket );

  ConnectionStatus GetConnectionStatus();

  // The main connection socket (for either connecting or hosting)
  SOCKET GetNetworkSocket();

  SOCKET HostTCP( int32_t i_iPort );

  int32_t Init( uint32_t i_uiInboundPacketPoolSize, uint32_t i_uiOutboundPacketPoolSize );
  void ScriptBind();
  int32_t Shutdown();

  void OnConnectionFailed();

  // Copy packets waiting for send to the appropriate network connection send queue
  int32_t UpdatePackets( const ConnectionHash& io_rcConnectionHash );
} // namespace network
