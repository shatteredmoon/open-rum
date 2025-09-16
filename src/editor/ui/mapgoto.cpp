#include <mapgoto.h>
#include <ui_mapgoto.h>


MapGoto::MapGoto( const QPoint& i_rcMapSize, QWidget* i_pcParent )
  : QDialog( i_pcParent )
  , m_pcUI( new Ui::MapGoto )
  , m_cPosMax( i_rcMapSize.x() - 1, i_rcMapSize.y() - 1 )
{
  m_pcUI->setupUi( this );
}


MapGoto::~MapGoto()
{
  delete m_pcUI;
}


void MapGoto::on_buttonBox_accepted()
{
  emit GotoMapPosition( QPoint( m_pcUI->spinBox->value(), m_pcUI->spinBox_2->value() ) );
  accept();
}


void MapGoto::on_spinBox_valueChanged( int32_t i_iPosX )
{
  m_pcUI->spinBox->setValue( qBound( 0, i_iPosX, m_cPosMax.x() ) );
}


void MapGoto::on_spinBox_2_valueChanged( int32_t i_iPosY )
{
  m_pcUI->spinBox_2->setValue( qBound( 0, i_iPosY, m_cPosMax.y() ) );
}
