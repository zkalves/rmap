/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2026 Ezequiel Alves. All rights reserved.
 */

#include <QtTest>
#include "ProtobufLogCollector.hpp"

class TestProtobufLogCollector : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testInitialEmpty();
    void testAddError();
    void testAddWarning();
    void testMultipleMessages();
    void testDestructorThroughBase();
};

void TestProtobufLogCollector::initTestCase()
{
}

void TestProtobufLogCollector::cleanupTestCase()
{
}

void TestProtobufLogCollector::testInitialEmpty()
{
    ProtobufLogCollector collector;
    QCOMPARE(collector.string(), std::string(""));
    QCOMPARE(collector.get_string(), std::string(""));
}

void TestProtobufLogCollector::testAddError()
{
    ProtobufLogCollector collector;
    collector.AddError(0, 5, "Syntax error at token");
    std::string expected = "ERROR (1,6):Syntax error at token\n";
    QCOMPARE(collector.string(), expected);
    QCOMPARE(collector.get_string(), expected);
}

void TestProtobufLogCollector::testAddWarning()
{
    ProtobufLogCollector collector;
    collector.AddWarning(10, 2, "Deprecated field used");
    std::string expected = "WARNING (11,3):Deprecated field used\n";
    QCOMPARE(collector.string(), expected);
    QCOMPARE(collector.get_string(), expected);
}

void TestProtobufLogCollector::testMultipleMessages()
{
    ProtobufLogCollector collector;
    collector.AddError(1, 0, "Missing semicolon");
    collector.AddWarning(4, 12, "Unused import");
    std::string expected = "ERROR (2,1):Missing semicolon\nWARNING (5,13):Unused import\n";
    QCOMPARE(collector.string(), expected);
    QCOMPARE(collector.get_string(), expected);
}

void TestProtobufLogCollector::testDestructorThroughBase()
{
    google::protobuf::io::ErrorCollector* collector = new ProtobufLogCollector();
    collector->AddError(0, 0, "test");
    delete collector;
}

QTEST_MAIN(TestProtobufLogCollector)
#include "test_ProtobufLogCollector.moc"
