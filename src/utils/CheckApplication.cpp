
#include "CheckApplication.h"
#include "data.hpp"

CheckApplication::CheckApplication(QWidget *parent)
    : AgreementDialog(parent)
{
}

bool CheckApplication::hasValidLicense()
{
    return hasAcceptedCheck("license.dat", 365);
}

QString CheckApplication::agreementTitle() const
{
    return tr("MIT License Agreement");
}

QString CheckApplication::agreementText() const
{
    return tr(
        "MIT License\n\n"
        "Copyright (c) 2007 Free Software Foundation <https://fsf.org/>\n\n"
        "Permission is hereby granted, free of charge, to any person obtaining a copy "
        "of this software and associated documentation files (the \"Software\"), to deal "
        "in the Software without restriction, including without limitation the rights "
        "to use, copy, modify, merge, publish, distribute, sublicense, and/or sell "
        "copies of the Software, and to permit persons to whom the Software is "
        "furnished to do so, subject to the following conditions:\n\n"
        "The above copyright notice and this permission notice shall be included in "
        "all copies or substantial portions of the Software.\n\n"
        "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR "
        "IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, "
        "FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE "
        "AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER "
        "LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING "
        "FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER "
        "DEALINGS IN THE SOFTWARE.\n\n"
        "This program is licensed under the MIT License.\n\n"
        "User: %1\n"
        "Version: %2")
        .arg(getComputerName())
        .arg(getCurrentVersion());
}

QString CheckApplication::checkFileName() const
{
    return "license.dat";
}

int CheckApplication::agreementMaxDays() const
{
    return 365;
}
