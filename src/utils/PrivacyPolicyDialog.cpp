
#include "PrivacyPolicyDialog.h"
#include "data.hpp"

PrivacyPolicyDialog::PrivacyPolicyDialog(QWidget *parent)
    : AgreementDialog(parent)
{
}

bool PrivacyPolicyDialog::hasAccepted()
{
    QString version = DataManager::instance().const_config_data.version;
    return hasAcceptedCheck("privacy.dat", -1);
}

QString PrivacyPolicyDialog::agreementTitle() const
{
    return tr("Privacy Policy");
}

QString PrivacyPolicyDialog::agreementText() const
{
    return tr(
        "Privacy Policy\n\n"
        "Last updated: June 2026\n\n"
        "This Privacy Policy describes how Pelr handles your data.\n\n"
        "1. Data Storage\n"
        "   All configuration data (API keys, model paths, display preferences) is stored "
        "locally in the user/ directory. Voice generation outputs are saved locally in "
        "voice_files/. Application logs are stored in log/. No data is transmitted to any "
        "server unless explicitly configured by you.\n\n"
        "2. Third-Party API Services\n"
        "   Pelr connects only to services that you explicitly configure:\n"
        "   - LLM APIs (OpenAI-compatible endpoints)\n"
        "   - TTS providers (Edge TTS, iFlytek, Voicevox, OpenAI TTS)\n"
        "   - Translation services (LibreTranslate, Tencent)\n"
        "   - OpenWeather API\n"
        "   API keys are stored locally and sent only to the corresponding services. "
        "You are responsible for reviewing the privacy policies of those third-party services.\n\n"
        "3. No Telemetry\n"
        "   Pelr does not collect usage statistics, analytics, or crash reports. "
        "No data is sent to the application authors.\n\n"
        "4. Data Security\n"
        "   API keys and configuration are stored in plaintext on your local machine. "
        "No encryption is applied to local storage. Protect your device accordingly.\n\n"
        "5. Open Source\n"
        "   Pelr is open source. You may inspect, build, and modify the source code "
        "to verify its data handling practices.\n\n"
        "User: %1\n"
        "Version: %2")
        .arg(getComputerName())
        .arg(getCurrentVersion());
}

QString PrivacyPolicyDialog::checkFileName() const
{
    return "privacy.dat";
}

int PrivacyPolicyDialog::agreementMaxDays() const
{
    return -1;
}
