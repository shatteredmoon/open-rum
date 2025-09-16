class U3_Castle_Fire_2_Map extends Map
{
  m_bDestroyed = false;


  function Destroy()
  {
    if( !m_bDestroyed )
    {
      DoScreenShakeEffect();
      ::rumSchedule( this, Destroy, frand( 1.0 ) );
    }
  }


  function DestroyEnd()
  {
    m_bDestroyed = true;
  }


  function DestroyStart()
  {
    ::rumSchedule( this, DestroyEnd, 120.0 );
    Destroy();
  }
}
