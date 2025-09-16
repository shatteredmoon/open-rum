#include <network/u_packet.h>

#include <network/u_network.h>
#include <u_log.h>
#include <u_rum.h>
#include <u_utility.h>

//#undef NETWORK_DEBUG
//#define NETWORK_DEBUG 1


namespace rumNetwork
{
  std::queue<rumInboundPacket*> rumInboundPacketPool::s_cAvailablePacketPool;
  std::vector<rumInboundPacket*> rumInboundPacketPool::s_cActivePacketVector;

  std::queue<rumOutboundPacket*> rumOutboundPacketPool::s_cAvailablePacketPool;
  std::vector<rumOutboundPacket*> rumOutboundPacketPool::s_cActivePacketVector;


  rumInboundPacket::rumInboundPacket( uint32_t i_uiSize )
    : m_cResourceLoader( i_uiSize )
  {
    // Skip the embedded size area
    m_cResourceLoader.SeekPos( s_uiEmbeddedSize + s_uiHeaderSize );
  }


  rumInboundPacket::rumInboundPacket( const char* i_pcBuffer, uint32_t i_uiSize, SOCKET i_iSocket )
    : m_iSocket( i_iSocket )
  {
    rumAssertMsg( i_pcBuffer && i_uiSize > 0, "i_pcBuffer NULL" );
    if( i_pcBuffer )
    {
      // Grab the packet info
      m_cResourceLoader.LoadMemoryFile( i_pcBuffer, i_uiSize );
      m_cResourceLoader.SeekPos( s_uiEmbeddedSize + s_uiHeaderSize );
    }
  }


  // override
  void rumInboundPacket::Reset()
  {
    m_cResourceLoader.SeekPos( s_uiEmbeddedSize + s_uiHeaderSize );
  }


  Sqrat::Object rumInboundPacket::ScriptRead() const
  {
    Sqrat::Object sqObject;
    rumDWord eVarType{ 0 };

    // Determine the type of the script object we're about to serialize
    m_cResourceLoader.Serialize( eVarType );

    // Provide a default value
    switch( eVarType )
    {
      case OT_ARRAY:    sqObject = m_cResourceLoader.ScriptSerializeArray( sqObject );    break;
      case OT_BOOL:     sqObject = m_cResourceLoader.ScriptSerializeBool( sqObject );     break;
      case OT_CLASS:    sqObject = m_cResourceLoader.ScriptSerializeClass( sqObject );    break;
      case OT_FLOAT:    sqObject = m_cResourceLoader.ScriptSerializeFloat( sqObject );    break;
      case OT_INSTANCE: sqObject = m_cResourceLoader.ScriptSerializeInstance( sqObject ); break;
      case OT_INTEGER:  sqObject = m_cResourceLoader.ScriptSerializeInt( sqObject );      break;
      case OT_STRING:   sqObject = m_cResourceLoader.ScriptSerializeString( sqObject );   break;
      case OT_NULL:     sqObject = m_cResourceLoader.ScriptSerializeNull( sqObject );     break;
      default:
        rumAssertArgs( false, "Unexpected script type", eVarType );
        break;
    }

    return sqObject;
  }


  // Network thread
  void rumInboundPacketPool::CheckinPacket( rumInboundPacket* io_pcPacket )
  {
    rumAssert( io_pcPacket );

    io_pcPacket->Reset();

    const size_t iCount{ s_cAvailablePacketPool.size() };
    s_cAvailablePacketPool.push( io_pcPacket );

    RUM_COUT_IFDEF( NETWORK_DEBUG,
                    "Checking in packet type " << (int32_t)io_pcPacket->GetHeaderType() << ", pool size : " <<
                    s_cAvailablePacketPool.size() << '\n' );

    rumAssertArgs( s_cAvailablePacketPool.size() > iCount, "Failed to checkin packet with header ",
                   rumStringUtils::ToString( (int32_t)io_pcPacket->GetHeaderType() ) );
  }


  // Network thread
  rumInboundPacket& rumInboundPacketPool::CheckoutPacket()
  {
    rumInboundPacket* pcPacket{ nullptr };

    if( s_cAvailablePacketPool.empty() )
    {
      // Nothing available, so create a new packet
      RUM_COUT_IFDEF( NETWORK_DEBUG, "Packet Pool empty, creating new packet\n" );
      pcPacket = new rumInboundPacket;
    }
    else
    {
      RUM_COUT_IFDEF( NETWORK_DEBUG, "Checking out packet, pool size: " << s_cAvailablePacketPool.size() << '\n' );

      pcPacket = s_cAvailablePacketPool.front();
      s_cAvailablePacketPool.pop();
    }

    rumAssert( pcPacket != nullptr );
    s_cActivePacketVector.push_back( pcPacket );

    return *pcPacket;
  }


