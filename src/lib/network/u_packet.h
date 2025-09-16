#pragma once

#include <network/u_network.h>
#include <u_assert.h>
#include <u_resource.h>

#include <queue>
#include <set>

#define PACKET_HEADER_TYPE rumByte
#define PACKET_EMBEDDED_SIZE_TYPE rumWord


namespace rumNetwork
{
  enum PacketHeaderType
  {
    // Generally, the packet is named based on its source
    PACKET_HEADER_CONNECTION_TERMINATED, // 0
    PACKET_HEADER_CLIENT_REQ_SERVER_INFO,
    PACKET_HEADER_SERVER_SERVER_INFO,
    PACKET_HEADER_CLIENT_ACCOUNT_CREATE,
    PACKET_HEADER_SERVER_ACCOUNT_CREATE_RESULT,
    PACKET_HEADER_CLIENT_ACCOUNT_LOGIN,
    PACKET_HEADER_SERVER_ACCOUNT_LOGIN_RESULT,
    PACKET_HEADER_CLIENT_ACCOUNT_LOGOUT,
    PACKET_HEADER_CLIENT_PLAYER_CREATE,
    PACKET_HEADER_SERVER_PLAYER_CREATE_RESULT,
    PACKET_HEADER_CLIENT_PLAYER_LOGIN, // 10
    PACKET_HEADER_SERVER_PLAYER_LOGIN_RESULT,
    PACKET_HEADER_CLIENT_PLAYER_LOGOUT,
    PACKET_HEADER_SERVER_PLAYER_LOGOUT_RESULT,
    PACKET_HEADER_CLIENT_PLAYER_DELETE,
    PACKET_HEADER_SERVER_ALL_PLAYER_INFO,
    PACKET_HEADER_SERVER_NEW_PLAYER_INFO,
    PACKET_HEADER_SERVER_PLAYER_MAP_UPDATE,
    PACKET_HEADER_SERVER_PROPERTY_REMOVED,
    PACKET_HEADER_SERVER_PROPERTY_UPDATE,
    PACKET_HEADER_SERVER_INVENTORY_UPDATE, // 20
    PACKET_HEADER_SERVER_PAWN_REMOVE,
    PACKET_HEADER_SERVER_PAWN_UPDATES,
    PACKET_HEADER_SERVER_PAWN_LIGHT_UPDATE,
    PACKET_HEADER_SERVER_PAWN_POSITION_UPDATE,
    PACKET_HEADER_SERVER_PAWN_COLLISIONFLAGS_UPDATE,
    PACKET_HEADER_SERVER_PAWN_MOVETYPE_UPDATE,
    PACKET_HEADER_SERVER_PAWN_STATE_UPDATE,
    PACKET_HEADER_SERVER_PAWN_VISIBILITY_UPDATE,
    PACKET_HEADER_SCRIPT_DEFINED,
    NUM_PACKET_HEADERS //30
  };


  // Packets that can be placed in a send or recv queue and transmitted
  class rumPacket
  {
  public:

    rumPacket() = default;
    virtual ~rumPacket() = default;

    virtual const char* GetBuffer() const = 0;

    virtual uint32_t GetBufferSize() const = 0;

    virtual PACKET_HEADER_TYPE GetHeaderType() const = 0;

    virtual void Reset() = 0;

    static constexpr size_t s_uiHeaderSize{ sizeof( PACKET_HEADER_TYPE ) };
    static constexpr size_t s_uiEmbeddedSize{ sizeof( PACKET_EMBEDDED_SIZE_TYPE ) };

  protected:

    static constexpr uint32_t s_uiInitialBufferSize{ 1024 };
  };


  class rumInboundPacket : public rumPacket
  {
  public:

    rumInboundPacket( uint32_t i_uiSize = s_uiInitialBufferSize );
    rumInboundPacket( const char* i_pcBuffer, uint32_t i_uiSize, SOCKET i_iSocket );

    // Disallow copying
    rumInboundPacket( const rumInboundPacket& i_rcPacket ) = delete;

    const char* GetBuffer() const override
    {
      return m_cResourceLoader.GetMemFile();
    }

    uint32_t GetBufferSize() const override
    {
      return (uint32_t)m_cResourceLoader.GetSize();
    }

    PACKET_HEADER_TYPE GetHeaderType() const override
    {
      const char* pcBuffer{ m_cResourceLoader.GetMemFile() };
      return ( PACKET_HEADER_TYPE ) * ( pcBuffer + s_uiEmbeddedSize );
    }

    SOCKET GetSocket() const
    {
      return m_iSocket;
    }

    bool HasMoreData()
    {
      return m_cResourceLoader.IsEndOfFile();
    }

    template <typename T>
    const rumInboundPacket& Read( T& i_cVar ) const
    {
      m_cResourceLoader.Serialize( i_cVar );
      return *this;
    }

    const rumInboundPacket& Read( const std::string& i_strVar ) const
    {
      m_cResourceLoader.Serialize( i_strVar );
      return *this;
    }

    const rumInboundPacket& Read( const char* i_strVar ) const
    {
      m_cResourceLoader.Serialize( i_strVar );
      return *this;
    }

    const rumInboundPacket& Read( Sqrat::Object& io_sqVar ) const
    {
      io_sqVar = ScriptRead();
      return *this;
    }

    Sqrat::Object ScriptRead() const;

    template <typename T>
    inline const rumInboundPacket& operator<<( T& i_rcVar ) const
    {
      return Read( i_rcVar );
    }

    inline const rumInboundPacket& operator<<( const std::string& i_strVar ) const
    {
      return Read( i_strVar );
    }

