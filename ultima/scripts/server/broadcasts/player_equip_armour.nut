// Received from client when player equips armour
class Player_Equip_Armour_Broadcast extends rumBroadcast
{
  var1 = 0;

  function OnRecv( i_iSocket, i_ciPlayer )
  {
    if( null == i_ciPlayer || !( i_ciPlayer instanceof Player ) )
    {
      return;
    }

    local iArmourID = var1;
    i_ciPlayer.EquipArmour( iArmourID );

    local fDelay = i_ciPlayer.GetActionDelay( ActionDelay.Long );
    ::rumSchedule( i_ciPlayer, i_ciPlayer.ProcessNextPacket, fDelay );
  }
}
