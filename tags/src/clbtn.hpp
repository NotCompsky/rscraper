#include <QColor>
#include <QMouseEvent>
#include <QPushButton>


class SelectColourButton : public QPushButton{
    Q_OBJECT
  private:
    void display_subs_w_tag();
  private Q_SLOTS:
    void mousePressEvent(QMouseEvent* e);
  public:
    uint64_t tag_id; // Should be const
    QColor colour;
    explicit SelectColourButton(const uint64_t id,  const unsigned char r,  const unsigned char g,  const unsigned char b,  const unsigned char a,  const char* name,  QWidget* parent);
  public Q_SLOTS:
    void set_colour();
};
