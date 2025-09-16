class U3_Moongate_Widget extends U3_Widget
{
  static s_fAnimationInterval = 0.25;

  m_iAnimFrame = MoongateState.Closed;


  constructor()
  {
    base.constructor();
    SetMoveType( MoveType.Stationary );
    UseAnimationSet( m_iAnimFrame );
  }


  function Close()
  {
    UseAnimationSet( MoongateState.Closed );
  }


  function GetDescription()
  {
    if( MoongateState.Closed == m_iAnimFrame )
    {
      return ::rumGetString( token_circle_stones_client_StringID );
    }

    return base.GetDescription();
  }


  function OnPropertyUpdated( i_ePropertyID, i_vValue )
  {
    base.OnPropertyUpdated( i_ePropertyID, i_vValue );

    if( State_PropertyID == i_ePropertyID )
    {
      if( MoongateState.Open == i_vValue )
      {
        Open();
      }
      else if( MoongateState.Closed == i_vValue )
      {
        Close();
      }
    }
  }


  function Open()
  {
    UseAnimationSet( MoongateState.Open );
  }
}


class U3_Moongate_Alt_Widget extends U3_Widget
{
  constructor()
  {
    base.constructor();
    SetMoveType( MoveType.Stationary );

    // Always show these moongates as fully open
    UseAnimationSet( MoongateState.Open );
  }
}


class U4_Moongate_Widget extends U4_Widget
{
  static s_fAnimationInterval = 0.25;

  m_iAnimFrame = MoongateAnimFrame.Closed;


  constructor()
  {
    base.constructor();
    SetMoveType( MoveType.Stationary );
    UseAnimationSet( m_iAnimFrame );
  }


  function Close()
  {
    if( m_iAnimFrame > MoongateAnimFrame.Closed )
    {
      UseAnimationSet( --m_iAnimFrame );
      ::rumSchedule( this, Close, s_fAnimationInterval );
    }
    else
    {
      UseAnimationSet( MoongateAnimFrame.Closed );
    }
  }


  function GetDescription()
  {
    if( MoongateAnimFrame.Closed == m_iAnimFrame )
    {
      return ::rumGetString( token_circle_stones_client_StringID );
    }

    return base.GetDescription();
  }


  function OnPropertyUpdated( i_ePropertyID, i_vValue )
  {
    base.OnPropertyUpdated( i_ePropertyID, i_vValue );

    if( State_PropertyID == i_ePropertyID )
    {
      if( MoongateState.Open == i_vValue )
      {
        Open();
      }
      else if( MoongateState.Closed == i_vValue )
      {
        Close();
      }
    }
  }


  function Open()
  {
    if( m_iAnimFrame < MoongateAnimFrame.FullyOpen )
    {
      UseAnimationSet( ++m_iAnimFrame );
      ::rumSchedule( this, Open, s_fAnimationInterval );
    }
    else
    {
      UseAnimationSet( MoongateAnimFrame.FullyOpen );
    }
  }
}
