#include "OutputSettingsWidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QRegularExpression>

namespace {

struct VirtualCameraResolutionPreset {
    const char *key;
    int width;
    int height;
};

constexpr VirtualCameraResolutionPreset kVirtualCameraResolutionPresets[] = {
    {"match", 0, 0},
    {"960x540", 960, 540},
    {"1280x720", 1280, 720},
    {"1920x1080", 1920, 1080}
};

QString buildResolutionLabel(const VirtualCameraResolutionPreset &preset)
{
    if (preset.width <= 0 || preset.height <= 0) {
        return OutputSettingsWidget::tr("Match preview resolution");
    }

    const int progressiveHeight = preset.height;
    return OutputSettingsWidget::tr("%1p (%2 x %3)")
        .arg(progressiveHeight)
        .arg(preset.width)
        .arg(preset.height);
}

QSize resolutionSizeForKey(const QString &key)
{
    if (key.isEmpty() || key.compare(QStringLiteral("match"), Qt::CaseInsensitive) == 0) {
        return QSize();
    }

    QString normalized = key.trimmed();
    normalized.replace(QLatin1Char('X'), QLatin1Char('x'));

    const int separator = normalized.indexOf(QLatin1Char('x'));
    if (separator <= 0) {
        return QSize();
    }

    bool okWidth = false;
    bool okHeight = false;
    const int width = normalized.left(separator).toInt(&okWidth);
    const int height = normalized.mid(separator + 1).toInt(&okHeight);

    if (!okWidth || !okHeight || width <= 0 || height <= 0) {
        return QSize();
    }

    return QSize(width, height);
}

QString modprobeCommandForDevice(const QString &devicePath)
{
    static const QRegularExpression videoRegex(QStringLiteral("^/dev/video(\\d+)$"));
    const QRegularExpressionMatch match = videoRegex.match(devicePath.trimmed());
    const QString videoNr = match.hasMatch() ? match.captured(1) : QStringLiteral("42");
    return OutputSettingsWidget::tr("sudo modprobe v4l2loopback video_nr=%1 card_label=\"OBSBOT Virtual Camera\" exclusive_caps=1")
        .arg(videoNr);
}

} // namespace

OutputSettingsWidget::OutputSettingsWidget(QWidget *parent)
    : QWidget(parent)
    , m_enableCheckbox(nullptr)
    , m_deviceEdit(nullptr)
    , m_resolutionCombo(nullptr)
    , m_statusLabel(nullptr)
    , m_resolutionHintLabel(nullptr)
{
    setupUI();
}

void OutputSettingsWidget::setupUI()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 8, 4, 8);
    layout->setSpacing(12);

    // Virtual Camera section header
    QLabel *sectionHeader = new QLabel(tr("Virtual Camera"), this);
    sectionHeader->setStyleSheet("font-weight: 600; font-size: 14px;");
    layout->addWidget(sectionHeader);

    // Enable checkbox
    m_enableCheckbox = new QCheckBox(tr("Enable virtual camera output"), this);
    m_enableCheckbox->setObjectName("footerCheckbox");
    connect(m_enableCheckbox, &QCheckBox::toggled,
            this, &OutputSettingsWidget::virtualCameraToggled);
    layout->addWidget(m_enableCheckbox);

    // Device path row
    QHBoxLayout *deviceLayout = new QHBoxLayout();
    deviceLayout->setContentsMargins(0, 0, 0, 0);
    deviceLayout->setSpacing(8);

    QLabel *deviceLabel = new QLabel(tr("Device path"), this);
    deviceLayout->addWidget(deviceLabel);

    m_deviceEdit = new QLineEdit(this);
    m_deviceEdit->setPlaceholderText("/dev/video42");
    connect(m_deviceEdit, &QLineEdit::editingFinished,
            this, &OutputSettingsWidget::devicePathEdited);
    deviceLayout->addWidget(m_deviceEdit, 1);

    layout->addLayout(deviceLayout);

    // Status label
    m_statusLabel = new QLabel(
        tr("Virtual camera support requires the v4l2loopback kernel module."),
        this);
    m_statusLabel->setWordWrap(true);
    m_statusLabel->setObjectName("virtualCameraStatus");
    layout->addWidget(m_statusLabel);

    // Resolution row
    QHBoxLayout *resolutionLayout = new QHBoxLayout();
    resolutionLayout->setContentsMargins(0, 0, 0, 0);
    resolutionLayout->setSpacing(8);

    QLabel *resolutionLabel = new QLabel(tr("Output resolution"), this);
    resolutionLayout->addWidget(resolutionLabel);

    m_resolutionCombo = new QComboBox(this);
    m_resolutionCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    for (const auto &preset : kVirtualCameraResolutionPresets) {
        const QString key = QLatin1String(preset.key);
        const QString label = buildResolutionLabel(preset);
        m_resolutionCombo->addItem(label, key);
    }
    connect(m_resolutionCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &OutputSettingsWidget::resolutionChanged);
    resolutionLayout->addWidget(m_resolutionCombo, 1);

    layout->addLayout(resolutionLayout);

    // Resolution hint
    m_resolutionHintLabel = new QLabel(
        tr("Pick a fixed size to keep Zoom and other apps happy when you change preview quality."),
        this);
    m_resolutionHintLabel->setWordWrap(true);
    m_resolutionHintLabel->setStyleSheet("color: palette(mid); font-size: 11px;");
    layout->addWidget(m_resolutionHintLabel);

    layout->addStretch();
}

