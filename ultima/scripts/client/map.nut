class Map extends rumMap
{
  function OnCreated()
  {
    // Add ref for this map
    g_ciCUO.m_ciCurrentMap = this;
  }


  function OnPlayerRemoved( i_ciPlayer )
  {
    if( i_ciPlayer != null && ( ::rumGetMainPlayer() == i_ciPlayer ) )
    {
      g_ciCUO.m_ciCurrentMap = null;
    }
  }


  function SetPeeringAttributes()
  {
    local ciDrawProps = GetDrawProps();

    if( g_ciCUO.m_bAloft || g_ciCUO.m_bPeering || GetProperty( Map_Always_Peer_PropertyID, false ) )
    {
      ciDrawProps.LineOfSight = false;
      ciDrawProps.ScaleTiles = true;
      ciDrawProps.Lighting = false;
      ciDrawProps.HorizontalTiles = g_ciUI.s_iVisibleMapTilesPeering;
      ciDrawProps.VerticalTiles = g_ciUI.s_iVisibleMapTilesPeering;
      ciDrawProps.TileHorizontalScale = 0.5;
      ciDrawProps.TileVerticalScale = 0.5;
      ciDrawProps.HorizontalTileOffset = g_ciUI.s_iBorderPixelWidth;
      ciDrawProps.VerticalTileOffset = g_ciUI.s_iBorderPixelWidth;
    }
    else
    {
      ciDrawProps.LineOfSight = true;
      ciDrawProps.ScaleTiles = false;
      ciDrawProps.HorizontalTiles = g_ciUI.s_iVisibleMapTilesStandard;
      ciDrawProps.VerticalTiles = g_ciUI.s_iVisibleMapTilesStandard;
      ciDrawProps.TileHorizontalScale = 1.0;
      ciDrawProps.TileVerticalScale = 1.0;
      ciDrawProps.HorizontalTileOffset = g_ciUI.s_iTilePixelWidth;
      ciDrawProps.VerticalTileOffset = g_ciUI.s_iTilePixelWidth;

      local eMapType = GetProperty( Map_Type_PropertyID, MapType.Invalid );
      switch( eMapType )
      {
        case MapType.Dungeon:
        case MapType.Abyss:
        case MapType.Altar:
        case MapType.Cave:
        case MapType.Codex:
          ciDrawProps.Lighting = true;
          break;
      }
    }

    SetDrawProps( ciDrawProps );
  }
}