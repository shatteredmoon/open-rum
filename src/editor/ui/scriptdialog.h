#ifndef SCRIPTDIALOG_H
#define SCRIPTDIALOG_H

#include <QDialog>

#undef TRUE
#undef FALSE

#include <u_script.h>

namespace Ui
{
  class ScriptDialog;
}

class ScriptDialog : public QDialog
{
  Q_OBJECT

public:

  explicit ScriptDialog( Sqrat::Object i_sqObj, QWidget* i_pcParent = 0 );
  ~ScriptDialog();

private slots:

  void on_buttonBox_accepted();

private:

  enum SpecialInputTypes
  {
    INTEGER_COMBO_BOX, INTEGER_LIST_WIDGET
  };

  Ui::ScriptDialog* m_pcUI;

  // List of user-provided input fields
  QList<QWidget*> m_cInputList;
  QList<int32_t> m_cInputTypeList;
};

#endif // SCRIPTDIALOG_H
