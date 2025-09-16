// Sent from client when a player equips armour
class Player_Equip_Armour_Broadcast extends rumBroadcast
{
  var1 = 0;

  constructor( ... )
  {
    base.constructor();

    if( 1 == vargv.len() )
    {
      var1 = vargv[0]; // object id or 0 for remove
    }
  }
}
