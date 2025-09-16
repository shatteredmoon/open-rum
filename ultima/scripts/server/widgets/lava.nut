class U3_Lava_Widget extends U3_Widget
{
  function AffectCreature( i_ciCreature )
  {
    if( i_ciCreature.GetMap() == GetMap() && i_ciCreature.GetPosition().Equals( GetPosition() ) )
    {
      local eMoveType = i_ciCreature.GetMoveType();
      local bImmune = !IsHarmful( i_ciCreature );

      if( i_ciCreature instanceof Player )
      {
        local iFlags = i_ciCreature.GetProperty( U3_Marks_PropertyID, 0 );
        if( ::rumBitOn( iFlags, U3_MarkType.Fire ) )
        {
          bImmune = true;
        }
      }

      if( !bImmune )
      {
        i_ciCreature.Burn();
        base.AffectCreature( i_ciCreature );
      }
    }
  }

  function IsHarmful( i_ciCreature )
  {
    local eMoveType = i_ciCreature.GetMoveType();
    local bImmune = i_ciCreature.ResistsFire() || ( MoveType.Aerial == eMoveType || MoveType.Drifts == eMoveType ||
                                                    MoveType.Hovers == eMoveType );
    return IsVisible() && !bImmune;
  }
}


class U4_Lava_Widget extends U4_Widget
{
  function AffectCreature( i_ciCreature )
  {
    if( ( i_ciCreature.GetMap() == GetMap() ) && i_ciCreature.GetPosition().Equals( GetPosition() ) )
    {
      if( IsHarmful( i_ciCreature ) )
      {
        i_ciCreature.Burn();
      }

      base.AffectCreature( i_ciCreature )
    }
  }


  function IsHarmful( i_ciCreature )
  {
    local eMoveType = i_ciCreature.GetMoveType();
    local bImmune = i_ciCreature.ResistsFire() || ( MoveType.Aerial == eMoveType || MoveType.Drifts == eMoveType ||
                                                    MoveType.Hovers == eMoveType );
    return IsVisible() && !bImmune;
  }
}
