#include <QWidget>
#include <QVBoxLayout>


class ClTagsTab : public QWidget{
    Q_OBJECT
  public:
    explicit ClTagsTab(const uint64_t id, QWidget* parent = 0);
    const uint64_t cat_id;
  public Q_SLOTS:
    void add_tag();
    uint64_t create_tag(QString& qs,  const char* s);
  private:
    QVBoxLayout* l;
};