  // static
  void rumInboundPacketPool::Init( uint32_t i_uiNumPackets )
  {
    for( uint32_t i = 0; i < i_uiNumPackets; ++i )
    {
      rumInboundPacket* pcPacket{ new rumInboundPacket };
      s_cAvailablePacketPool.push( pcPacket );
    }

    s_cActivePacketVector.reserve( i_uiNumPackets );
  }


  // static
  void rumInboundPacketPool::Shutdown()
  {
    while( !s_cAvailablePacketPool.empty() )
    {
      // Free all available packets
      auto pcPacket{ s_cAvailablePacketPool.front() };
      s_cAvailablePacketPool.pop();
      delete pcPacket;
    }

    // Free active packets
    for( auto iter : s_cActivePacketVector )
    {
      delete iter;
    }

    s_cActivePacketVector.clear();
  }


  // static
  void rumInboundPacketPool::Update()
  {
    // If a packet is no longer sending (its refcount has fallen to zero), then move the packet back to the available
    // pool and erase its entry in the active vector
    //s_cActivePacketVector.erase( std::remove_if( s_cActivePacketVector.begin(), s_cActivePacketVector.end(),
    //                                             []( rumInboundPacket* i_pcPacket )
    //                                             {
    //                                               if( !i_pcPacket->IsSending() )
    //                                               {
    //                                                 CheckinPacket( i_pcPacket );
    //                                                 return true;
    //                                               }
    //
    //                                               return false;
    //                                             } ), s_cActivePacketVector.end() );
  }


  rumOutboundPacket::rumOutboundPacket( uint32_t i_uiSize /* = BUFFER_SIZE */ )
    : m_cResourceSaver( i_uiSize )
  {
    // Skip the embedded size area
    m_cResourceSaver.SeekPos( s_uiEmbeddedSize + s_uiHeaderSize );
  }


  rumOutboundPacket::rumOutboundPacket( PacketHeaderType i_eHeader, uint32_t i_uiSize /* = BUFFER_SIZE */ )
    : m_cResourceSaver( i_uiSize )
  {
    SetHeader( i_eHeader );

    // Skip the embedded size area
    m_cResourceSaver.SeekPos( s_uiEmbeddedSize + s_uiHeaderSize );
  }


  void rumOutboundPacket::Reset()
  {
    m_cResourceSaver.SeekPos( s_uiEmbeddedSize + s_uiHeaderSize );
    m_iRecipients.clear();
    //m_iIgnoredRecipients.clear();
    m_iSendRefs = 0;
    m_bHeaderWritten = false;
    m_bSizeWritten = false;
  }


  // Application thread
  int32_t rumOutboundPacket::Send( SOCKET i_iSocket )
  {
    int32_t eResult{ RESULT_FAILED };

    // If a valid socket was provided, add it to the recipient list
    if( i_iSocket != INVALID_SOCKET )
    {
      m_iRecipients.insert( i_iSocket );
    }

    m_iSendRefs = m_iRecipients.size();

    if( IsSendable() )
    {
      if( !m_bSizeWritten )
      {
        WriteSize();
      }

      // Add to program's send queue
      EnqueuePacket( PACKET_QUEUE_SEND, this );

      RUM_COUT_IFDEF( NETWORK_DEBUG, "Queued packet type: " << (int32_t)GetHeaderType() << '\n' );

      eResult = RESULT_SUCCESS;
    }
    else
    {
      rumAssertMsg( IsSendable(), "rumOutboundPacket not sendable!" );
      std::string strError{ "Encountered unsendable rumOutboundPacket: Header type " };
      strError += rumStringUtils::ToString( GetHeaderType() );
      strError += ", recipients: ";
      strError += rumStringUtils::ToString( (int32_t)m_iSendRefs );
      Logger::LogStandard( strError );
    }

    return eResult;
  }


