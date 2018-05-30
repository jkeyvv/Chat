/********************************************************************************
** Form generated from reading UI file 'chat.ui'
**
** Created by: Qt User Interface Compiler version 5.11.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CHAT_H
#define UI_CHAT_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_chat
{
public:
    QPushButton *start;
    QLabel *label;

    void setupUi(QWidget *chat)
    {
        if (chat->objectName().isEmpty())
            chat->setObjectName(QStringLiteral("chat"));
        chat->resize(1323, 839);
        start = new QPushButton(chat);
        start->setObjectName(QStringLiteral("start"));
        start->setGeometry(QRect(70, 770, 111, 51));
        start->setAutoDefault(false);
        start->setFlat(false);
        label = new QLabel(chat);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(20, 20, 1280, 720));

        retranslateUi(chat);

        start->setDefault(true);


        QMetaObject::connectSlotsByName(chat);
    } // setupUi

    void retranslateUi(QWidget *chat)
    {
        chat->setWindowTitle(QApplication::translate("chat", "Chat", nullptr));
        start->setText(QApplication::translate("chat", "Start", nullptr));
        label->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class chat: public Ui_chat {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CHAT_H
