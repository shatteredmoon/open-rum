class U4_Twister_Widget extends U4_Widget
{
  static s_fMoveTime = 5.0;
  static s_fRelocationTime = 120.0;


  constructor()
  {
    base.constructor();
    SetMoveType( MoveType.Hovers );

    ::rumSchedule( this, Move, s_fMoveTime );
    ::rumSchedule( this, Relocate, s_fRelocationTime );
  }


  function AffectCreature( i_ciCreature )
  {
    if( i_ciCreature.IsVisible() && !i_ciCreature.IsDead() )
    {
      if( i_ciCreature.GetMap() == GetMap() && i_ciCreature.GetPosition().Equals( GetPosition() ) )
      {
        local ciBroadcast = ::rumCreate( Client_Effect_BroadcastID, ClientEffectType.ScreenShake );
        i_ciCreature.SendBroadcast( ciBroadcast );

        local iDamage = 16 + rand() % 32;
        i_ciCreature.Damage( iDamage, this, null, true );

        base.AffectCreature( i_ciCreature );
      }
    }
  }


  function IsHarmful( i_ciCreature )
  {
    return true;
  }


  function Move()
  {
    local ciMap = GetMap();
    ciMap.OffsetPawn( this, GetRandomDirectionVector() );
    ::rumSchedule( this, Move, s_fMoveTime );
  }


  function Relocate()
  {
    local ciMap = GetMap();
    local ciPos = rumPos( rand() % ciMap.GetNumColumns(), rand() % ciMap.GetNumRows() );
    ciMap.MovePawn( this, ciPos, rumIgnoreDistanceMoveFlag );
    ::rumSchedule( this, Relocate, s_fRelocationTime );
  }
}
