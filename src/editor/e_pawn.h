// NOTE: This class should be considered ABSTRACT and should not be directly instantiated

#ifndef _E_PAWN_H_
#define _E_PAWN_H_

#include <u_animation.h>

class EditorPawn : public rumAnimation
{
public:

  void OnCreated() override;

  static void ScriptBind();

private:

  rumUniqueID m_iPersistentID{ INVALID_GAME_ID };

  using super = rumAnimation;
};


// These classes exist only so that we can provide a Pawn Type without
// requiring end users to properly provide them in their scripts. Once they
// extend a script class from a native class, the type is embedded here upon
// construction automatically.

class EditorCreature : public EditorPawn
{
public:

  void AllocateGameID( rumUniqueID i_uiGameID ) override;

  PawnType GetPawnType() const override
  {
    return rumPawn::Creature_PawnType;
  }
};


class EditorPortal : public EditorPawn
{
public:

  void AllocateGameID( rumUniqueID i_uiGameID ) override;

  PawnType GetPawnType() const override
  {
    return rumPawn::Portal_PawnType;
  }
};


class EditorWidget : public EditorPawn
{
public:

  void AllocateGameID( rumUniqueID i_uiGameID ) override;

  PawnType GetPawnType() const override
  {
    return rumPawn::Widget_PawnType;
  }
};

#endif // _E_PAWN_H_
