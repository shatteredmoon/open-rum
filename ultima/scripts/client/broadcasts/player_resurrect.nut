// Received from server when a player resurrects
class Player_Resurrect_Broadcast extends rumBroadcast
{
  var = 0;

  constructor( ... )
  {
    base.constructor();

    if( vargv.len() > 0 )
    {
      var = vargv[0];
    }
  }


  function OnRecv()
  {
    local ciPlayer = ::rumGetPlayer( var );
    if( ciPlayer != null && ( ciPlayer instanceof Player ) )
    {
      ciPlayer.UpdateGraphic();
      ciPlayer.SetLightRange( 0 );
    }
  }
}
