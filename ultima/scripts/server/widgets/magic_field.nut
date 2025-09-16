class U4_Magic_Field_Widget extends U4_Widget
{
  function Dispell()
  {
    local bDispelled = false;

    if( IsVisible() )
    {
      SetVisibility( false );
      ScheduleRespawn();

      bDispelled = true;
    }

    return bDispelled;
  }
}


class U4_Field_Poison_Widget extends U4_Magic_Field_Widget
{
  function AffectCreature( i_ciCreature )
  {
    if( ( i_ciCreature.GetMap() == GetMap() ) && i_ciCreature.GetPosition().Equals( GetPosition() ) )
    {
      base.AffectCreature( i_ciCreature )

      if( IsVisible() )
      {
        i_ciCreature.Poison();
      }
    }
  }


  function IsHarmful( i_ciCreature )
  {
    local bImmune = i_ciCreature.IsVenomous() || i_ciCreature.ResistsPoison() || i_ciCreature.IsUndead();
    return IsVisible() && !bImmune;
  }
}


class U4_Field_Lightning_Widget extends U4_Magic_Field_Widget
{
  function AffectCreature( i_ciCreature )
  {
    if( ( i_ciCreature.GetMap() == GetMap() ) && i_ciCreature.GetPosition().Equals( GetPosition() ) )
    {
      base.AffectCreature( i_ciCreature )

      if( IsVisible() && !i_ciCreature.ResistsLightning() )
      {
        local ciWeapon = ::rumGetAsset( U4_Lightning_Field_Weapon_InventoryID );
        if( ciWeapon != null )
        {
          local ciImmunityTable = {};
          local iChainDepth = ciWeapon.GetProperty( Inventory_Weapon_Damage_Chain_Depth_PropertyID, 1 );
          i_ciCreature.Electrify( ciImmunityTable, iChainDepth );
        }
      }
    }
  }


  function IsHarmful( i_ciCreature )
  {
    return IsVisible() && !i_ciCreature.ResistsLightning();
  }
}


class U4_Field_Fire_Widget extends U4_Magic_Field_Widget
{
  function AffectCreature( i_ciCreature )
  {
    if( ( i_ciCreature.GetMap() == GetMap() ) && i_ciCreature.GetPosition().Equals( GetPosition() ) )
    {
      base.AffectCreature( i_ciCreature )

      if( IsVisible() && !i_ciCreature.ResistsFire() )
      {
        i_ciCreature.Burn();
      }
    }
  }


  function IsHarmful( i_ciCreature )
  {
    return IsVisible() && !i_ciCreature.ResistsFire();
  }
}


class U4_Field_Sleep_Widget extends U4_Magic_Field_Widget
{
  function AffectCreature( i_ciCreature )
  {
    if( ( i_ciCreature.GetMap() == GetMap() ) && i_ciCreature.GetPosition().Equals( GetPosition() ) )
    {
      base.AffectCreature( i_ciCreature )

      if( IsVisible() )
      {
        local bImmune = i_ciCreature.ResistsSleep() || i_ciCreature.IsUndead();
        if( !bImmune )
        {
          i_ciCreature.Incapacitate();
        }
      }
    }
  }


  function IsHarmful( i_ciCreature )
  {
    local bImmune = i_ciCreature.ResistsSleep() || i_ciCreature.IsUndead();
    return IsVisible() && !bImmune;
  }
}


class U4_Field_Energy_Widget extends U4_Magic_Field_Widget
{
  function AffectCreature( i_ciCreature )
  {
    if( IsVisible() && ( i_ciCreature instanceof Player ) )
    {
      i_ciCreature.Resurrect( ResurrectionType.EnergyField );
    }
  }


  function Dispell()
  {
    // This field type cannot be dispelled
    return false;
  }
}
