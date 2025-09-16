#ifndef _S_MAP_H_
#define _S_MAP_H_

#include <u_map.h>

class rumServerPlayer;
class rumServerPawn;

enum MapStatusType
{
  STATUS_UNINITIALIZED,
  STATUS_LOADING,
  STATUS_ACTIVE,
  STATUS_LOAD_FAILED,
  NUM_STATUS_TYPES
};


class rumServerMap : public rumMap
{
public:

  bool AddPawn( rumPawn* i_pcPawn, const rumPosition& i_rcPos ) override;
  bool RemovePawn( rumPawn* i_pcPawn ) override;

  int32_t Exit( Sqrat::Object i_sqInstance, rumMap* i_pcMap );

  int32_t Load() override;

#pragma message("#TODO - use player id or pointer")
  int32_t SynchronizeClient( SOCKET i_iSocket ) const;

  // Sends the packet to all players on the map (unless it is the optional ignored socket)
  //void SendPacket( rumNetwork::rumOutboundPacket& rPacket, SOCKET iIgnoredRecipient = INVALID_SOCKET ) const override;

  void OnPropertyRemoved( rumAssetID i_ePropertyID ) override;
  void OnPropertyUpdated( rumAssetID i_ePropertyID, Sqrat::Object i_sqValue, bool i_bAdded ) override;

  int32_t ScriptBroadcastRadial( Sqrat::Object i_sqInstance, rumPosition i_rcPosition, uint32_t i_uiRadius ) const;
  int32_t ScriptBroadcastRegional( Sqrat::Object i_sqInstance ) const;

  static void ScriptBind();

protected:

  void AllocateGameID( rumUniqueID i_uiGameID = INVALID_GAME_ID ) override;

private:

  int32_t AddPlayer( rumPlayer* i_pcPlayer ) override;

  int32_t Serialize( rumResource& io_rcResource ) final;
  int32_t SerializePawns( rumResource& io_rcResource ) final;

  MapStatusType m_eStatus{ STATUS_UNINITIALIZED };

  //typedef std::queue<Sqrat::Object> PendingPlayerQueue;
  //PendingPlayerQueue pendingPlayers;

  typedef rumMap super;
};

#endif // _S_MAP_H_
