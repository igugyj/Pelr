
#pragma once

#include "AgreementDialog.h"

class PrivacyPolicyDialog : public AgreementDialog
{
    Q_OBJECT

public:
    explicit PrivacyPolicyDialog(QWidget *parent = nullptr);
    static bool hasAccepted();

protected:
    QString agreementTitle() const override;
    QString agreementText() const override;
    QString checkFileName() const override;
    int agreementMaxDays() const override;
};