bool OutputSettingsWidget::isVirtualCameraEnabled() const
{
    return m_enableCheckbox && m_enableCheckbox->isChecked();
}

QString OutputSettingsWidget::devicePath() const
{
    if (!m_deviceEdit) {
        return QStringLiteral("/dev/video42");
    }

    const QString path = m_deviceEdit->text().trimmed();
    if (path.isEmpty()) {
        return QStringLiteral("/dev/video42");
    }

    return path;
}

QString OutputSettingsWidget::resolutionKey() const
{
    if (!m_resolutionCombo || m_resolutionCombo->currentIndex() < 0) {
        return QStringLiteral("match");
    }
    return m_resolutionCombo->currentData().toString();
}

void OutputSettingsWidget::setVirtualCameraEnabled(bool enabled)
{
    if (m_enableCheckbox) {
        m_enableCheckbox->blockSignals(true);
        m_enableCheckbox->setChecked(enabled);
        m_enableCheckbox->blockSignals(false);
    }
}

void OutputSettingsWidget::setDevicePath(const QString &path)
{
    if (m_deviceEdit) {
        m_deviceEdit->blockSignals(true);
        m_deviceEdit->setText(path);
        m_deviceEdit->blockSignals(false);
    }
}

void OutputSettingsWidget::setResolutionKey(const QString &key)
{
    if (!m_resolutionCombo) {
        return;
    }

    m_resolutionCombo->blockSignals(true);
    int index = m_resolutionCombo->findData(key);
    if (index < 0) {
        const QSize customSize = resolutionSizeForKey(key);
        if (customSize.isValid()) {
            const QString label = tr("Custom (%1 x %2)")
                .arg(customSize.width())
                .arg(customSize.height());
            m_resolutionCombo->addItem(label, key);
            index = m_resolutionCombo->count() - 1;
        } else {
            index = m_resolutionCombo->findData(QStringLiteral("match"));
        }
    }
    if (index >= 0) {
        m_resolutionCombo->setCurrentIndex(index);
    }
    m_resolutionCombo->blockSignals(false);
}

void OutputSettingsWidget::setStatusText(const QString &text, const QString &color)
{
    if (m_statusLabel) {
        m_statusLabel->setText(text);
        m_statusLabel->setStyleSheet(QStringLiteral("color: %1;").arg(color));
    }
    if (m_enableCheckbox) {
        m_enableCheckbox->setToolTip(text);
    }
}

void OutputSettingsWidget::setAvailable(bool available)
{
    Q_UNUSED(available);
    // Could be used to disable controls when not available
}
