/* This file is part of rmap.
 *
 * rmap is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * rmap is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with rmap.  If not, see <https://www.gnu.org/licenses/>.
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