    inline const rumInboundPacket& operator<<( const char* i_pcVar ) const
    {
      return Read( i_pcVar );
    }

    inline const rumInboundPacket& operator<<( Sqrat::Object& i_sqVar ) const
    {
      return Read( i_sqVar );
    }

    void Reset() override;

  private:

    mutable rumResourceLoader m_cResourceLoader;
    SOCKET m_iSocket{ INVALID_SOCKET };
  };


  // This is controlled by the network thread
  class rumInboundPacketPool
  {
  public:

    static void Init( uint32_t i_uiNumPackets );
    static void Shutdown();

    // Move active packets that have been processed back into the available packet pool
    static void Update();

    static rumInboundPacket& CheckoutPacket();

  private:

    static void CheckinPacket( rumInboundPacket* i_rcPacket );

    static std::queue<rumInboundPacket*> s_cAvailablePacketPool;
    static std::vector<rumInboundPacket*> s_cActivePacketVector;
  };


  // Outgoing packet construction helper. Method chaining is used here to provide a streaming style construction to
  // network packets. Outbound packets are always borrowed from a pool to keep heap allocations to a minimum. When an
  // outbound packet is checked out, it maintains a recipient list so that it knows how many recipients it has to send
  // to before relinquishing the packet back to the pool. This is accomplished by calling PushSendRef() when a packet
  // is queued for sending, then PopSendRef() when the packet is actually sent. When the packet is sent, the send ref
  // count is again checked and any time it falls to zero, it is released back to the pool.
  class rumOutboundPacket : public rumPacket
  {
  public:

    rumOutboundPacket( uint32_t i_uiSize = s_uiInitialBufferSize );
    rumOutboundPacket( PacketHeaderType i_eHeader, uint32_t i_uiSize = s_uiInitialBufferSize );

    // Disallow copying
    rumOutboundPacket( const rumOutboundPacket& i_rcPacket ) = delete;

    void AddRecipient( SOCKET i_iSocket )
    {
      m_iRecipients.insert( i_iSocket );
    }

    PACKET_HEADER_TYPE GetHeaderType() const override
    {
      const char* pcBuffer{ m_cResourceSaver.GetMemFile() };
      return (PACKET_HEADER_TYPE)*( pcBuffer + s_uiEmbeddedSize );
    }

    rumOutboundPacket& SetHeader( PacketHeaderType i_eHeader );

    const std::unordered_set<SOCKET>& GetRecipientSet() const
    {
      return m_iRecipients;
    }

    bool HasRecipients() const
    {
      return !m_iRecipients.empty();
    }

    //bool IsRecipientIgnored( SOCKET i_iSocket ) const
    //{
    //  const auto iter{ m_iIgnoredRecipients.find( i_iSocket ) };
    //  return iter != m_iIgnoredRecipients.end();
    //}

    bool IsSending() const
    {
      return m_iSendRefs > 0;
    }

    void PopSendRef()
    {
      --m_iSendRefs;
      rumAssert( m_iSendRefs >= 0 );
    }

    void Reset() override;

    int32_t Send( SOCKET i_iSocket = INVALID_SOCKET );

    template <typename T>
    rumOutboundPacket& Write( const T& i_rcVar )
    {
      m_cResourceSaver.Serialize( i_rcVar );
      return *this;
    }

    rumOutboundPacket& Write( const std::string& i_strVar )
    {
      m_cResourceSaver.Serialize( i_strVar );
      return *this;
    }

    rumOutboundPacket& Write( const char* i_pcVar )
    {
      m_cResourceSaver.Serialize( i_pcVar );
      return *this;
    }

    rumOutboundPacket& Write( Sqrat::Object i_sqVar );

    template <typename T>
    inline rumOutboundPacket& operator<<( const T& i_rcVar )
    {
      return Write( i_rcVar );
    }

    inline rumOutboundPacket& operator<<( const std::string& i_strVar )
    {
      return Write( i_strVar );
    }

    inline rumOutboundPacket& operator<<( const char* i_pcVar )
    {
      return Write( i_pcVar );
    }

    inline rumOutboundPacket& operator<<( Sqrat::Object i_sqVar )
    {
      return Write( i_sqVar );
    }

  private:

    const char* GetBuffer() const override
    {
      return m_cResourceSaver.GetMemFile();
    }

    uint32_t GetBufferSize() const override
    {
      return (uint32_t)m_cResourceSaver.GetPos();
    }

    void SetRawData( const char* i_pcData, uint32_t i_uiNumBytes )
    {
      m_cResourceSaver.SetRawData( i_pcData, i_uiNumBytes );
    }

    bool IsSendable() const
    {
      return m_bHeaderWritten && !m_iRecipients.empty();
    }

    rumOutboundPacket& WriteSize();

    rumResourceSaver m_cResourceSaver;

    std::unordered_set<SOCKET> m_iRecipients;
    //std::set<SOCKET> m_iIgnoredRecipients;

    size_t m_iSendRefs{ 0 };

    bool m_bHeaderWritten{ false };
    bool m_bSizeWritten{ false };
  };


  // This is controlled by the application thread, not the network thread!
  class rumOutboundPacketPool
  {
  public:

    static void Init( uint32_t i_uiNumPackets );
    static void Shutdown();

    // Move active packets that have been sent back into the available packet pool
    static void Update();

    static rumOutboundPacket& CheckoutPacket();

  private:

    static void CheckinPacket( rumOutboundPacket* i_rcPacket );

    static std::queue<rumOutboundPacket*> s_cAvailablePacketPool;
    static std::vector<rumOutboundPacket*> s_cActivePacketVector;
  };
} // namespace network
