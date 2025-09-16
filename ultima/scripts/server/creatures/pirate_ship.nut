class U1_Pirate_Ship_Creature extends U1_NPC
{
  m_eHeading = Direction.West;


  function OnDeath( i_ciSource, i_bKillCredit = true )
  {
    base.OnDeath( i_ciSource, i_bKillCredit );
    CreateTransport( U1_Frigate_WidgetID, 0, GetMap(), GetPosition() );
  }
}


class U2_Pirate_Ship_Creature extends U2_NPC
{
  m_eHeading = Direction.West;


  function OnDeath( i_ciSource, i_bKillCredit = true )
  {
    base.OnDeath( i_ciSource, i_bKillCredit );

    // There is a small chance of giving items to players
    if( i_ciSource instanceof Player )
    {
      if( rand() % 100 < 10 )
      {
        local iNewAmount = i_ciSource.GetProperty( U2_Blue_Tassles_PropertyID, 0 ) + 1;
        i_ciSource.SetProperty( U2_Blue_Tassles_PropertyID, iNewAmount );
      }
    }

    CreateTransport( U2_Ship_WidgetID, 0, GetMap(), GetPosition() );
  }
}


class U3_Pirate_Ship_Creature extends U3_NPC
{
  m_eHeading = Direction.West;


  function OnDeath( i_ciSource, i_bKillCredit = true )
  {
    base.OnDeath( i_ciSource, i_bKillCredit );
    CreateTransport( U3_Ship_WidgetID, 0, GetMap(), GetPosition() );
  }
}


class U4_Pirate_Ship_Creature extends U4_NPC
{
  m_eHeading = Direction.West;


  function OnDeath( i_ciSource, i_bKillCredit = true )
  {
    CreateTransport( U4_Ship_WidgetID, 0, GetMap(), GetPosition() );
    base.OnDeath( i_ciSource, i_bKillCredit );
  }
}
