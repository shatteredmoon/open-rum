class U1_Barrier_Widget extends U1_Widget
{
  function Dispell()
  {
    if( IsVisible() )
    {
      SetVisibility( false );
      ScheduleRespawn();

      return true;
    }

    return false;
  }
}