  rumOutboundPacket& rumOutboundPacket::SetHeader( PacketHeaderType i_eHeader )
  {
    const bool bValid{ i_eHeader >= 0 && i_eHeader < NUM_PACKET_HEADERS };

    rumAssertArgs( bValid, "Unexpected packet header type ", i_eHeader );

    if( bValid )
    {
      // Write header
      PACKET_HEADER_TYPE eHeader{ static_cast<PACKET_HEADER_TYPE>( i_eHeader ) };
      const size_t uiSavedPos{ m_cResourceSaver.GetPos() };
      m_cResourceSaver.SeekPos( s_uiEmbeddedSize );
      m_cResourceSaver.Serialize( eHeader );
      m_cResourceSaver.SeekPos( uiSavedPos );

      // Packet is now sendable since it contains a header
      m_bHeaderWritten = true;

      RUM_COUT_IFDEF( NETWORK_DEBUG, "Outbound packet header set to type " << i_eHeader << '\n' );
    }

    return *this;
  }

  rumOutboundPacket& rumOutboundPacket::Write( Sqrat::Object i_sqVar )
  {
    // We must write out a type id so that the deserializer knows which type of script object to deserialize
    m_cResourceSaver.Serialize( (rumDWord)i_sqVar.GetType() );
    m_cResourceSaver.ScriptSerialize( i_sqVar );
    return *this;
  }


  rumOutboundPacket& rumOutboundPacket::WriteSize()
  {
    const PACKET_EMBEDDED_SIZE_TYPE uiSize{ (rumWord)GetBufferSize() };
    const size_t uiSavedPos{ m_cResourceSaver.GetPos() };
    m_cResourceSaver.SeekPos( 0 );
    m_cResourceSaver.Serialize( uiSize );
    m_cResourceSaver.SeekPos( uiSavedPos );

    m_bSizeWritten = true;

    return *this;
  }


  // Application thread
  void rumOutboundPacketPool::CheckinPacket( rumOutboundPacket* io_pcPacket )
  {
    rumAssert( io_pcPacket );

    io_pcPacket->Reset();

    const size_t iCount{ s_cAvailablePacketPool.size() };
    s_cAvailablePacketPool.push( io_pcPacket );

    RUM_COUT_IFDEF( NETWORK_DEBUG,
                    "Checking in packet type " << (int32_t)io_pcPacket->GetHeaderType() << ", pool size : " <<
                    s_cAvailablePacketPool.size() << '\n' );

    rumAssertArgs( s_cAvailablePacketPool.size() > iCount, "Failed to checkin packet with header ",
                   rumStringUtils::ToString( (int32_t)io_pcPacket->GetHeaderType() ) );
  }


  // Application thread
  rumOutboundPacket& rumOutboundPacketPool::CheckoutPacket()
  {
    rumOutboundPacket* pcPacket{ nullptr };

    if( s_cAvailablePacketPool.empty() )
    {
      // Nothing available, so create a new packet
      RUM_COUT_IFDEF( NETWORK_DEBUG, "Packet Pool empty, creating new packet\n" );
      pcPacket = new rumOutboundPacket;
    }
    else
    {
      RUM_COUT_IFDEF( NETWORK_DEBUG, "Checking out packet, pool size: " << s_cAvailablePacketPool.size() << '\n' );

      pcPacket = s_cAvailablePacketPool.front();
      s_cAvailablePacketPool.pop();
    }

    rumAssert( pcPacket != nullptr );
    s_cActivePacketVector.push_back( pcPacket );

    return *pcPacket;
  }


  // static
  void rumOutboundPacketPool::Init( uint32_t i_uiNumPackets )
  {
    for( uint32_t i = 0; i < i_uiNumPackets; ++i )
    {
      rumOutboundPacket* pcPacket{ new rumOutboundPacket };
      s_cAvailablePacketPool.push( pcPacket );
    }

    s_cActivePacketVector.reserve( i_uiNumPackets );
  }


  // static
  void rumOutboundPacketPool::Shutdown()
  {
    while( !s_cAvailablePacketPool.empty() )
    {
      // Free all available packets
      auto pcPacket{ s_cAvailablePacketPool.front() };
      s_cAvailablePacketPool.pop();
      delete pcPacket;
    }

    // Free active packets
    for( auto iter : s_cActivePacketVector )
    {
      delete iter;
    }
    
    s_cActivePacketVector.clear();
  }


  // static
  void rumOutboundPacketPool::Update()
  {
    // If a packet is no longer sending (its refcount has fallen to zero), then move the packet back to the available
    // pool and erase its entry in the active vector
    s_cActivePacketVector.erase( std::remove_if( s_cActivePacketVector.begin(), s_cActivePacketVector.end(),
                                                 []( rumOutboundPacket* i_pcPacket )
                                                 {
                                                   if( !i_pcPacket->IsSending() )
                                                   {
                                                     CheckinPacket( i_pcPacket );
                                                     return true;
                                                   }

                                                   return false;
                                                 } ), s_cActivePacketVector.end() );
  }
} // namespace network
