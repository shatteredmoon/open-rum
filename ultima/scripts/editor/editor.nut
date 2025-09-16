class U1_NPC extends NPC
{
  function OnEditorPlaced()
  {
    local ciMap = GetMap();
    local eMapType = ciMap.GetProperty( Map_Type_PropertyID, MapType.Invalid );
    switch( eMapType)
    {
      case MapType.World:
      case MapType.Abyss:
      case MapType.Altar:
      case MapType.Dungeon:
        SetProperty( NPC_Posture_Type_PropertyID, PostureType.Attack );
        break;
    }
  }
}


class U2_NPC extends NPC
{
  function OnEditorPlaced()
  {
    local ciMap = GetMap();
    if( ciMap.GetAssetID() == U2_Earth_Legends_Castle_Shadow_Guard_MapID )
    {
      SetProperty( NPC_Posture_Type_PropertyID, PostureType.Attack );
    }
    else
    {
      local eMapType = ciMap.GetProperty( Map_Type_PropertyID, MapType.Invalid );
      switch( eMapType)
      {
        case MapType.World:
        case MapType.Dungeon:
        case MapType.Tower:
          SetProperty( NPC_Posture_Type_PropertyID, PostureType.Attack );
          break;
      }
    }
  }
}


class U3_NPC extends NPC
{
  function OnEditorPlaced()
  {
    local ciMap = GetMap();
    if( ciMap.GetAssetID() == U3_Castle_Fire_MapID )
    {
      SetProperty( NPC_Posture_Type_PropertyID, PostureType.Attack );
    }
    else
    {
      local eMapType = ciMap.GetProperty( Map_Type_PropertyID, MapType.Invalid );
      switch( eMapType)
      {
        case MapType.World:
        case MapType.Dungeon:
          SetProperty( NPC_Posture_Type_PropertyID, PostureType.Attack );
          break;
      }
    }
  }
}


class U4_NPC extends NPC
{
  function OnEditorPlaced()
  {
    local ciMap = GetMap();
    local eMapType = ciMap.GetProperty( Map_Type_PropertyID, MapType.Invalid );
    switch( eMapType)
    {
      case MapType.World:
      case MapType.Abyss:
      case MapType.Altar:
      case MapType.Dungeon:
        SetProperty( NPC_Posture_Type_PropertyID, PostureType.Attack );
        break;
    }
  }
}



function OnFrameStart( i_fElapsedTime )
{
  // Shift down
  local ciVector = rumVector( 0, 1 );

  ::rumGetGraphic( U1_Water_GraphicID ).Shift( ciVector );

  ::rumGetGraphic( U2_Water_GraphicID ).Shift( ciVector );
  ::rumGetGraphic( U2_Force_Field_GraphicID ).Shift( ciVector );

  ::rumGetGraphic( U3_Water_GraphicID ).Shift( ciVector );
  ::rumGetGraphic( U3_Force_Field_GraphicID ).Shift( ciVector );
  ::rumGetGraphic( U3_Lava_GraphicID ).Shift( ciVector );
  ::rumGetGraphic( U3_Moongate_GraphicID ).Shift( ciVector );
  ::rumGetGraphic( U3_Mark_Rod_GraphicID ).Shift( ciVector );

  ::rumGetGraphic( U4_Water_GraphicID ).Shift( ciVector );
  ::rumGetGraphic( U4_Water_Deep_GraphicID ).Shift( ciVector );
  ::rumGetGraphic( U4_Water_Shoals_GraphicID ).Shift( ciVector );
  ::rumGetGraphic( U4_Field_Energy_GraphicID ).Shift( ciVector );
  ::rumGetGraphic( U4_Field_Fire_GraphicID ).Shift( ciVector );
  ::rumGetGraphic( U4_Field_Force_GraphicID ).Shift( ciVector );
  ::rumGetGraphic( U4_Field_Lightning_GraphicID ).Shift( ciVector );
  ::rumGetGraphic( U4_Field_Poison_GraphicID ).Shift( ciVector );
  ::rumGetGraphic( U4_Field_Sleep_GraphicID ).Shift( ciVector );
  //::rumGetGraphic( U4_Phantom_Field_GraphicID ).Shift( ciVector );
  ::rumGetGraphic( U4_Lava_GraphicID ).Shift( ciVector );
  ::rumGetGraphic( U4_Projectile_Lava_GraphicID ).Shift( ciVector );
}


function OnGameInit( i_fEditorTime )
{
  print( "Editor: Initializing editor scripts\n" );

  // In the editor, move the animation set over by one to show the conspicuous secret door
  local ciGraphic = ::rumGetGraphic( U4_Door_Secret_GraphicID );
  local ciGdp = ciGraphic.GetAttributes();
  ciGdp.AnimationSet = 1;
  ciGraphic.SetAttributes( ciGdp );

  ciGraphic = ::rumGetGraphic( U3_Door_Secret_GraphicID );
  ciGdp = ciGraphic.GetAttributes();
  ciGdp.AnimationSet = 1;
  ciGraphic.SetAttributes( ciGdp );

  ciGraphic = ::rumGetGraphic( U2_Door_Secret_GraphicID );
  ciGdp = ciGraphic.GetAttributes();
  ciGdp.AnimationSet = 1;
  ciGraphic.SetAttributes( ciGdp );

  ciGraphic = ::rumGetGraphic( U4_Moongate_GraphicID );
  ciGdp = ciGraphic.GetAttributes();
  ciGdp.AnimationSet = MoongateAnimFrame.FullyOpen;
  ciGraphic.SetAttributes( ciGdp );

  ciGraphic = ::rumGetGraphic( U3_Moongate_GraphicID );
  ciGdp = ciGraphic.GetAttributes();
  ciGdp.AnimationSet = MoongateState.Open;
  ciGraphic.SetAttributes( ciGdp );

  ciGraphic = ::rumGetGraphic( U2_Timegate_GraphicID );
  ciGdp = ciGraphic.GetAttributes();
  ciGdp.AnimationSet = MoongateAnimFrame.FullyOpen;
  ciGraphic.SetAttributes( ciGdp );

  ciGraphic = ::rumGetGraphic( U4_Trap_GraphicID );
  ciGdp = ciGraphic.GetAttributes();
  ciGdp.AnimationSet = 1;
  ciGraphic.SetAttributes( ciGdp );

  ciGraphic = ::rumGetGraphic( U3_Trap_GraphicID );
  ciGdp = ciGraphic.GetAttributes();
  ciGdp.AnimationSet = 1;
  ciGraphic.SetAttributes( ciGdp );

  return true;
}
