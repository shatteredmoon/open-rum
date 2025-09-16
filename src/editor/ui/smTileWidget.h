#ifndef SM_TILEWIDGET_H
#define SM_TILEWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFUnctions>

class MapEditor;
class smMapWidget;


class smTileWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
  Q_OBJECT

public:
  smTileWidget( QWidget* i_pcParent = 0 );

  void initializeGL() override;
  void paintGL() override;

  void Update()
  {
    update();
  }

  void OnAnimTimer();

private:
  MapEditor* m_pcParent;

  static float s_fCursorColor;
  static float s_fCursorAnimDelta;

  static uint32_t s_iHeight;
  static uint32_t s_iWidth;

  using super = QOpenGLWidget;
};

#endif // SM_TILEWIDGET_H
