/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#include <QtTest>
#include <QRegularExpression>
#include <QDir>
#include "RmapVersion.hpp"

class TestVersion : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testNumericVersionComponents();
    void testSemVerFormat();
    void testGitAndBuildMetadata();
    void testVersionConsistency();
    void testPrereleaseAndBuild();
};

void TestVersion::initTestCase()
{
    QDir dir("work");
    if (dir.exists()) {
        dir.removeRecursively();
    }
    dir.mkpath(".");
}

void TestVersion::cleanupTestCase()
{
    QDir dir("work");
    if (dir.exists()) {
        dir.removeRecursively();
    }
}

void TestVersion::testNumericVersionComponents()
{
    QCOMPARE(rmap::version::major(), RMAP_VERSION_MAJOR);
    QCOMPARE(rmap::version::minor(), RMAP_VERSION_MINOR);
    QCOMPARE(rmap::version::patch(), RMAP_VERSION_PATCH);

    QVERIFY(RMAP_VERSION_MAJOR >= 0);
    QVERIFY(RMAP_VERSION_MINOR >= 0);
    QVERIFY(RMAP_VERSION_PATCH >= 0);
}

void TestVersion::testSemVerFormat()
{
    const QString versionStr = QString::fromLatin1(rmap::version::string());
    QVERIFY(!versionStr.isEmpty());
    QCOMPARE(versionStr, QString::fromLatin1(RMAP_VERSION_STRING));

    // SemVer 2.0.0 regex specification
    QRegularExpression semverRegex(
        R"(^(0|[1-9]\d*)\.(0|[1-9]\d*)\.(0|[1-9]\d*)(?:-((?:0|[1-9]\d*|\d*[a-zA-Z-][0-9a-zA-Z-]*)(?:\.(?:0|[1-9]\d*|\d*[a-zA-Z-][0-9a-zA-Z-]*))*))?(?:\+([0-9a-zA-Z-]+(?:\.[0-9a-zA-Z-]+)*))?$)"
    );
    QRegularExpressionMatch match = semverRegex.match(versionStr);
    QVERIFY2(match.hasMatch(), qPrintable(QString("Version '%1' does not match SemVer 2.0.0").arg(versionStr)));

    int parsedMajor = match.captured(1).toInt();
    int parsedMinor = match.captured(2).toInt();
    int parsedPatch = match.captured(3).toInt();

    QCOMPARE(parsedMajor, RMAP_VERSION_MAJOR);
    QCOMPARE(parsedMinor, RMAP_VERSION_MINOR);
    QCOMPARE(parsedPatch, RMAP_VERSION_PATCH);
}

void TestVersion::testGitAndBuildMetadata()
{
    const QString hash = QString::fromLatin1(rmap::version::gitHash());
    const QString branch = QString::fromLatin1(rmap::version::gitBranch());
    const QString date = QString::fromLatin1(rmap::version::buildDate());

    QVERIFY(!hash.isEmpty());
    QVERIFY(!branch.isEmpty());
    QVERIFY(!date.isEmpty());

    // Date format YYYY-MM-DD
    QRegularExpression dateRegex(R"(^\d{4}-\d{2}-\d{2}$)");
    QVERIFY(dateRegex.match(date).hasMatch());
}

void TestVersion::testVersionConsistency()
{
    QString expectedPrefix = QString("%1.%2.%3")
        .arg(RMAP_VERSION_MAJOR)
        .arg(RMAP_VERSION_MINOR)
        .arg(RMAP_VERSION_PATCH);

    QString actualVersion = QString::fromLatin1(rmap::version::string());
    QVERIFY(actualVersion.startsWith(expectedPrefix));
}

void TestVersion::testPrereleaseAndBuild()
{
    const char* pre = rmap::version::prerelease();
    const char* bld = rmap::version::build();
    QVERIFY(pre != nullptr);
    QVERIFY(bld != nullptr);

    QString fullStr = QString::fromLatin1(rmap::version::string());
    QString preStr = QString::fromLatin1(pre);
    if (!preStr.isEmpty()) {
        QVERIFY(fullStr.contains(preStr));
    }
    QString bldStr = QString::fromLatin1(bld);
    if (!bldStr.isEmpty()) {
        QVERIFY(fullStr.contains(bldStr));
    }
}

QTEST_MAIN(TestVersion)
#include "test_Version.moc"
