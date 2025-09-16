// Sent to client to apply graphic/sound effects
class Client_Effect_Broadcast extends rumBroadcast
{
  var1 = 0;
  var2 = 0;

  constructor(...)
  {
    base.constructor();

    if( vargv.len() > 0 )
    {
      var1 = vargv[0]; // Client effect type

      if( vargv.len() > 1 )
      {
        var2 = vargv[1]; // Object ID
      }
    }
  }
}


function SendClientEffect( i_ciSource, i_eEffectType )
{
  if( null == i_ciSource )
  {
    return;
  }

  local uiRadius = 11;

  local ciMap = i_ciSource.GetMap();
  if( ciMap != null )
  {
    local ciBroadcast = ::rumCreate( Client_Effect_BroadcastID, i_eEffectType, i_ciSource.GetID() );
    ciMap.SendRadial( ciBroadcast, i_ciSource.GetPosition(), uiRadius );
  }
}
