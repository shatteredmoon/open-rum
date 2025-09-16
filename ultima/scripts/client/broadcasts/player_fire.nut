// Sent from client
class Player_Fire_Broadcast extends rumBroadcast
{
  var1 = 0; // Attack type
  var2 = 0; // Direction or TargetID

  constructor( i_vVar1, i_vVar2 )
  {
    base.constructor();

    var1 = i_vVar1;
    var2 = i_vVar2;
  }
}
