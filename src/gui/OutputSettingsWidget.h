#ifndef OUTPUTSETTINGSWIDGET_H
#define OUTPUTSETTINGSWIDGET_H

#include <QWidget>

class QCheckBox;
class QLineEdit;
class QComboBox;
class QLabel;

/**
 * @brief Widget for virtual camera output settings
 *
 * Contains controls for enabling virtual camera output via v4l2loopback,
 * selecting the output device, and configuring the output resolution.
 */
class OutputSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit OutputSettingsWidget(QWidget *parent = nullptr);

    // Accessors
    bool isVirtualCameraEnabled() const;
    QString devicePath() const;
    QString resolutionKey() const;

    // Setters (for loading config)
    void setVirtualCameraEnabled(bool enabled);
    void setDevicePath(const QString &path);
    void setResolutionKey(const QString &key);

    // Status updates
    void setStatusText(const QString &text, const QString &color);
    void setAvailable(bool available);

signals:
    void virtualCameraToggled(bool enabled);
    void devicePathEdited();
    void resolutionChanged(int index);

private:
    void setupUI();

    QCheckBox *m_enableCheckbox;
    QLineEdit *m_deviceEdit;
    QComboBox *m_resolutionCombo;
    QLabel *m_statusLabel;
    QLabel *m_resolutionHintLabel;
};

#endif // OUTPUTSETTINGSWIDGET_H
