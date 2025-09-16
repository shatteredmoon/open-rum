#ifndef _S_PLAYER_H_
#define _S_PLAYER_H_

#include <platform.h>
#include <u_player.h>
#include <u_structs.h>
#include <u_timer.h>

#include <queue>
#include <unordered_map>

class rumMap;
class rumServerInventory;
class rumServerPawn;


class rumServerPlayer : public rumPlayer
{
public:

  struct PlayerCreationInfo
  {
    std::string strPlayerName;
    rumTimer timeSubmitted;
  };

  void CheckIdleTime() override;

  bool IsPersistenceEnabled() const override
  {
    return m_bPersistenceEnabled;
  }

  void SetPersistenceEnabled( bool i_bEnabled )
  {
    m_bPersistenceEnabled = i_bEnabled;
  }

  bool IsWaitingForBroadcastPacket() const
  {
    return m_bPacketRequested;
  }

  void KeepAlive() override;

  void OnCreated( SOCKET i_iSocket, std::string i_strPlayerName, rumUniqueID i_uiPlayerID,
                  rumPawn* i_pcPawn ) override;

  void OnMapUpdated( rumUniqueID i_eMapID, const rumPosition& i_rcPos ) override;

  void OnPositionUpdated( const rumPosition& i_rcPos, uint32_t i_uiDistance ) override;

  void OnRemoved() override;

  const std::string& PasswordGet() const
  {
    return m_strPassword;
  }

  void PasswordSet( const std::string& i_strPassword )
  {
    m_strPassword = i_strPassword;
  }

  // Pops a received packet and executes it
  bool PopPacket() override;

  static void CheckIdlePlayers();

  static void CheckPlayerCreations();
  static void CheckPlayerLogouts();
  static PlayerCreateResultType CheckPlayerName( const std::string& i_strPlayerName );

  static rumPlayer* FetchBySocket( SOCKET i_iSocket );

  using LoginResult = std::pair<rumPlayer::PlayerLoginResultType, Sqrat::Object>;
  static LoginResult OnPlayerLogin( SOCKET i_iSocket, const std::string& i_strPlayerName, rumUniqueID i_uiPlayerID,
                                    bool i_bRestoreDB );

  static void ProcessBroadcast( SOCKET i_iSocket, Sqrat::Object i_sqInstance );

  static void ProcessPlayerCreateRequest( SOCKET i_iSocket, const std::string& i_strPlayerName );

  static void ProcessPlayerDeleteRequest( SOCKET i_iSocket, const std::string& i_strPlayerName );

  static LoginResult ProcessPlayerLoginRequest( SOCKET i_iSocket, const std::string& i_strPlayerName,
                                                bool i_bRestoreDB );

  static Sqrat::Object ProcessPlayerLoginRequest_VM( SOCKET i_iSocket, Sqrat::Object i_sqPlayerName,
                                                     bool i_bRestoreDB );
  static void ProcessPlayerLogoutRequest( SOCKET i_iSocket );

  static void ScriptBind();

  static int32_t ScriptBroadcastGlobal( Sqrat::Object sqInstance );
  static int32_t ScriptBroadcastPrivate( SOCKET i_iSocket, Sqrat::Object sqInstance );

  static int32_t SendGlobalPacket( rumNetwork::rumOutboundPacket& i_rcPacket,
                                   SOCKET i_iIgnoredRecipient = INVALID_SOCKET );
  static void SendPacket( rumNetwork::rumOutboundPacket& i_rcPacket, const GameIDHash& i_hashPlayers,
                          rumUniqueID i_uiIgnoredPlayerID = INVALID_GAME_ID );
  static void SendPacket( rumNetwork::rumOutboundPacket& i_rcPacket, rumUniqueID i_uiPlayerID );

  static void SendPlayerLoginFailed( PlayerLoginResultType i_eReason, SOCKET i_iSocket );

  static void Shutdown();

  // Sends NEW_PLAYER_INFO and MAP_UPDATE notifications to all other players when this player logs in
  static void SyncAllPlayerInfo( SOCKET i_iSocket );

private:

  void InitDone() override;
  bool InitFromDB() override;

  void PawnPositionSave( const rumPosition& i_rcPos ) const;
  void PawnPositionSave( int32_t i_iPosX, int32_t i_iPosY ) const
  {
    PawnPositionSave( rumPosition( i_iPosX, i_iPosY ) );
  }

  bool ProcessBroadcastPacket( Sqrat::Object i_sqInstance ) override;

  int32_t QueryInventory( rumPawn* i_pcPawn );

  void RemovePackets();

  void Replicate() override;

  static rumPlayer* Create( SOCKET i_iSocket );
  static Sqrat::Object ScriptFetchBySocket( SOCKET i_iSocket );

  static bool Remove( rumUniqueID i_uiPlayerID );

  // A queue of received packets mapped to each player's socket id
  typedef std::unordered_map<SOCKET, std::queue<Sqrat::Object> > PlayerPacketContainer;
  static PlayerPacketContainer s_hashPlayerPackets;

  typedef std::unordered_map<SOCKET, PlayerCreationInfo> PendingPlayerContainer;
  static PendingPlayerContainer s_hashPendingPlayers;

  typedef std::queue<SOCKET> PendingLogoutContainer;
  static PendingLogoutContainer s_queuePendingPlayerLogouts;

  typedef std::unordered_map<SOCKET, rumUniqueID> SocketContainer;
  static SocketContainer s_hashSockets;

  static constexpr uint32_t s_uiMaxQueuedPackets{ 32 };
  static constexpr uint32_t s_uiPositionSaveMoves{ 10 };

  std::string m_strPassword;

  uint32_t m_uiMovesSinceSave{ 0 };
  uint32_t m_uiMovesOnCurrentMap{ 0 };

  rumTimer m_cLastPacketTime{};

  // Set to true when a player has requested a packet from the queue
  bool m_bPacketRequested{ true };

  // When true, db modifications will be written when attributes change
  bool m_bPersistenceEnabled{ false };

  bool m_bIdleWarningSent{ false };

  using super = rumPlayer;
};

#endif // _S_PLAYER_H_
