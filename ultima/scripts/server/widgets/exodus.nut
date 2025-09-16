class U3_Exodus_Widget extends U3_Widget
{
  constructor()
  {
    base.constructor();
    SetMoveType( MoveType.Stationary );

    SetProperty( State_PropertyID, U3_ExodusComponentState.Functional );
  }
}
