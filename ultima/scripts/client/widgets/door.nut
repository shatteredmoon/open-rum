class U1_Door_Widget extends U1_Widget
{
  // The base door object that all U1 door types extend from
}


class U1_Door_Locked_Widget extends U1_Door_Widget
{
  function OnPropertyUpdated( i_ePropertyID, i_vValue )
  {
    if( Locked_PropertyID == i_ePropertyID )
    {
      if( (null == i_vValue) || !i_vValue )
      {
        SetGraphic( U4_Door_GraphicID );
      }
      else
      {
        // Reset to the default graphic
        SetGraphic( U4_Door_Locked_GraphicID );
      }
    }
  }
}


class U2_Door_Widget extends U2_Widget
{
  // The base door object that all U2 door types extend from
}


class U2_Door_Locked_Widget extends U2_Door_Widget
{
  function OnPropertyUpdated( i_ePropertyID, i_vValue )
  {
    if( Locked_PropertyID == i_ePropertyID )
    {
      if( (null == i_vValue) || !i_vValue )
      {
        SetGraphic( U2_Door_GraphicID );
      }
      else
      {
        // Reset to the default graphic
        SetGraphic( U4_Door_Locked_GraphicID );
      }
    }
  }
}


class U3_Door_Widget extends U3_Widget
{
  // The base door object that all U3 door types extend from
}


class U3_Door_Locked_Widget extends U3_Door_Widget
{
  function OnPropertyUpdated( i_ePropertyID, i_vValue )
  {
    if( Locked_PropertyID == i_ePropertyID )
    {
      if( (null == i_vValue) || !i_vValue )
      {
        SetGraphic( U4_Door_GraphicID );
      }
      else
      {
        // Reset to the default graphic
        SetGraphic( U4_Door_Locked_GraphicID );
      }
    }
  }
}


class U4_Door_Widget extends U4_Widget
{
  // The base door object that all U4 door types extend from
}


class U4_Door_Locked_Widget extends U4_Door_Widget
{
  function OnPropertyUpdated( i_ePropertyID, i_vValue )
  {
    if( Locked_PropertyID == i_ePropertyID )
    {
      local ciAsset = ::rumGetWidgetAsset( GetAssetID() );
      if( ciAsset != null )
      {
        if( (null == i_vValue) || !i_vValue )
        {
          if( ciAsset.GetGraphicID() == U4_Door_Windowed_Locked_GraphicID )
          {
            SetGraphic( U4_Door_Windowed_GraphicID )
          }
          else
          {
            SetGraphic( U4_Door_GraphicID );
          }
        }
        else
        {
          // Reset to the default graphic
          SetGraphic( ciAsset.GetGraphicID() );
        }
      }
    }
  }
}
