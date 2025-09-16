#include <network/u_connection.h>

#include <network/u_network.h>
#include <network/u_packet.h>
#include <u_log.h>
#include <u_rum.h>
#include <u_utility.h>

//#undef NETWORK_DEBUG
//#define NETWORK_DEBUG 1


namespace rumNetwork
{
  rumConnection::~rumConnection()
  {
    for( int32_t i = 0; i < NUM_PACKET_QUEUES; ++i )
    {
      while( m_cPacketQueue[i].size() > 0 )
      {
        m_cPacketQueue[i].pop();
      }
    }

    if( m_pcRecvBuffer )
    {
      delete[] m_pcRecvBuffer;
      m_pcRecvBuffer = nullptr;
    }

    if( m_pcSendBuffer )
    {
      delete[] m_pcSendBuffer;
      m_pcSendBuffer = nullptr;
    }

    /*if (m_iSocket != INVALID_SOCKET)
    {
        closeSocket();
        m_iSocket = INVALID_SOCKET;
    }*/
  }


  int32_t rumConnection::CloseSocket()
  {
    // Make sure the connection's queue is empty
    rumAssert( this->m_cPacketQueue->empty() );

    if( m_iSocket != INVALID_SOCKET )
    {
#pragma message("TODO- This is not okay")
      /* #TODO - CheckoutPacket can only be called by the application thread, not the network threadand CloseSocket is called by the NetworkThread :
      // Manually construct a termination packet
      auto& pcPacket{ rumOutboundPacketPool::CheckoutPacket() };
      pcPacket.SetHeader( PACKET_HEADER_CONNECTION_TERMINATED );
      pcPacket.AddRecipient( m_iSocket );

      // Add to global recv queue
      EnqueuePacket( PACKET_QUEUE_RECV, &pcPacket );*/

      // Terminate the socket
      rumNetwork::Disconnect( m_iSocket );
      m_iSocket = INVALID_SOCKET;
    }

    return RESULT_SUCCESS;
  }


  // Pop a packet off of the client's local packet queue
  rumPacket* rumConnection::DequeueLocalPacket( PacketQueueType i_eQueueType )
  {
    rumPacket* pcPacket{ nullptr };

    if( m_cPacketQueue[i_eQueueType].size() > 0 )
    {
      pcPacket = m_cPacketQueue[i_eQueueType].front();
      m_cPacketQueue[i_eQueueType].pop();
    }

    return pcPacket;
  }


  void rumConnection::EnqueueLocalPacket( PacketQueueType i_eQueueType, rumPacket* i_pcPacket )
  {
    m_cPacketQueue[i_eQueueType].push( i_pcPacket );
  }


  int32_t rumConnection::ReceivePackets()
  {
    const char* pcRecvBuffer{ m_pcRecvBuffer };

    // See if the buffer is incapable of receiving data
    if( m_uiRecvBufferCapacity - m_uiRecvBufferUsed <= 0 )
    {
      // Determine if the packet is just too large to fit into our buffer
      PACKET_EMBEDDED_SIZE_TYPE uiPacketSize{ 0 };
      rumSerializationUtils::Read( uiPacketSize, pcRecvBuffer );
      if( uiPacketSize > m_uiRecvBufferCapacity )
      {
        // Our send buffer has to grow, double its size
        const size_t uiNewSize{ m_uiRecvBufferCapacity * 2 };

        RUM_COUT_IFDEF( NETWORK_DEBUG,
                        "Network recv buffer growing from " << m_uiRecvBufferCapacity << " bytes to " << uiNewSize <<
                        " bytes\n" );

        char* pcBuffer{ new char[uiNewSize] };

        // Copy the current buffer to the new one
        memcpy( pcBuffer, m_pcRecvBuffer, sizeof( char ) * m_uiRecvBufferUsed );

        delete[] m_pcRecvBuffer;
        m_pcRecvBuffer = pcBuffer;

        m_uiRecvBufferCapacity = (uint32_t)uiNewSize;
        pcRecvBuffer = m_pcRecvBuffer;
      }
    }

    // Perform the actual winsock receive
    const int32_t iBytesReceived{ recv( m_iSocket, m_pcRecvBuffer + m_uiRecvBufferUsed,
                                  m_uiRecvBufferCapacity - m_uiRecvBufferUsed, 0 ) };

    if( ( 0 == iBytesReceived ) || ( SOCKET_ERROR == iBytesReceived ) )
    {
      // Let the program handle the error result
      return SOCKET_ERROR;
    }

    // We read some bytes, so advance the buffer size counter
    m_uiRecvBufferUsed += iBytesReceived;

    bool bDone{ false };
    static PACKET_HEADER_TYPE scLastHeader{ 0 };
    static PACKET_EMBEDDED_SIZE_TYPE suiLastPacketSize{ 0 };

    do
    {
      // Process all received packets

      // Determine the packet size
      PACKET_EMBEDDED_SIZE_TYPE uiPacketSize{ 0 };
      rumSerializationUtils::Read( uiPacketSize, pcRecvBuffer );
      if( uiPacketSize == 0 )
      {
        Logger::LogStandard( "Packet of size 0 encountered", Logger::LOG_WARNING );
        rumAssert( false );
        continue;
      }

      // Make sure the header is valid
      PACKET_HEADER_TYPE iHeader{ 0 };
      rumSerializationUtils::Read( iHeader, pcRecvBuffer + rumPacket::s_uiEmbeddedSize );
      if( iHeader >= NUM_PACKET_HEADERS )
      {
        std::string strError{ "Invalid packet header type parsed: " };
        strError += rumStringUtils::ToString( iHeader );
        strError += ". Last packet type parsed: ";
        strError += rumStringUtils::ToString( scLastHeader );
        Logger::LogStandard( strError, Logger::LOG_ERROR );
        rumAssert( false );
        continue;
      }

      // See if the buffer has a complete packet
      if( m_uiRecvBufferUsed >= uiPacketSize )
      {
        // Create a new packet from network transmission
        // #TODO - shared pool for inbound packets
        rumInboundPacket* pcPacket{ new rumInboundPacket( pcRecvBuffer, uiPacketSize, m_iSocket ) };
        {
          RUM_COUT_IFDEF( NETWORK_DEBUG, "Received packet type " << (int32_t)iHeader << '\n' );

          // Add to local recv queue
          EnqueueLocalPacket( PACKET_QUEUE_RECV, pcPacket );

          m_uiRecvBufferUsed -= uiPacketSize;
          pcRecvBuffer += uiPacketSize;
        }
      }
      else
      {
        // The information is not a complete packet, stop processing until we receive more data
        bDone = true;
      }

      scLastHeader = iHeader;
      suiLastPacketSize = uiPacketSize;

    } while( !bDone && m_uiRecvBufferUsed != 0 );

    // Extra information is present, move the memory to the front of the buffer
    if( m_uiRecvBufferUsed > 0 )
    {
      memmove( m_pcRecvBuffer, pcRecvBuffer, m_uiRecvBufferUsed );
      pcRecvBuffer = m_pcRecvBuffer;
    }

    return RESULT_SUCCESS;
  }


