class Widget extends rumWidget
{
  // Default value for how often a widget will affect any creature that is occupying the same space as the widget
  static s_fRecurrencyInterval = 2.0;

  m_iGroupID = 0;
  m_bDefaultVisibility = true;


  function AffectAllCreatures()
  {
    local ciCreature = null;
    local ciMap = GetMap();
    local ciPosData = ciMap.GetPositionData( GetPosition() );
    while( ciCreature = ciPosData.GetNext( rumCreaturePawnType ) )
    {
      AffectCreature( ciCreature );
    }
  }


  function AffectAllPlayers()
  {
    local ciPlayer = null;
    local ciMap = GetMap();
    local ciPosData = ciMap.GetPositionData( GetPosition() );
    while( ciPlayer = ciPosData.GetNext( rumCreaturePawnType ) )
    {
      if( ciPlayer instanceof Player )
      {
        AffectCreature( ciPlayer );
      }
    }
  }


  function AffectCreature( i_ciCreature )
  {
    local bRecurrent = GetProperty( Widget_Recurrent_PropertyID, false );
    if( bRecurrent )
    {
      // Schedule another occurrence
      local fInterval = GetProperty( Widget_Recurrency_Interval_PropertyID, s_fRecurrencyInterval );
      ::rumSchedule( this, AffectCreature, fInterval, i_ciCreature );
    }
  }


  function Damage( i_iAmount, i_ciSource, i_eWeaponType = rumInvalidAssetID, i_bSendClientEffect = true )
  {
    return false;
  }


  function Expire()
  {
    local ciMap = GetMap();
    ciMap.RemovePawn( this );
  }


  function GetArmourType()
  {
    return rumInvalidAssetID;
  }


  function GetDexterity()
  {
    return 25;
  }


  function IsDead()
  {
    return true;
  }


  function IsHarmful( i_ciCreature )
  {
    return false;
  }


  function OnPropertyUpdated( i_ePropertyID, i_vValue )
  {
    if( Widget_Group_ID_PropertyID == i_ePropertyID )
    {
      m_iGroupID = i_vValue;
    }
    else if( Widget_Default_Visibility_PropertyID == i_ePropertyID )
    {
      m_bDefaultVisibility = i_vValue;
      SetVisibility( m_bDefaultVisibility );
    }
    else if( Light_Source_Range_PropertyID == i_ePropertyID )
    {
      SetLightRange( i_vValue );
    }
  }


  function Respawn()
  {
    SetVisibility( true );
  }


  function ScheduleRespawn()
  {
    local fRespawnTime = GetProperty( Respawn_Interval_PropertyID, 10.0 );
    if( fRespawnTime > 0.0 )
    {
      ::rumSchedule( this, Respawn, fRespawnTime );
    }
  }
}


class U1_Widget extends Widget
{}


class U1_Hotspot_Widget extends U1_Widget
{}


class U2_Widget extends Widget
{}


class U3_Widget extends Widget
{}


class U4_Widget extends Widget
{}
