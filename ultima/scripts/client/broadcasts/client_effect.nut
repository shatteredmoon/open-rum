// Received from server when an effect occurs on an object that should be shown via graphics or sound
class Client_Effect_Broadcast extends rumBroadcast
{
  var1 = 0; // Client effect type
  var2 = 0; // Object id
  var3 = true; // Play sound

  function OnRecv()
  {
    local eType = var1;

    if( ( ClientEffectType.Damage == eType ) || ( ClientEffectType.Cast == eType ) )
    {
      local ciPawn = ::rumFetchPawn( var2 );

      if( ClientEffectType.Damage == eType )
      {
        DoDamageEffect( ciPawn, var3 );
      }
      else if( ClientEffectType.Cast == eType )
      {
        DoSpellEffect( ciPawn, var3 );
      }
    }
    else if( ClientEffectType.ScreenShake == eType )
    {
      DoScreenShakeEffect( var3 );
    }
  }
}
