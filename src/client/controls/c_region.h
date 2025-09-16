#pragma once

#include <controls/c_control.h>

struct rumPoint;


// A region is just a rectangular bounds that can capture input. The region doesn't contain anything to display.
class rumClientRegion : public rumClientControl
{
public:

  void Clear() override;

  void Draw( const rumPoint& i_rcPos ) override;

  static void ScriptBind();

private:

  bool Validate() override;

  typedef rumClientControl super;
};
