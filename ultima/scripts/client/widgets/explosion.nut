class U1_Explosion_Widget extends U1_Widget
{
  function OnAddedToMap()
  {
    PlaySound3D( Player_Hit_SoundID, GetPosition() );
  }
}
