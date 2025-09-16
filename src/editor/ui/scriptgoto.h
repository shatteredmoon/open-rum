#ifndef SCRIPTGOTO_H
#define SCRIPTGOTO_H

#include <QDialog>

class QTextDocument;

namespace Ui
{
  class ScriptGoto;
}

class ScriptGoto : public QDialog
{
  Q_OBJECT

public:

  explicit ScriptGoto( QTextDocument* i_pcDocument, QWidget* i_pcParent = 0 );
  ~ScriptGoto();

signals:

  void GotoPosition( int32_t i_iLine, int32_t i_iColumn );

private slots:

  void on_buttonBox_accepted();
  void on_spinBoxLine_valueChanged( int32_t i_iLine );
  void on_spinBoxColumn_valueChanged( int32_t i_iColumn );

private:

  Ui::ScriptGoto* i_pcUI;

  int32_t m_iLineMax;
};

#endif // SCRIPTGOTO_H
