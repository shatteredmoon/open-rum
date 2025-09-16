// buffer += "s may NOT wear ";
// buffer += "s may NOT use a ";

// Sent from client when a player equips a weapon
class Player_Equip_Weapon_Broadcast extends rumBroadcast
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
