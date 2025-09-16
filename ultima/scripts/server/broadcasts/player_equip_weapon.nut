// Received from client when player equips a weapon
class Player_Equip_Weapon_Broadcast extends rumBroadcast
{
  var1 = 0;

  function OnRecv( i_iSocket, i_ciPlayer )
  {
    if( null == i_ciPlayer || !( i_ciPlayer instanceof Player ) )
    {
      return;
    }

    local iWeaponID = var1;
    i_ciPlayer.EquipWeapon( iWeaponID );

    local fDelay = i_ciPlayer.GetActionDelay( ActionDelay.Medium );
    ::rumSchedule( i_ciPlayer, i_ciPlayer.ProcessNextPacket, fDelay );
  }
}
