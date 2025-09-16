// Sent to client when player resurrects
class Player_Resurrect_Broadcast extends rumBroadcast
{
  var = 0; // Resurrection type


  constructor( ... )
  {
    base.constructor();

    if( vargv.len() > 0 )
    {
      var = vargv[0]; // Player ID
    }
  }


  function OnRecv( i_iSocket, i_ciPlayer )
  {
    if( i_ciPlayer instanceof Player )
    {
      local eResurrectionType = var;
      i_ciPlayer.Resurrect( eResurrectionType );

      i_ciPlayer.PopPacket();
    }
  }
}
