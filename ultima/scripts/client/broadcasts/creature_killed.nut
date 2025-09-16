// Received from the server any time a creature is killed
class Creature_Killed_Broadcast extends rumBroadcast
{
  var1 = 0; // Creature type
  var2 = 0; // XP reward

  function OnRecv()
  {
    local eCreatureType = var1;
    local iXPReward = var2;

    local strDesc;

    if( eCreatureType != rumInvalidAssetID )
    {
      local ciAsset = ::rumGetCreatureAsset( eCreatureType );
      if( ciAsset != null )
      {
        strDesc = format( ::rumGetString( msg_killed_client_StringID ),
                          ::rumGetStringByName( ciAsset.GetName() + "_client_StringID" ) );
        ShowString( strDesc, g_strColorTagArray.Yellow );
      }
    }

    if( iXPReward > 0 )
    {
      strDesc = format( ::rumGetString( msg_experience_client_StringID ), iXPReward );
      ShowString( strDesc, g_strColorTagArray.Green );
    }
  }
}
