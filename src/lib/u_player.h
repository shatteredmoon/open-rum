#pragma once

#include <u_utility.h>
#include <u_script.h>
#include <u_structs.h>

#include <unordered_map>

namespace rumNetwork
{
  class rumOutboundPacket;
}

class rumInventory;
class rumPawn;

#define SCRIPT_PLAYER_SCRIPT_CLASS "Player"


class rumPlayer
{
public:

  virtual ~rumPlayer() = default;

  typedef std::unordered_map<rumUniqueID, rumPlayer*> PlayerContainer;

  enum PlayerCreateResultType
  {
    PLAYER_CREATE_SUCCESS,
    PLAYER_CREATE_ERROR,
    PLAYER_CREATE_FAIL_NAME_EXISTS,
    PLAYER_CREATE_FAIL_NAME_INVALID,
    PLAYER_CREATE_FAIL_NAME_RESTRICTED,
    PLAYER_CREATE_FAIL_TIMEOUT
  };

  enum PlayerLoginResultType
  {
    PLAYER_LOGIN_SUCCESS,
    PLAYER_LOGIN_ERROR,
    PLAYER_LOGIN_FAIL_NAME_INVALID,
    PLAYER_LOGIN_FAIL_PLAYER_NOT_FOUND,
    PLAYER_LOGIN_FAIL_PLAYER_ACTIVE,
    PLAYER_LOGIN_FAIL_PLAYER_CYCLING
  };

  virtual void CheckIdleTime()
  {}

  rumPawn* GetPlayerPawn() const;

  uint64_t GetPlayerID() const
  {
    return m_uiPlayerID;
  }

  const std::string& GetName() const
  {
    return m_strName;
  }

  void SetName( const std::string& i_strName )
  {
    m_strName = i_strName;
  }

  rumUniqueID GetPawnID() const
  {
    return m_uiPawnID;
  }

  // TODO - this should be protected
  void SetPawnID( rumUniqueID i_uiPawnID )
  {
    m_uiPawnID = i_uiPawnID;
  }

  SOCKET GetSocket() const
  {
    return m_iSocket;
  }

  virtual bool IsPersistenceEnabled() const
  {
    return false;
  }

  virtual void KeepAlive()
  {}

  virtual bool PopPacket()
  {
    return false;
  }

  virtual void Replicate()
  {}

  virtual void InitDone()
  {}

  virtual bool InitFromDB()
  {
    return true;
  }

  static bool Manage( rumPlayer* i_pcPlayer );

  static rumPlayer* FetchByPlayerID( rumUniqueID i_uiPlayerID );

  virtual bool ProcessBroadcastPacket( Sqrat::Object i_sqInstance );

  virtual void OnCreated( SOCKET i_iSocket, std::string i_strPlayerName, rumUniqueID i_uiPlayerID, rumPawn* i_pcPawn );

  virtual void OnMapUpdated( rumUniqueID i_eMapID, const rumPosition& i_rcPos )
  {}

  virtual void OnPositionUpdated( const rumPosition& i_rcPos, uint32_t i_uiDistance )
  {}

  virtual void OnRemoved()
  {}

  static rumPlayer* Create( SOCKET i_iSocket );

  static const std::string& GetCreateResultTypeString( PlayerCreateResultType i_eType )
  {
    return s_aPlayerCreateResultTypeStrings[i_eType];
  }

  static const std::string& GetLoginResultTypeString( PlayerLoginResultType i_eType )
  {
    return s_aPlayerLoginResultTypeStrings[i_eType];
  }

  static uint32_t GetNumPlayers()
  {
    return (uint32_t)s_hashPlayers.size();
  }

  static Sqrat::Object GetPlayerClass()
  {
    return s_sqClass;
  }

  static PlayerContainer& GetPlayerHash()
  {
    return s_hashPlayers;
  }

  // Not to be confused with ScriptGetPlayer. This version uses a PlayerID which will have a high byte of 0x02 unlike
  // ScriptGetPlayer which uses a pawn ID that will have a high byte of 0x01. The only script that will use this is are
  // client scripts that fetch the main player because scripts do not have to be aware of the PlayerID. On the server,
  // the PlayerID is never exposed to scripts so there's no real mechanism for fetching a player in this way there.
  static Sqrat::Object GetScriptPlayer( rumUniqueID i_uiPlayerID );

  static int32_t Init();

  static void ScriptBind();
  static Sqrat::Object ScriptGetAllPlayers();
  static Sqrat::Object ScriptGetPlayer( rumUniqueID i_uiGameID );
  static Sqrat::Object ScriptGetPlayerByName( Sqrat::Object i_sqStrName );

  static void Shutdown();

protected:

  static bool Remove( rumUniqueID i_uiPlayerID );

  void SetPlayerID( rumUniqueID i_uiPlayerID )
  {
    m_uiPlayerID = i_uiPlayerID;
  }

  void SetSocket( SOCKET i_iSocket )
  {
    m_iSocket = i_iSocket;
  }

private:

  static PlayerContainer s_hashPlayers;

  // The script-side class that creates players
  static Sqrat::Object s_sqClass;

  static const std::string s_aPlayerCreateResultTypeStrings[];
  static const std::string s_aPlayerLoginResultTypeStrings[];

  // The player's name
  std::string m_strName;

  // The GameID of the player
  rumUniqueID m_uiPlayerID{ INVALID_GAME_ID };

  // The GameID of the native pawn that this player controls
  rumUniqueID m_uiPawnID{ INVALID_GAME_ID };

  SOCKET m_iSocket{ INVALID_SOCKET };
};
