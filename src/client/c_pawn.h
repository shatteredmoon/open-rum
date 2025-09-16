// NOTE: This class should be considered ABSTRACT and should not be directly instantiated

#ifndef _C_PAWN_H_
#define _C_PAWN_H_

#include <u_animation.h>


class rumClientPawn : public rumAnimation
{
public:

  static void ScriptBind();

protected:

  void OnCreated() override;

private:

  int32_t Serialize( rumResource& io_rcResource ) final;

  typedef rumAnimation super;
};


// These classes exist only so that we can provide a Pawn Type without
// requiring end users to properly provide them in their scripts. Once they
// extend a script class from a native class, the type is embedded here upon
// construction automatically.

class rumClientCreature : public rumClientPawn
{
public:

  rumClientCreature()
  {
    SetDrawOrder( -1.f );
  }

  PawnType GetPawnType() const override
  {
    return rumPawn::Creature_PawnType;
  }

protected:

  void AllocateGameID( rumUniqueID i_uiGameID = INVALID_GAME_ID ) override;
};


class rumClientPortal : public rumClientPawn
{
public:

  rumClientPortal()
  {
    SetDrawOrder( 1.f );
  }

  PawnType GetPawnType() const override
  {
    return rumPawn::Portal_PawnType;
  }

protected:

  void AllocateGameID( rumUniqueID i_uiGameID = INVALID_GAME_ID ) override;
};


class rumClientWidget : public rumClientPawn
{
public:
  rumClientWidget()
  {
    SetDrawOrder( 0.f );
  }

  PawnType GetPawnType() const override
  {
    return rumPawn::Widget_PawnType;
  }

protected:

  void AllocateGameID( rumUniqueID i_uiGameID = INVALID_GAME_ID ) override;
};

#endif // _C_PAWN_H_
