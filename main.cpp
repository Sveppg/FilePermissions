/*
 * Qt Stuff to change filepermissions
*/
#include <QApplication>
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QLabel>

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>

class PermissionWindow : public QWidget {
public:
    PermissionWindow(QWidget *parent = nullptr) : QWidget(parent) {
        setWindowTitle("File Permission Tool");

        filePathEdit = new QLineEdit(this);
        QPushButton *browseButton = new QPushButton("Auswählen", this);

        QHBoxLayout *fileLayout = new QHBoxLayout;
        fileLayout->addWidget(new QLabel("Datei:", this));
        fileLayout->addWidget(filePathEdit);
        fileLayout->addWidget(browseButton);

        changeExistingButton = new QRadioButton("Rechte bestehender Datei ändern", this);
        createNewButton = new QRadioButton("Neue Datei mit Rechten erstellen", this);
        changeExistingButton->setChecked(true);

        QVBoxLayout *actionLayout = new QVBoxLayout;
        actionLayout->addWidget(changeExistingButton);
        actionLayout->addWidget(createNewButton);

        QGridLayout *permLayout = new QGridLayout;
        permLayout->addWidget(new QLabel("Read"), 0, 1);
        permLayout->addWidget(new QLabel("Write"), 0, 2);
        permLayout->addWidget(new QLabel("Execute"), 0, 3);

        permLayout->addWidget(new QLabel("Owner"), 1, 0);
        permLayout->addWidget(new QLabel("Group"), 2, 0);
        permLayout->addWidget(new QLabel("Other"), 3, 0);

        ownerRead = new QCheckBox(this);
        ownerWrite = new QCheckBox(this);
        ownerExec = new QCheckBox(this);

        groupRead = new QCheckBox(this);
        groupWrite = new QCheckBox(this);
        groupExec = new QCheckBox(this);

        otherRead = new QCheckBox(this);
        otherWrite = new QCheckBox(this);
        otherExec = new QCheckBox(this);

        permLayout->addWidget(ownerRead, 1, 1);
        permLayout->addWidget(ownerWrite, 1, 2);
        permLayout->addWidget(ownerExec, 1, 3);

        permLayout->addWidget(groupRead, 2, 1);
        permLayout->addWidget(groupWrite, 2, 2);
        permLayout->addWidget(groupExec, 2, 3);

        permLayout->addWidget(otherRead, 3, 1);
        permLayout->addWidget(otherWrite, 3, 2);
        permLayout->addWidget(otherExec, 3, 3);

        // def: 644
        ownerRead->setChecked(true);
        ownerWrite->setChecked(true);
        groupRead->setChecked(true);
        otherRead->setChecked(true);

        QPushButton *applyButton = new QPushButton("Anwenden", this);

        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        mainLayout->addLayout(fileLayout);
        mainLayout->addLayout(actionLayout);
        mainLayout->addSpacing(10);
        mainLayout->addLayout(permLayout);
        mainLayout->addSpacing(10);
        mainLayout->addWidget(applyButton);

        connect(browseButton, &QPushButton::clicked, this, [this]() {
            QString path;

            if (changeExistingButton->isChecked()) {
                path = QFileDialog::getOpenFileName(this, "Datei auswählen");
            } else {
                path = QFileDialog::getSaveFileName(this, "Neue Datei erstellen");
            }

            if (!path.isEmpty()) {
                filePathEdit->setText(path);
            }
        });

        connect(applyButton, &QPushButton::clicked, this, [this]() {
            applyPermissions();
        });
    }

private:
    QLineEdit *filePathEdit;

    QRadioButton *changeExistingButton;
    QRadioButton *createNewButton;

    QCheckBox *ownerRead;
    QCheckBox *ownerWrite;
    QCheckBox *ownerExec;

    QCheckBox *groupRead;
    QCheckBox *groupWrite;
    QCheckBox *groupExec;

    QCheckBox *otherRead;
    QCheckBox *otherWrite;
    QCheckBox *otherExec;

    mode_t collectMode() const {
        mode_t mode = 0;
	// Permissions for files using POSIX Standard
        if (ownerRead->isChecked())  mode |= S_IRUSR;
        if (ownerWrite->isChecked()) mode |= S_IWUSR;
        if (ownerExec->isChecked())  mode |= S_IXUSR;

        if (groupRead->isChecked())  mode |= S_IRGRP;
        if (groupWrite->isChecked()) mode |= S_IWGRP;
        if (groupExec->isChecked())  mode |= S_IXGRP;

        if (otherRead->isChecked())  mode |= S_IROTH;
        if (otherWrite->isChecked()) mode |= S_IWOTH;
        if (otherExec->isChecked())  mode |= S_IXOTH;

        return mode;
    }

    void applyPermissions() {
        QString path = filePathEdit->text();

        if (path.isEmpty()) {
            QMessageBox::warning(this, "Fehler", "Bitte eine Datei auswählen.");
            return;
        }

        mode_t mode = collectMode();
        QByteArray pathBytes = path.toLocal8Bit();

        if (changeExistingButton->isChecked()) {
            if (chmod(pathBytes.constData(), mode) == -1) {
                QMessageBox::critical(
                    this,
                    "Fehler",
                    QString("chmod fehlgeschlagen:\n%1").arg(strerror(errno))
                    );
                return;
            }

            QMessageBox::information(
                this,
                "Erfolg",
                QString("Rechte geändert auf: %1").arg(mode, 3, 8, QChar('0'))
                );
        } else {
            mode_t oldUmask = umask(0);

            int fd = open(
                pathBytes.constData(),
                O_CREAT | O_EXCL | O_WRONLY,
                mode
                );

            umask(oldUmask);

            if (fd == -1) {
                QMessageBox::critical(
                    this,
                    "Fehler",
                    QString("Datei konnte nicht erstellt werden:\n%1").arg(strerror(errno))
                    );
                return;
            }

            close();

            QMessageBox::information(
                this,
                "Erfolg",
                QString("Datei erstellt mit Rechten: %1").arg(mode, 3, 8, QChar('0'))
                );
        }
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    PermissionWindow window;
    window.resize(500, 250);
    window.show();

    return app.exec();
}