  int32_t rumConnection::SendPackets()
  {
    int32_t eResult{ RESULT_SUCCESS };

    rumPacket* pcPacket{ nullptr };
    bool bBufferFull{ false };

    // Empty the local packet queue - copy the packet buffers into the connection's send buffer
    while( m_cPacketQueue[PACKET_QUEUE_SEND].size() > 0 && !bBufferFull )
    {
      pcPacket = m_cPacketQueue[PACKET_QUEUE_SEND].front();
      rumAssertMsg( pcPacket, "pPacket NULL" );
      if( pcPacket )
      {
        // See if our buffer is large enough for the packet
        if( pcPacket->GetBufferSize() > m_uiSendBufferCapacity )
        {
          // Our send buffer has to grow, so increase it as needed
          const size_t uiNewSize{ pcPacket->GetBufferSize() };

          RUM_COUT_IFDEF( NETWORK_DEBUG, "Network send buffer is growing from " << m_uiSendBufferCapacity <<
                          " bytes to " << uiNewSize << " bytes\n" );

          char* pcBuffer{ new char[uiNewSize] };

          // Copy the contents of the current buffer to the new one
          memcpy( pcBuffer, m_pcSendBuffer, sizeof( char ) * m_uiSendBufferUsed );

          // Free the existing buffer
          delete[] m_pcSendBuffer;

          // Point to the new buffer
          m_pcSendBuffer = pcBuffer;

          m_uiSendBufferCapacity = (uint32_t)uiNewSize;
        }

        // Is there enough space in the send buffer for this packet?
        if( pcPacket->GetBufferSize() + m_uiSendBufferUsed < m_uiSendBufferCapacity )
        {
          // Copy the packet to the send buffer
          memcpy( m_pcSendBuffer + m_uiSendBufferUsed, pcPacket->GetBuffer(),
                  sizeof( char ) * pcPacket->GetBufferSize() );

          // Adjust the send buffer's size
          m_uiSendBufferUsed += pcPacket->GetBufferSize();

          // Pop the packet and free its memory
          m_cPacketQueue[PACKET_QUEUE_SEND].pop();

          rumOutboundPacket* pcOutboundPacket{ (rumOutboundPacket*)pcPacket };
          rumAssert( pcOutboundPacket );

          pcOutboundPacket->PopSendRef();
        }
        else
        {
          // Stop adding information to the send buffer
          bBufferFull = true;
        }
      }
    }

    if( m_uiSendBufferUsed > 0 )
    {
      // Perform the actual winsock send
      const int32_t iBytesSent{ send( m_iSocket, m_pcSendBuffer, m_uiSendBufferUsed, 0 ) };
      if( ( 0 == iBytesSent ) || ( SOCKET_ERROR == iBytesSent ) )
      {
        // Let the program handle the error result
        return SOCKET_ERROR;
      }

      if( iBytesSent == m_uiSendBufferUsed )
      {
        // Everything got sent, so take a shortcut on clearing buffer
        m_uiSendBufferUsed = 0;
      }
      else
      {
        // We sent part of the buffer's data, remove that data from the buffer
        m_uiSendBufferUsed -= iBytesSent;
        memmove( m_pcSendBuffer, m_pcSendBuffer + iBytesSent, m_uiSendBufferUsed );
      }
    }

    return eResult;
  }
} // namespace network
