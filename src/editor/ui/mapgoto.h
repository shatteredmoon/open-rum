#ifndef MAPGOTO_H
#define MAPGOTO_H

#include <QDialog>

namespace Ui
{
  class MapGoto;
}

class MapGoto : public QDialog
{
  Q_OBJECT

public:

  explicit MapGoto( const QPoint& i_rcPosMax, QWidget* i_pcParent = 0 );
  ~MapGoto();

signals:

  void GotoMapPosition( const QPoint& i_rcPos );

private slots:

  void on_buttonBox_accepted();
  void on_spinBox_valueChanged( int32_t i_iPosX );
  void on_spinBox_2_valueChanged( int32_t i_iPosY );

private:

  Ui::MapGoto* m_pcUI;

  QPoint m_cPosMax;
};

#endif // MAPGOTO_H
