class U2_Timegate_Widget extends U2_Widget
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


class U2_Timegate_Facade_Widget extends U2_Widget
{
  m_iAnimFrame = MoongateAnimFrame.FullyOpen;


  constructor()
  {
    base.constructor();
    SetMoveType( MoveType.Stationary );
    UseAnimationSet( m_iAnimFrame );
  }
}
