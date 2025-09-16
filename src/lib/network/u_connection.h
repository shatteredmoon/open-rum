#pragma once

#include <network/u_packet.h>


namespace rumNetwork
{
  class rumConnection
  {
  public:

    rumConnection( SOCKET i_iSocket ) : m_iSocket( i_iSocket ) {}
    ~rumConnection();

    bool CanReceivePackets() const
    {
      return m_uiRecvBufferUsed < m_uiRecvBufferCapacity;
    }

    int32_t CloseSocket();
    SOCKET GetSocket() const
    {
      return m_iSocket;
    }

    rumPacket* DequeueLocalPacket( PacketQueueType i_eQueueType );
    void EnqueueLocalPacket( PacketQueueType i_eQueueType, rumPacket* io_pcPacket );

    bool HasQueuedPackets( PacketQueueType i_eQueueType ) const
    {
      return !m_cPacketQueue[i_eQueueType].empty();
    }

    // Data came in on a client socket, so read it into the buffer.
    // Returns false on failure, or when the client closes its half of the connection.
    // (WSAEWOULDBLOCK doesn't count as a failure.)
    int32_t ReceivePackets();

    int32_t SendPackets();

  private:

    // #TODO Ideally this would be much smaller, like just 1KB instead of 1MB
    static constexpr uint32_t s_uiInitialBufferSize{ 1024 * 1024 };

    // default construction is not allowed
    rumConnection();

    SOCKET m_iSocket{ INVALID_SOCKET };

    uint32_t m_uiRecvBufferUsed{ 0 };
    uint32_t m_uiSendBufferUsed{ 0 };

    uint32_t m_uiRecvBufferCapacity{ s_uiInitialBufferSize };
    uint32_t m_uiSendBufferCapacity{ s_uiInitialBufferSize };

    char* m_pcRecvBuffer{ new char[s_uiInitialBufferSize] };
    char* m_pcSendBuffer{ new char[s_uiInitialBufferSize] };

    PacketQueue m_cPacketQueue[PacketQueueType::NUM_PACKET_QUEUES];
  };
} // namespace network
