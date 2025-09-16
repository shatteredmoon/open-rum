#ifndef _C_PLAYER_H_
#define _C_PLAYER_H_

#include <u_player.h>


class rumClientPlayer : public rumPlayer
{
public:

  static rumClientPlayer* Create( rumUniqueID i_uiPlayerID );

  static rumClientPlayer* FetchByPlayerID( rumUniqueID i_uiPlayerID )
  {
    return SafeCastNative( super::FetchByPlayerID( i_uiPlayerID ) );
  }

  static rumClientPlayer* SafeCastNative( rumPlayer* i_pcPlayer )
  {
    return dynamic_cast<rumClientPlayer*>( i_pcPlayer );
  }

  static rumClientPlayer* GetClientPlayer()
  {
    return FetchByPlayerID( s_uiClientPlayerID );
  }

  static Sqrat::Object GetClientPlayerVM()
  {
    return GetScriptPlayer( s_uiClientPlayerID );
  }

  static void CreateResult( PlayerCreateResultType i_eResult, const std::string& i_strPlayerName );
  static void LoginResult( rumUniqueID i_uiPlayerID, PlayerLoginResultType i_eResult );
  static void LogoutResult( rumUniqueID i_uiPlayerID );

  static void ProcessBroadcast( Sqrat::Object i_sqInstance );

  static void RecvInventoryUpdate( rumUniqueID i_uiPlayerID, rumUniqueID i_uiInventoryID, rumAssetID i_eInventoryID,
                                   bool i_bCreate );
  static void RecvMapUpdate( rumUniqueID i_uiPlayerID, rumAssetID i_eMapAssetID, rumUniqueID i_uiMapID,
                             const rumPosition& i_rcPos );
  static void RecvNewPlayerInfo( const std::string& i_strPlayerName, rumUniqueID i_uiPlayerID,
                                 rumUniqueID i_uiPawnID );

  static PlayerCreateResultType RequestCreatePlayer( const std::string& i_strPlayerName );
  static void RequestDeletePlayer( const std::string& i_strPlayerName );

  static PlayerLoginResultType RequestLogin( const std::string& i_strPlayerName );

  static void RequestLogout();

  static void ScriptBind();
  static int32_t SendPacketVM( Sqrat::Object i_sqInstance );

  // The active player for this particular client
  static void SetClientPlayerID( rumUniqueID i_uiPlayerID )
  {
    s_uiClientPlayerID = i_uiPlayerID;
  }

  static rumUniqueID GetClientPlayerID()
  {
    return s_uiClientPlayerID;
  }

  // The server-side socket
  static void SetClientPlayerSocket( SOCKET i_iSocket )
  {
    s_uiClientPlayerSocket = i_iSocket;
  }

  static SOCKET GetClientPlayerSocket()
  {
    return s_uiClientPlayerSocket;
  }

private:

  typedef rumPlayer super;

  static rumUniqueID s_uiClientPlayerID;
  static SOCKET s_uiClientPlayerSocket;
};

#endif // _C_PLAYER_H_
