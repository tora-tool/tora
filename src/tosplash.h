/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */


#ifndef TOSPLASH_H
#define TOSPLASH_H

#include <QWidget>
#include <QLabel>

class QLabel;
class QProgressBar;

class toSplash : public QWidget
{
    QProgressBar *Progress;
    QLabel       *Label;

public:
    toSplash(QWidget *parent = 0, const char *name = 0, Qt::WFlags f = 0);
    ~toSplash();
    QLabel* label(void);
    QProgressBar* progress(void);
};

inline QLabel* toSplash::label(void)
{
    return Label;
}

inline QProgressBar* toSplash::progress(void)
{
    return Progress;
}

#endif
