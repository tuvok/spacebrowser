#ifndef TAB_H_
#define TAB_H_

#include <db/Tabs2.h>

#include <QByteArray>
#include <QHash>
#include <QObject>
#include <QStandardItem>
#include <QString>
#include <QVariant>

#include <vector>

class Tab : public QStandardItem
{
public:
    Tab(int id);
    Tab(const db::Tabs2::TabInfo& ti);

    bool operator== (const Tab &c1)
    {
        /// It is enough to compare id, it's uniqueness is guaranteed
        /// by the database
        ///

        return this->id == c1.id;
    }

    QVariant data(int column) const override;
    void updateIndent();

    int getId() const;
    QString getTitle() const;
    QString getIcon() const;
    QString getUrl() const;
    QVariant getView() const;

    void setTitle(QString title_);
    void setIcon(QString icon_);
    void setView(QVariant view_);

signals:
    void titleChanged();

public:
    static QHash<int, QByteArray> roles;

private:
    int columnCnt = 1;
    int id = -1;

    int indent = 0;

    QString url;
    QString title = "Empty";
    QString icon;

    QVariant view;
};

#endif /* TAB_H_ */
